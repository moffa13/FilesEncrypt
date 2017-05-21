#include <QFile>
#include <QFileInfo>
#include <iostream>
#include <QtDebug>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include "FilesEncrypt.h"
#include "utilities.h"
#include "openssl/err.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "Logger.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <cassert>

#define TIME_MIN_REMOVE_AES 3
#define CURRENT_VERSION 2

// Current crpyts number
unsigned FilesEncrypt::m_pendingCrypt = 0;
QMutex FilesEncrypt::m_mutex;


FilesEncrypt::FilesEncrypt(std::string const &key_file) : m_key_file(key_file){
    init();
    readFromFile();
}

FilesEncrypt::FilesEncrypt(const char* aes){
    init();
    setAES(aes);
}

void FilesEncrypt::init(){
    m_aes_decrypted = reinterpret_cast<unsigned char*>(malloc(32));
}

FilesEncrypt::~FilesEncrypt(){
    free(m_aes_decrypted);
}

void FilesEncrypt::addPendingCrypt(){
    m_mutex.lock();
    ++m_pendingCrypt;
    m_mutex.unlock();
}

void FilesEncrypt::removePendingCrypt(){
    m_mutex.lock();
    --m_pendingCrypt;
    emit file_done();
    m_mutex.unlock();
}


void FilesEncrypt::setAES(const char* aes){
    m_aes_decrypted_set = true;
    memcpy(m_aes_decrypted, aes, 32);
}

void FilesEncrypt::unsetAES(){
    m_aes_decrypted_set = false;
    memset(m_aes_decrypted, 0, 32);
}

const unsigned char* FilesEncrypt::getAES() const{
    return m_aes_decrypted;
}

unsigned FilesEncrypt::getPendingCrypt(){
    return m_pendingCrypt;
}

bool FilesEncrypt::genKey(QString const& file, QString const& password){
    bool success = false;

    // Vars to be unallocated after
    unsigned char* aes = nullptr;
    unsigned char* aes_encrypted = nullptr;
    char* rsaBuff = nullptr;
    BIO *bio = nullptr;
    EVP_PKEY* rsa = nullptr;
    RSA* public_key = nullptr;

    // Gen rsa, this has to be unallocated
    rsa = Crypt::genRSA(4096);
    public_key = Crypt::getRSAFromEVP_PKEY(rsa);

    // Write private key encrypted to <rsaStr>
    bio = BIO_new(BIO_s_mem());

    PEM_write_bio_PrivateKey(
        bio,
        rsa,
        EVP_des_ede3_cbc(),
        const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(password.toStdString().c_str())), // Might be dangerous
        password.length(),
        NULL,
        NULL
    );

    constexpr unsigned bufSize = 16;
    rsaBuff = reinterpret_cast<char*>(malloc(bufSize + 1));
    std::string rsaStr;
    while(BIO_read(bio, reinterpret_cast<char*>(rsaBuff), bufSize) > 0){
        rsaBuff[bufSize] = '\0';
        rsaStr += rsaBuff;
        memset(rsaBuff, '\0', bufSize);
    }

    // Gen aes key
    aes = reinterpret_cast<unsigned char*>(malloc(AESSIZE::S256));
    Crypt::genAES(AESSIZE::S256, aes);

    // Encrypt aes
    aes_encrypted = reinterpret_cast<unsigned char*>(malloc(RSA_size(public_key)));
    Crypt::encrypt(public_key, aes, AESSIZE::S256, aes_encrypted);

    QFile f(std::move(file));
    if(f.open(QFile::WriteOnly)){
        f.seek(0);
        f.write(rsaStr.c_str());
        f.write(&PRIVATE_ENCRYPT_AES_SEPARATOR, 1);
        f.write(reinterpret_cast<char*>(aes_encrypted), RSA_size(public_key));
        f.close();
        Logger::info("Key successfully created");
        success = true;
    }else{
        Logger::info("Can't write aes key to file");
    }

    if(aes != nullptr){
        free(aes);
    }
    if(aes_encrypted != nullptr) {
        free(aes_encrypted);
    }
    if(rsaBuff != nullptr) {
        free(rsaBuff);
    }
    if(bio != nullptr) {
        BIO_free(bio);
    }
    if(rsa != nullptr) {
        EVP_PKEY_free(rsa);
    }
    if(public_key != nullptr) {
        RSA_free(public_key);
    }

    return success;
}


