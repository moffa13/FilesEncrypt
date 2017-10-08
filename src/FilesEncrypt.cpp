#include <QFile>
#include <QFileInfo>
#include <iostream>
#include <QtDebug>
#include <QTemporaryFile>
#include <QThread>
#include <QTimer>
#include <QSettings>
#include "FilesEncrypt.h"
#include "utilities.h"
#include "openssl/err.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "Logger.h"
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <cassert>
#include "ui/SettingsWindow.h"
#include <QApplication>
#include <QStack>

#define TIME_MIN_REMOVE_AES 3
#define CURRENT_VERSION 2

// Current crpyts number
unsigned FilesEncrypt::s_pendingCrypt = 0;
QMutex FilesEncrypt::s_mutex;
const char FilesEncrypt::compare[] = {'E', 0x31, 'N', 0x31, 'C', 0x31, 'R', 0x31, 'Y', 0x31, 'P', 0x31, 'T', 0x31, 'E', 0x31, 'D', 0x31};
const size_t FilesEncrypt::COMPARE_SIZE = sizeof(compare)/sizeof(*compare);
const size_t FilesEncrypt::VERSION_LENGTH = 1 + 5 + 1; // V00000;
const size_t FilesEncrypt::SIZE_BEFORE_CONTENT = COMPARE_SIZE + AES_BLOCK_SIZE + VERSION_LENGTH + 1 + 8 + 256;
const char FilesEncrypt::PRIVATE_ENCRYPT_AES_SEPARATOR = 0x10;

/**
 * Constructs from a key path string
 */
FilesEncrypt::FilesEncrypt(std::string key_file) : m_key_file{std::move(key_file)}{
    init();
    m_key_file_loaded = readFromFile();
}

/**
 * Directly constructs via a decrypted aes key
 * @param aes the 32-bytes aes key
 */
FilesEncrypt::FilesEncrypt(const char* aes){
    init();
    setAES(aes);
}

/**
 * Allocates some vars and sets up the timer to delete the decrypted aes after TIME_MIN_REMOVE_AES. It DOES not start it
 * @brief FilesEncrypt::init
 */
void FilesEncrypt::init(){
    m_aes_crypted = reinterpret_cast<unsigned char*>(malloc(512));
    m_aes_decrypted = reinterpret_cast<unsigned char*>(malloc(32));

    m_deleteAESTimer.setSingleShot(true);
    connect(&m_deleteAESTimer, &QTimer::timeout, [this](){
        if(s_pendingCrypt == 0){
            unsetAES();
            emit keyEncrypted();
            Logging::Logger::debug("AES key deleted from ram");
        }else{
            Logging::Logger::warn("Impossible to remove the key, already crypting/decrypting" + QString::number(s_pendingCrypt) + " file(s)");
            startDeleteAesTimer();
        }
    });
}


bool FilesEncrypt::isValidKey(QFile &f){
    if(QString::fromLocal8Bit(f.read(37)) == "-----BEGIN ENCRYPTED PRIVATE KEY-----")
        return true;
    return false;
}

FilesEncrypt::~FilesEncrypt(){
    m_deleteAESTimer.stop();
    free(m_aes_crypted);
    free(m_aes_decrypted);
}

void FilesEncrypt::addPendingCrypt(){
    QMutexLocker locker{&s_mutex};
    ++s_pendingCrypt;
}

void FilesEncrypt::removePendingCrypt(){
    QMutexLocker locker{&s_mutex};
    --s_pendingCrypt;
}

unsigned FilesEncrypt::getPendingCrypt(){
    QMutexLocker locker{&s_mutex};
    return s_pendingCrypt;
}

void FilesEncrypt::setAES(const char* aes){
    m_aes_decrypted_set = true;
    memcpy(m_aes_decrypted, aes, 32);
}

/**
 * Fills the decrypted aes with zeros
 * @brief FilesEncrypt::unsetAES
 */
void FilesEncrypt::unsetAES(){
    m_aes_decrypted_set = false;
    memset(m_aes_decrypted, 0, 32);
}

const unsigned char* FilesEncrypt::getAES() const{
    return m_aes_decrypted;
}

