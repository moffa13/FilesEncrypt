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
#include "openssl/aes.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "Logger.h"

#define TIME_MIN_REMOVE_AES 3

constexpr char compare[] = {'E', 0x31, 'N', 0x31, 'C', 0x31, 'R', 0x31, 'Y', 0x31, 'P', 0x31, 'T', 0x31, 'E', 0x31, 'D', 0x31};
constexpr size_t COMPARE_SIZE = sizeof(compare)/sizeof(*compare);
constexpr size_t SIZE_BEFORE_CONTENT = COMPARE_SIZE * 2 + AES_BLOCK_SIZE;


// Current crpyts number
unsigned FilesEncrypt::m_pendingCrypt = 0;
QMutex FilesEncrypt::m_mutex;


FilesEncrypt::FilesEncrypt(std::string const &key_file) : m_key_file(key_file){
    init();
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

const unsigned char* FilesEncrypt::getAES() const{
    return m_aes_decrypted;
}

unsigned FilesEncrypt::getPendingCrypt()
{
    return m_pendingCrypt;
}

bool FilesEncrypt::genKey(QString const& file, QString const& password){
    bool success = false;

    // Vars to be unallocated after
    unsigned char* aes = NULL;
    unsigned char* aes_encrypted = NULL;
    char* rsaBuff = NULL;
    unsigned char* passwordNew = NULL;
    BIO *bio = NULL;
    EVP_PKEY* rsa = NULL;
    RSA* public_key = NULL;

    // Gen rsa
    rsa = Crypt::genRSA(4096);
    public_key = Crypt::getRSAFromEVP_PKEY(rsa);

    // Write private key encrypted to <rsaStr>
    bio = BIO_new(BIO_s_mem());
    passwordNew = reinterpret_cast<unsigned char*>(malloc(password.length()));
    memcpy(passwordNew, password.toStdString().c_str(), password.length());
    PEM_write_bio_PrivateKey(
        bio,
        rsa,
        EVP_des_ede3_cbc(),
        passwordNew,
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
    aes = reinterpret_cast<unsigned char*>(malloc(AESSIZE::S256 + 1));
    Crypt::genAES(AESSIZE::S256, aes);

    // Encrypt aes
    aes_encrypted = reinterpret_cast<unsigned char*>(malloc(RSA_size(public_key)));
    Crypt::encrypt(public_key, aes, AESSIZE::S256, aes_encrypted);

    constexpr char sep = 0x10;

    QFile f(std::move(file));
    if(f.open(QFile::WriteOnly)){
        f.seek(0);
        f.write(rsaStr.c_str());
        f.write(&sep, 1);
        f.write(reinterpret_cast<char*>(aes_encrypted), RSA_size(public_key));
        f.close();
        Logger::info("Key successfully created");
        success = true;
    }else{
        Logger::info("Can't write aes key to file");
    }


    if(aes != NULL){
        free(aes);
    }
    if(aes_encrypted != NULL) {
        free(aes_encrypted);
    }
    if(rsaBuff != NULL) {
        free(rsaBuff);
    }
    if(passwordNew != NULL){
        free(passwordNew);
    }
    if(bio != NULL) {
        BIO_free(bio);
    }
    if(rsa != NULL) {
        EVP_PKEY_free(rsa);
    }
    if(public_key != NULL) {
        RSA_free(public_key);
    }

    return success;
}


bool FilesEncrypt::init(){
    QFile f(m_key_file.c_str());
    if(!f.exists() || !f.open(QFile::ReadOnly)){
        Logger::error("Cannot retrieve key");
        f.close();
        return false;
    }

    // Get separated the private key and the aes-crypted key
    QByteArray arr(f.readAll());
    int split = arr.indexOf(0x10);
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

bool FilesEncrypt::isAesUncrypted(){
    return m_aes_decrypted != NULL;
}

bool FilesEncrypt::requestAesDecrypt(std::string const& password, bool* passOk){

    bool success = false;
    BIO* bio = NULL;
    EVP_PKEY* container = NULL;
    RSA* private_key = NULL;
    char* passwordStr = NULL;


    // Write rsa to RSA container (RSA_st)
    bio = BIO_new(BIO_s_mem());
    BIO_write(bio, m_private_key_crypted.c_str(), m_private_key_crypted.length());
    container = EVP_PKEY_new();
    passwordStr = reinterpret_cast<char*>(malloc(password.length()));
    strcpy(passwordStr, password.c_str());
    container = PEM_read_bio_PrivateKey(bio, NULL, NULL, passwordStr);
    if(container == NULL){
        if(passOk != NULL)
            *passOk = false;
        Logger::error("Incorrect password or something else");
        goto end;
    }else{
        if(passOk != NULL)
            *passOk = true;
    }

    if(m_aes_decrypted == NULL){
        //free(passwordStr);
        private_key = Crypt::getRSAFromEVP_PKEY(container); // Save the private key
        m_aes_decrypted = reinterpret_cast<unsigned char*>(malloc(32));
        if(Crypt::decrypt(
            private_key,
            m_aes_crypted,
            m_aes_crypted_length,
            m_aes_decrypted
        ) != -1 ){
            success = true;
            Logger::info("AES successfully decrypted");
            startDeleteAesTimer();

        }else{
            Logger::info("AES not successfully decrypted");
        }


    }else{
        Logger::error("AES already decrypted");
        success = true;
    }

end:
    if(bio != NULL){
        BIO_free(bio);
    }
    if(container != NULL){
        EVP_PKEY_free(container);
    }
    if(private_key != NULL){
        RSA_free(private_key);
    }
    if(passwordStr != NULL){
        //free(passwordStr);
        // BUG BUG BUG
    }

    return success;

}

void FilesEncrypt::startDeleteAesTimer(){
    QTimer::singleShot(1000 * 60 * TIME_MIN_REMOVE_AES, [this](){
        if(m_pendingCrypt == 0){
            memset(m_aes_decrypted, 0, 32);
            delete m_aes_decrypted;
            m_aes_decrypted = NULL;
            Logger::info("AES key deleted from ram");
        }else{
            Logger::warn("Impossible to remove the key, already crypting/decrypting" + QString::number(m_pendingCrypt) + " file(s)");
            startDeleteAesTimer();
        }

    });
}

bool FilesEncrypt::encryptFile(QFile* file, EncryptDecrypt op){


    Logger::info("Thread " + QString::number(reinterpret_cast<int>(QThread::currentThreadId())) + " encrypting");

    bool success = false;

    QString filename = QFileInfo(*file).fileName();
    QByteArray fileContentEncrypted;
    Logger::info("File size : " + QString::number(file->size()) + " bytes");

    unsigned char* encrypted = NULL;
    unsigned char* iv = NULL;

    QTemporaryFile tmpFile;
    tmpFile.setAutoRemove(true);
    tmpFile.open();
    QString name(file->fileName());

    Crypt crypt;
    connect(&crypt, SIGNAL(aes_decrypt_updated(qint32)), this, SIGNAL(encrypt_updated(qint32)));
    connect(&crypt, SIGNAL(aes_encrypt_updated(qint32)), this, SIGNAL(decrypt_updated(qint32)));


    if(op == EncryptDecrypt::ENCRYPT){

        if(guessEncrypted(file->read(512)) == EncryptDecrypt::ENCRYPT){
            Logger::warn("Trying to encrypt a file maybe already encrypted");
            goto end;
        }

        addPendingCrypt();

        file->seek(0);

        // Gen IV
        iv = reinterpret_cast<unsigned char*>(malloc(AES_BLOCK_SIZE));
        Crypt::genRandomIV(iv);

        // Add Header E1N1C1R1Y1P1T1E1D1AAAAAAAAAAAAAAAAE1N1C1R1Y1P1T1E1D1AAAAA...
        Logger::info("Encrypting file " + filename);
        fileContentEncrypted.append(compare);
        fileContentEncrypted.append(reinterpret_cast<char*>(iv), AES_BLOCK_SIZE);
        fileContentEncrypted.append(compare);
        tmpFile.write(fileContentEncrypted);


        // Log final size
        quint64 futureSize = (file->size() / 16 + 1) * 16 + SIZE_BEFORE_CONTENT;
        Logger::info("Future file size will be " + utilities::speed_to_human(futureSize) + " bytes");

        // Crypt data

        crypt.aes_crypt(file, &tmpFile, m_aes_decrypted, iv);

        free(iv);

        removePendingCrypt();

        Logger::info("File " + filename + " encrypted");
        success = true;
    }else{

        if(guessEncrypted(file->read(512)) == EncryptDecrypt::DECRYPT){
            Logger::warn("Trying to decrypt a file maybe not encrypted");
            goto end;
        }

	addPendingCrypt();

        // Gen IV
	file->seek(COMPARE_SIZE);
        QByteArray ivB = file->read(AES_BLOCK_SIZE);
        iv = reinterpret_cast<unsigned char*>(ivB.data());
	QString msg = "";
	msg += "File's IV is ";
	for(quint8 i{0}; i < 16; ++i){
	    msg += QString::number(*(iv + i)) + " ";
	}
	Logger::info(msg.trimmed());


	file->seek(SIZE_BEFORE_CONTENT);
        crypt.aes_decrypt(file, &tmpFile, m_aes_decrypted, iv);

        success = true;
        Logger::info("File " + filename + " decrypted");
        removePendingCrypt();

    }


    file->remove();
    tmpFile.copy(name);

end:

    if(encrypted != NULL){
        free(encrypted);
    }

    return success;


}

EncryptDecrypt FilesEncrypt::guessEncrypted(QFile& file){
    QByteArray content = file.read(SIZE_BEFORE_CONTENT);
    return FilesEncrypt::guessEncrypted(content);
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
	    if(FilesEncrypt::guessEncrypted(file) == EncryptDecrypt::ENCRYPT){
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
           FilesAndSize f_tmp{getFilesFromDirRecursive(QDir(object.absoluteFilePath()))};
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

EncryptDecrypt FilesEncrypt::guessEncrypted(QByteArray const& content){
    QByteArray header = content.mid(0, COMPARE_SIZE); // Be sure we check the right size
    if(strcmp(header.constData(), &compare[0]) == 0){
        return EncryptDecrypt::ENCRYPT;
    }
    return EncryptDecrypt::DECRYPT;
}