/**
 * Tries to retrieve the key file in m_key_file, if it doesn't exist, returns false
 * Copies the crypted aes and private key to m_aes_crypted and m_private_key_crypted
 * @brief FilesEncrypt::init
 * @return True if key was retrieved, either false
 */
bool FilesEncrypt::readFromFile(){
    QFile f(m_key_file.c_str());
    if(!f.exists() || !f.open(QFile::ReadOnly)){
        Logger::error("Cannot retrieve key");
        f.close();
        return false;
    }

    // Get separated the private key and the aes-crypted key
    QByteArray arr(f.readAll());
    int split = arr.indexOf(PRIVATE_ENCRYPT_AES_SEPARATOR);
    QByteArray private_key = arr.mid(0, split);
    QByteArray aes_crypted = arr.mid(split + 1, -1);

    m_aes_crypted = reinterpret_cast<unsigned char*>(malloc(aes_crypted.length()));
    m_aes_crypted_length = aes_crypted.length();

    memcpy(reinterpret_cast<void*>(m_aes_crypted), aes_crypted.constData(), aes_crypted.length()); // Save the crypted aes
    m_private_key_crypted = private_key.toStdString(); // Save the crypted private key

    Logger::info("Crypted aes and crypted private key saved");

    f.close();
    return true;
}

bool FilesEncrypt::isAesDecrypted() const{
    return m_aes_decrypted_set;
}

bool FilesEncrypt::requestAesDecrypt(std::string const& password, bool* passOk){

    bool success = false;
    BIO* bio = nullptr;
    EVP_PKEY* container = nullptr;
    RSA* private_key = nullptr;

    // Write rsa to RSA container (RSA_st)
    bio = BIO_new(BIO_s_mem());
    BIO_write(bio, m_private_key_crypted.c_str(), m_private_key_crypted.length());
    container = EVP_PKEY_new();
    container = PEM_read_bio_PrivateKey(bio, NULL, NULL, const_cast<char*>(password.c_str()));
    if(container == NULL){
        if(passOk != NULL)
            *passOk = false;
        Logger::error("Incorrect password or something else");
        goto end;
    }else{
        if(passOk != NULL)
            *passOk = true;
    }

    if(!isAesDecrypted()){
        private_key = Crypt::getRSAFromEVP_PKEY(container); // Save the private key
        if(Crypt::decrypt(
            private_key,
            m_aes_crypted,
            m_aes_crypted_length,
            m_aes_decrypted
        ) != -1 ){
            success = true;
            Logger::info("AES successfully decrypted");
            m_aes_decrypted_set = true;
            startDeleteAesTimer();

        }else{
            Logger::info("AES not successfully decrypted");
        }
    }else{
        Logger::error("AES already decrypted");
        success = true;
    }

end:
    if(bio != nullptr){
        BIO_free(bio);
    }
    if(container != nullptr){
        EVP_PKEY_free(container);
    }
    if(private_key != nullptr){
        RSA_free(private_key);
    }

    return success;

}

void FilesEncrypt::startDeleteAesTimer(){
    QTimer::singleShot(1000 * 60 * TIME_MIN_REMOVE_AES, [this](){
        if(m_pendingCrypt == 0){
            unsetAES();
            Logger::info("AES key deleted from ram");
        }else{
            Logger::warn("Impossible to remove the key, already crypting/decrypting" + QString::number(m_pendingCrypt) + " file(s)");
            startDeleteAesTimer();
        }
    });
}