/**
 * Makes a 256 bits aes-cbc key encrypted with rsa by a password
 * Private key locked with a password and encrypted aes with public key are stored in the file
 * @param file The file where we write the keys
 * @param password The password which will lock the private key
 * @param aes_copy If not nullptr, instead of generating a new aes, it copies it
 * @return true if everything happend right
 */
bool FilesEncrypt::genKey(QString const& file, QString const& password, const unsigned char* aes_copy){
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
    if(aes_copy == nullptr){
        Crypt::genAES(AESSIZE::S256, aes);
    }else{
        memcpy(aes, aes_copy, 32);
    }

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
        Logging::Logger::debug("Key successfully created");
        success = true;
    }else{
        Logging::Logger::debug("Can't write aes key to file");
    }

    free(aes);
    free(aes_encrypted);
    free(rsaBuff);

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
        Logging::Logger::error("Cannot retrieve key");
        f.close();
        return false;
    }

    // Get separated the private key and the aes-crypted key
    QByteArray arr{f.readAll()};
    int split = arr.indexOf(PRIVATE_ENCRYPT_AES_SEPARATOR);
    QByteArray private_key = arr.mid(0, split);
    QByteArray aes_crypted = arr.mid(split + 1, -1);

    m_aes_crypted_length = aes_crypted.length();

    memcpy(reinterpret_cast<void*>(m_aes_crypted), aes_crypted.constData(), aes_crypted.length()); // Save the crypted aes
    m_private_key_crypted = private_key.toStdString(); // Save the crypted private key

    Logging::Logger::debug("Crypted aes and crypted private key saved");

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
    container = PEM_read_bio_PrivateKey(bio, NULL, NULL, const_cast<char*>(password.c_str()));
    if(container == nullptr){
        if(passOk != nullptr)
            *passOk = false;
        Logging::Logger::error("Incorrect password or something else");
        goto end;
    }else{
        if(passOk != nullptr)
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
            emit keyDecrypted();
            Logging::Logger::debug("AES successfully decrypted");
            m_aes_decrypted_set = true;
            startDeleteAesTimer();

        }else{
            Logging::Logger::debug("AES not successfully decrypted");
        }
    }else{
        Logging::Logger::error("AES already decrypted");
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
    m_deleteAESTimer.start(1000 * 60 * TIME_MIN_REMOVE_AES);
}


/**
 * @brief FilesEncrypt::encryptFile
 * @param file The file must be open
 * @param op
 * @param filenameNeedsEncryption
 * @return
 */
finfo_s FilesEncrypt::encryptFile(QFile* file, EncryptDecrypt op, bool filenameNeedsEncryption) const{

    file->seek(0);

    finfo_s result;
    result.state = op;
    result.success = false;
    result.offsetBeforeContent = 0;

    // Create a temporary file
    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(true);
    tmpFile.open();

    // The new name the file will have (absolute file name)

    QFileInfo fileInfo{file->fileName()};
    QString name{fileInfo.absoluteFilePath()};

    // Connect signals
    Crypt crypt;
    connect(&crypt, SIGNAL(aes_decrypt_updated(qint32)), this, SIGNAL(encrypt_updated(qint32)));
    connect(&crypt, SIGNAL(aes_encrypt_updated(qint32)), this, SIGNAL(decrypt_updated(qint32)));

    EncryptDecrypt_s fileState{guessEncrypted(*file)};

    // If there is no action to do to the file
    if(op == fileState.state){
        Logging::Logger::warn("Trying to encrypt/decrypt a file maybe already encrypted/uncrypted");
        goto end;
    }

    // Tell the class that one more object is being used
    addPendingCrypt();


    if(op == EncryptDecrypt::ENCRYPT){

        file->seek(0);

        // Gen IV
        unsigned char* iv = reinterpret_cast<unsigned char*>(malloc(AES_BLOCK_SIZE));
        Crypt::genRandomIV(iv);


        QString nameWithoutPath{fileInfo.fileName()};

        unsigned char* encrypted_filename = nullptr;

        if(filenameNeedsEncryption){

            {
                QString newName;

                do{
                    newName = "/" + utilities::randomString(15);
                }while(QFile::exists(fileInfo.absolutePath() + newName + ".filesencrypt"));

                name = fileInfo.absolutePath() + newName + ".filesencrypt";
            }

            encrypted_filename = reinterpret_cast<unsigned char*>(malloc(getEncryptedSize(nameWithoutPath.length())));
            crypt.aes_crypt(reinterpret_cast<const unsigned char*>(nameWithoutPath.toStdString().c_str()), nameWithoutPath.length(), encrypted_filename, m_aes_decrypted, iv);
        }

        // Add Header
        auto blob = getEncryptBlob(reinterpret_cast<char*>(iv), CURRENT_VERSION, filenameNeedsEncryption, reinterpret_cast<const char*>(encrypted_filename), getEncryptedSize(nameWithoutPath.length()));
        result.offsetBeforeContent = blob.size();

        QByteArray fileContentEncrypted{blob};
        tmpFile.write(fileContentEncrypted);

        free(encrypted_filename);


        // Save final size
        quint64 futureSize = getEncryptedSize(file->size()) + blob.size();
        result.size = futureSize;

        // Crypt data
        crypt.aes_crypt(file, &tmpFile, m_aes_decrypted, iv);

        free(iv);
        iv = nullptr;

    }else{

        // Gen IV
        file->seek(fileState.offsetBeforeContent);
        result.size = crypt.aes_decrypt(file, &tmpFile, m_aes_decrypted, const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(fileState.iv.constData())));

        // If there is a new file name to apply
        if(!Crypt::isAborted() && fileState.filenameChanged){
            char* uncrypted_filename = reinterpret_cast<char*>(malloc(fileState.newFilename.size()));
            auto nameSize = crypt.aes_decrypt(
                        reinterpret_cast<const unsigned char*>(fileState.newFilename.constData()),
                        fileState.newFilename.size(),
                        reinterpret_cast<unsigned char*>(uncrypted_filename),
                        m_aes_decrypted,
                        const_cast<unsigned char*>(reinterpret_cast<const unsigned char*>(fileState.iv.constData()))
            );

            QByteArray str(uncrypted_filename, nameSize);
            name = fileInfo.absolutePath() + "/" + QString::fromLocal8Bit(str);
            free(uncrypted_filename);
        }
    }

    result.success = !Crypt::isAborted();

    if(result.success){
        result.name = name;

        qDebug() << "Future name : " << result.name;
        qDebug() << "Future size : " << result.size;
        qDebug() << "Future state : " << result.state;


        file->remove();
        tmpFile.copy(name);
    }else{
        result.state = fileState.state;
    }

    removePendingCrypt();
    emit file_done();
end:
    return result;
}

EncryptDecrypt_s FilesEncrypt::guessEncrypted(QFile& file){
    QByteArray content = file.read(SIZE_BEFORE_CONTENT);
    return FilesEncrypt::guessEncrypted(content);
}

size_t FilesEncrypt::getEncryptedSize(int message_length){
    return (message_length / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE;
}

EncryptDecrypt FilesEncrypt::guessEncrypted(QDir const& dir){

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
    QDir::Filters entryFlags{QDir::NoDotAndDotDot | QDir::Files | QDir::Dirs | QDir::Hidden};
    QFileInfoList objects = dir.entryInfoList(entryFlags);
    QStringList files;
    quint64 size{0};
    QStack<QFileInfo> stack;

    for(QFileInfo const& fInfo : objects){
        stack.push(fInfo);
    }

    while(!stack.isEmpty()){
        QFileInfo info{stack.pop()};
        if(!info.isDir()){
            files.append(info.absoluteFilePath());
            size += info.size();
        }else{
            QFileInfoList newFiles{QDir{info.absoluteFilePath()}.entryInfoList(entryFlags)};
            for(QFileInfo const& file : newFiles){
                stack.push(file);
            }
        }
    }

    FilesAndSize f;
    f.size = size;
    f.files = files;

    return f;
}

QByteArray FilesEncrypt::getEncryptBlob(const char* iv, quint32 version, bool filenameChanged, const char* newFilename, int newFilename_size){
    if(!filenameChanged) newFilename_size = 0;
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

    if(memcmp(header.mid(0, COMPARE_SIZE).constData(), &compare[0], COMPARE_SIZE) != 0){
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