bool FilesEncrypt::encryptFile(QFile* file, EncryptDecrypt op){

    bool success{false};

    QString filename{QFileInfo(*file).fileName()};

    Logger::info("File size : " + QString::number(file->size()) + " bytes");

    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(true);
    tmpFile.open();
    QString name(file->fileName());
    QFileInfo fileInfo{file->fileName()};
    Crypt crypt;
    connect(&crypt, SIGNAL(aes_decrypt_updated(qint32)), this, SIGNAL(encrypt_updated(qint32)));
    connect(&crypt, SIGNAL(aes_encrypt_updated(qint32)), this, SIGNAL(decrypt_updated(qint32)));


    if(op == EncryptDecrypt::ENCRYPT){

        if(guessEncrypted(*file).state == EncryptDecrypt::ENCRYPT){
            Logger::warn("Trying to encrypt a file maybe already encrypted");
            goto end;
        }

        addPendingCrypt();

        file->seek(0);

        // Gen IV
        unsigned char* iv = reinterpret_cast<unsigned char*>(malloc(AES_BLOCK_SIZE));
        Crypt::genRandomIV(iv);


        QString nameWithoutPath{fileInfo.fileName()};
        QString newName;

        do{
            newName = "/" + utilities::randomString(15);
        }while(QFile::exists(fileInfo.absolutePath() + newName));

        name = fileInfo.absolutePath() + newName;

        unsigned char* encrypted_filename = reinterpret_cast<unsigned char*>(malloc(getEncryptedSize(nameWithoutPath.length())));

        crypt.aes_crypt(reinterpret_cast<const unsigned char*>(nameWithoutPath.toStdString().c_str()), nameWithoutPath.length(), encrypted_filename, m_aes_decrypted, iv);

        // Add Header E1N1C1R1Y1P1T1E1D1VZZZZZ;AAAAAAAAAAAAAAAAE1N1C1R1Y1P1T1E1D1XXXXX...
        auto blob = getEncryptBlob(reinterpret_cast<char*>(iv), CURRENT_VERSION, true, reinterpret_cast<const char*>(encrypted_filename), getEncryptedSize(nameWithoutPath.length()));
        QByteArray fileContentEncrypted{blob};
        tmpFile.write(fileContentEncrypted);

        free(encrypted_filename);


        // Log final size
        quint64 futureSize = getEncryptedSize(file->size()) + blob.size();
        Logger::info("Future file size will be " + utilities::speed_to_human(futureSize));

        // Crypt data
        crypt.aes_crypt(file, &tmpFile, m_aes_decrypted, iv);

        free(iv);
        iv = nullptr;

        removePendingCrypt();

        Logger::info("File " + filename + " encrypted");
        success = true;
    }else{

        EncryptDecrypt_s state{guessEncrypted(*file)};

        if(state.state == EncryptDecrypt::DECRYPT){
            Logger::warn("Trying to decrypt a file maybe not encrypted");
            goto end;
        }

        addPendingCrypt();

        // Gen IV
        file->seek(state.offsetBeforeContent);
        crypt.aes_decrypt(file, &tmpFile, m_aes_decrypted, const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(state.iv.constData())));

        char* uncrypted_filename = reinterpret_cast<char*>(malloc(state.newFilename.size()));
        auto nameSize = crypt.aes_decrypt(
                    reinterpret_cast<const unsigned char*>(state.newFilename.constData()),
                    state.newFilename.size(),
                    reinterpret_cast<unsigned char*>(uncrypted_filename),
                    m_aes_decrypted,
                    const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(state.iv.constData()))
        );

        QByteArray str(uncrypted_filename, nameSize);
        name = fileInfo.absolutePath() + "/" + QString::fromLocal8Bit(str);

        success = true;
        Logger::info("File " + filename + " decrypted");
        removePendingCrypt();

    }


    file->remove();
    tmpFile.copy(name);

end:
    return success;
}

EncryptDecrypt_s FilesEncrypt::guessEncrypted(QFile& file){
    QByteArray content = file.read(SIZE_BEFORE_CONTENT);
    return FilesEncrypt::guessEncrypted(content);
}

size_t FilesEncrypt::getEncryptedSize(int message_length){
    return (message_length / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
}

EncryptDecrypt FilesEncrypt::guessEncrypted(QDir& dir){

    unsigned crypted = 0;
    unsigned uncrypted = 0;
    unsigned total = 0;

    foreach(QFileInfo const& f, dir.entryInfoList()){
        if(f.isFile()){
            total++;
            QFile file(f.absoluteFilePath());
            if(!file.open(QFile::ReadOnly)){
                ;; // TODO MOTHERFUCKER
            }
        if(FilesEncrypt::guessEncrypted(file).state == EncryptDecrypt::ENCRYPT){
                crypted++;
            }else{
                uncrypted++;
            }
        }
    }

    if(crypted == total){
        return EncryptDecrypt::ENCRYPT;
    }else if(uncrypted == total){
        return EncryptDecrypt::DECRYPT;
    }else{
        return EncryptDecrypt::PARTIAL;
    }
}

FilesAndSize FilesEncrypt::getFilesFromDirRecursive(QDir const& dir){
    QFileInfoList objects = dir.entryInfoList(QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs);
    QStringList files;
    quint64 size{0};
    foreach(auto object, objects){
        if(object.isDir()){
           //qDebug() << "Entering recursive " << object.absoluteFilePath();
           FilesAndSize f_tmp{getFilesFromDirRecursive(QDir{object.absoluteFilePath()})};
           size += f_tmp.size;
           files.append(f_tmp.files);
        }else{
            files.append(object.absoluteFilePath());
            size += object.size();
        }
    }
    FilesAndSize f;
    f.files = files;
    f.size = size;
    return f;
}

QByteArray FilesEncrypt::getEncryptBlob(const char* iv, quint32 version, bool filenameChanged, const char* newFilename, int newFilename_size){
    QByteArray content{};
    content.append(compare);
    content.append("V");
    content.append(QString::number(version));
    content.append(";");
    content.append(iv, AES_BLOCK_SIZE);
    content.append(QString::number(filenameChanged));
    content.append(QString::number(newFilename_size));
    content.append(";");
    content.append(newFilename, newFilename_size);
    return content;
}

EncryptDecrypt_s FilesEncrypt::guessEncrypted(QByteArray const& content){
    QByteArray header = content.mid(0, SIZE_BEFORE_CONTENT); // Be sure we check the right size

    EncryptDecrypt_s state;
    state.state = DECRYPT;
    state.version = 1;
    state.offsetBeforeContent = 0;
    state.filenameChanged = false;

    if(header.mid(0, COMPARE_SIZE) != compare){
        return state;
    }

    state.state = EncryptDecrypt::ENCRYPT;

    header = header.mid(COMPARE_SIZE);

    if(header.mid(AES_BLOCK_SIZE, COMPARE_SIZE) == compare){ // There is not a version
        state.iv = header.mid(0, AES_BLOCK_SIZE);
        state.offsetBeforeContent = COMPARE_SIZE * 2 + AES_BLOCK_SIZE;
    }else{
        auto ivIndex = header.indexOf(';') + 1;
        auto version{header.mid(1, ivIndex - 2).toInt()};
        state.iv = header.mid(ivIndex, AES_BLOCK_SIZE);

        header = header.mid(ivIndex + AES_BLOCK_SIZE); // Begins at filenameChanged
        state.filenameChanged = header.mid(0, 1).toInt();

        header = header.mid(1);
        auto filenameIndex = header.indexOf(';') + 1;
        auto filenameSize{header.mid(0, filenameIndex - 1).toInt()};

        header = header.mid(filenameIndex);
        state.newFilename = header.mid(0, filenameSize);
        header = header.mid(filenameIndex + filenameSize);

        state.version = version;
        state.offsetBeforeContent = COMPARE_SIZE + ivIndex + AES_BLOCK_SIZE + 1 + filenameIndex + filenameSize ;
    }


    assert(state.iv.size() == 16);
    return state;
}
