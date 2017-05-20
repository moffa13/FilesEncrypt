#include "Crypt.h"
#include "utilities.h"
#include <cstdio>
#include <string>
#include "openssl/x509.h"
#include "openssl/rsa.h"
#include "openssl/pem.h"
#include "openssl/evp.h"
#include "openssl/aes.h"
#include "openssl/rand.h"
#include "openssl/err.h"
#include "Logger.h"
#include <chrono>
#include <ctime>
#include <ratio>
#include <QFile>
#include <QFileInfo>
#include <iostream>
#include <QThread>
#include <QCoreApplication>
#include <cassert>

#if defined(Q_OS_WIN32)
#include "openssl/applink.c"
#endif

#define MINIMUM_KEY_LENGTH 2048

using namespace std;

bool Crypt::paused = false;


Crypt::Crypt()
{
    init();
}

Crypt::~Crypt(){

}

void Crypt::init(){
    OpenSSL_add_all_algorithms();
    ERR_load_BIO_strings();
    // ERR_load_crypto_strings();
    // SSL_load_error_strings();
}


EVP_PKEY* Crypt::genRSA(int keyLength){
    Logger::info("Generating RSA keypair");
    //Private key container
    EVP_PKEY* prvKey = EVP_PKEY_new();
    // RSA
    auto t1 = chrono::high_resolution_clock::now();
    RSA* rsa = RSA_generate_key(keyLength, RSA_F4, NULL, NULL);
    auto t2 = chrono::high_resolution_clock::now();
    auto timeToGenRSAKeyPair = chrono::duration_cast<std::chrono::microseconds>(t2-t1);
    Logger::info("Took " + QString::number((double)(timeToGenRSAKeyPair.count() / double(1000000))) + "s");

    // Assign rsa to container
    EVP_PKEY_assign_RSA(prvKey, rsa);
    // Clean pointer
    rsa = nullptr;
    return prvKey;
}

void Crypt::writePrivateKey(EVP_PKEY* x, char* password) const{
    FILE* keyFile = fopen(m_key.c_str(), "wb");
    if(password == nullptr){
        PEM_write_PrivateKey(
           keyFile,
           x,
           NULL,
           NULL,
           0,
           NULL,
           NULL
        );
    }else{
        PEM_write_PrivateKey(
            keyFile,
            x,
            EVP_des_ede3_cbc(),
            reinterpret_cast<unsigned char *>(password),
            strlen(password),
            NULL,
            NULL
        );
    }

    fclose(keyFile);
}

void Crypt::writePublicKey(EVP_PKEY* x) const{
    FILE* keyFile = fopen(m_pbc_key.c_str(), "wb");
    PEM_write_PUBKEY(
       keyFile,
       x
    );
    fclose(keyFile);
}

void Crypt::genCert(CertInfos const &infos, int keyLength) const{

    if(keyLength < MINIMUM_KEY_LENGTH){
        Logger::warn("The key length is to small ("+QString::number(keyLength)+"). Must be at least " + MINIMUM_KEY_LENGTH + " bits");
    }

    Logger::info("Generating Cert file...");

    //Private key container
    EVP_PKEY* prvKey = genRSA(keyLength);

    // Create cert
    X509* x509 = X509_new();
    // Cert serial n°
    ASN1_INTEGER_set(X509_get_serialNumber(x509), 1);
    // Cert exp date
    X509_gmtime_adj(X509_get_notBefore(x509), 0);
    X509_gmtime_adj(X509_get_notAfter(x509), 1576800000);
    // Assign prvKey with cert
    X509_set_pubkey(x509, prvKey);
    // Cert infos
    X509_NAME* certInfos = X509_get_subject_name(x509);
    X509_NAME_add_entry_by_txt(certInfos, "C",  MBSTRING_ASC,
                               reinterpret_cast<const unsigned char *>(infos.country_code.c_str()), -1, -1, 0);
    X509_NAME_add_entry_by_txt(certInfos, "O",  MBSTRING_ASC,
                                reinterpret_cast<const unsigned char *>(infos.organisation_name.c_str()), -1, -1, 0);
    X509_NAME_add_entry_by_txt(certInfos, "CN", MBSTRING_ASC,
                                reinterpret_cast<const unsigned char *>(infos.website_url.c_str()), -1, -1, 0);
    X509_set_issuer_name(x509, certInfos);
    // Cert sign
    X509_sign(x509, prvKey, EVP_sha256());

    // char* strcopy = reinterpret_cast<char*>(malloc(strlen(infos.password.c_str()) + 1));
    // strcpy(strcopy, infos.password.c_str());
    //    PEM_write_PrivateKey(
    //        m_keyFile,
    //        prvKey,
    //        EVP_des_ede3_cbc(),
    //         reinterpret_cast<unsigned char *>(strcopy),
    //        9,
    //        NULL,
    //        NULL
    //    );

    // Write prvKey
    writePrivateKey(prvKey);
    EVP_PKEY_free(prvKey);

    Logger::info("Private key successfully written.");
    // Write cert
    FILE* certFile = fopen(m_cert.c_str(), "wb");
    PEM_write_X509(
        certFile,
        x509
    );
    fclose(certFile);
    X509_free(x509);

    Logger::info("Certificate successfully written.");
}

X509* Crypt::loadCert() const{
    X509* cert;
    BIO* bio;
    bio = BIO_new(BIO_s_file());
    BIO_read_filename(bio, m_cert.c_str());
    cert = PEM_read_bio_X509_AUX(bio, NULL, NULL, NULL);
    BIO_free(bio);
    return cert;
}

EVP_PKEY* Crypt::getPublicKeyFromCertificate(X509* cert){
    EVP_PKEY* pubKey = X509_get_pubkey(cert);
    return pubKey;
}

RSA* Crypt::getRSAFromEVP_PKEY(EVP_PKEY* pKey){
    return EVP_PKEY_get1_RSA(pKey);
}

RSA* Crypt::getPublicKeyFromFile() const{
    return getPublicKeyFromFile(m_pbc_key);
}


RSA* Crypt::getPublicKeyFromFile(string const &filename){
    FILE* keyFile = fopen(filename.c_str(), "rb");
    RSA* pbcKey = PEM_read_RSA_PUBKEY(keyFile, NULL, NULL, NULL);
    fclose(keyFile);
    return pbcKey;
}

RSA* Crypt::getPrivateKeyFromFile() const{
    return getPrivateKeyFromFile(m_key);
}

RSA* Crypt::getPrivateKeyFromFile(string const &filename){
    FILE* keyFile = fopen(filename.c_str(), "rb");
    RSA* prvKey = PEM_read_RSAPrivateKey(keyFile, NULL, NULL, NULL);
    fclose(keyFile);
    return prvKey;
}

int Crypt::encrypt(RSA* public_key, const unsigned char* message, int len, unsigned char* encrypted){
    int ret = RSA_public_encrypt(len, message, encrypted, public_key, RSA_PKCS1_OAEP_PADDING);
    if(ret == -1){
        ulong error = ERR_peek_last_error();
        Logger::error("Error while crypting RSA : " + QString(ERR_error_string(error, NULL)));
    }
    return ret;
}

int Crypt::decrypt(RSA* private_key, const unsigned char* encrypted, int len, unsigned char* message){
    int ret = RSA_private_decrypt(len, encrypted, message, private_key, RSA_PKCS1_OAEP_PADDING);
    if(ret == -1){
        ulong error = ERR_peek_last_error();
        Logger::error("Error while decrypting RSA : " + QString(ERR_error_string(error, NULL)));
    }
    return ret;
}

// Encrypt by buffer
void Crypt::aes_crypt(
        const unsigned char* uncrypted,
        int uncrypted_size,
        unsigned char* encrypted,
        const unsigned char* key,
        unsigned char* iv
){

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit(ctx, EVP_aes_256_cbc(), key, iv);

    qint64 read{0};
    int lastLength{0};
    unsigned pass{0};
    unsigned readPass{0};

    unsigned current_uncrypted_position{0};
    unsigned current_crypted_position{0};

    do{
        if(current_crypted_position + AES_BLOCK_SIZE - 1 < uncrypted_size){
            read = AES_BLOCK_SIZE;
        }else{
            read = uncrypted_size - current_uncrypted_position;
        }
        EVP_EncryptUpdate(ctx, encrypted + current_crypted_position, &lastLength, uncrypted + current_uncrypted_position, read);
        while(paused) QThread::msleep(100);
        current_uncrypted_position += read;
        current_crypted_position += lastLength;
        ++pass;
        readPass += read;
        if(pass >= 64){
            emit aes_encrypt_updated(readPass);
            pass = 0;
            readPass = 0;
        }
    }while(read == AES_BLOCK_SIZE);

    emit aes_encrypt_updated(readPass);

    // Writes the padding
    EVP_EncryptFinal(ctx, encrypted + current_crypted_position, &lastLength);
    EVP_CIPHER_CTX_free(ctx);
}

// Encrypt by stream
void Crypt::aes_crypt(QFile* file, QFile* tmpFile, const unsigned char* key, unsigned char* iv){
    unsigned char* buffer = reinterpret_cast<unsigned char*>(malloc(16));
    unsigned char* crypted = reinterpret_cast<unsigned char*>(malloc(16));

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_EncryptInit(ctx, EVP_aes_256_cbc(), key, iv);

    qint64 read{0};
    int lastLength{0};
    unsigned pass{0};
    unsigned readPass{0};

    while((read = file->read(reinterpret_cast<char*>(buffer), 16)) > 0){
        EVP_EncryptUpdate(ctx, crypted, &lastLength, buffer, read);
        tmpFile->write(reinterpret_cast<char*>(crypted), lastLength);
        while(paused) QThread::msleep(100);
        ++pass;
        readPass += read;
        if(pass >= 64){
            emit aes_encrypt_updated(readPass);
            pass = 0;
            readPass = 0;
        }

        memset(buffer, 0, 16);
        memset(crypted, 0, 16);
    }

    emit aes_encrypt_updated(readPass);

    // Writes the padding
    EVP_EncryptFinal(ctx, crypted, &lastLength);
    tmpFile->write(reinterpret_cast<char*>(crypted), lastLength);

    tmpFile->close();
    EVP_CIPHER_CTX_free(ctx);
    free(buffer);
    free(crypted);
}

// Decrypt by stream
void Crypt::aes_decrypt(QFile* file, QFile* tmpFile, const unsigned char* key, unsigned char* iv){

    unsigned char* buffer = reinterpret_cast<unsigned char*>(malloc(16));
    unsigned char* uncrypted = reinterpret_cast<unsigned char*>(malloc(32));

    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit(ctx, EVP_aes_256_cbc(), key, iv);


    qint64 read{0};
    int lastLength{0};
    unsigned pass{0};
    unsigned readPass{0};
    while((read = file->read(reinterpret_cast<char*>(buffer), 16)) > 0){

        EVP_DecryptUpdate(ctx, uncrypted, &lastLength, buffer, read);
        tmpFile->write(reinterpret_cast<char*>(uncrypted), lastLength);
        while(paused) QThread::msleep(100);
        ++pass;
        readPass += read;
        if(pass >= 64){
            emit aes_decrypt_updated(readPass);
            pass = 0;
            readPass = 0;
        }

        memset(buffer, 0, 16);
        memset(uncrypted, 0, 16);
    }

    emit aes_decrypt_updated(readPass);

    // Writes the padding
    EVP_DecryptFinal(ctx, uncrypted, &lastLength);
    tmpFile->write(reinterpret_cast<char*>(uncrypted), lastLength);

    tmpFile->close();

    free(buffer);
    free(uncrypted);
    EVP_CIPHER_CTX_free(ctx);
}

// Decrypt by buffer
void Crypt::aes_decrypt(
        const unsigned char* encrypted,
        int encrypted_size,
        unsigned char* uncrypted,
        const unsigned char* key,
        unsigned char* iv
){
    EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
    EVP_DecryptInit(ctx, EVP_aes_256_cbc(), key, iv);

    qint64 read{0};
    int lastLength{0};
    unsigned pass{0};
    unsigned readPass{0};

    unsigned current_uncrypted_position{0};
    unsigned current_crypted_position{0};

    do{
        if(current_crypted_position + AES_BLOCK_SIZE - 1 < encrypted_size){
            read = AES_BLOCK_SIZE;
        }else{
            read = encrypted_size - current_crypted_position;
        }
        EVP_DecryptUpdate(ctx, uncrypted + current_uncrypted_position, &lastLength, encrypted + current_crypted_position, read);
        while(paused) QThread::msleep(100);
        current_crypted_position += read;
        current_uncrypted_position += lastLength;
        ++pass;
        readPass += read;
        if(pass >= 64){
            emit aes_decrypt_updated(readPass);
            pass = 0;
            readPass = 0;
        }
    }while(read == AES_BLOCK_SIZE);

    emit aes_decrypt_updated(readPass);

    // Writes the padding
    EVP_DecryptFinal(ctx, uncrypted + current_uncrypted_position, &lastLength);
    EVP_CIPHER_CTX_free(ctx);
}

void Crypt::genAES(AESSIZE length, unsigned char* p){
    RAND_bytes(p, length);
}

void Crypt::genRandomIV(unsigned char* p){
    memset(p, 0, AES_BLOCK_SIZE);
    RAND_bytes(p, AES_BLOCK_SIZE);
}

bool Crypt::checkCert(){
    bool integrityOk = false;
    bool foundOnDisk = false;
    EVP_PKEY* pubKey;
    RSA* pubKeyRSA;
    if(m_certsExists){
        foundOnDisk = true;
        Logger::info("Key and cert found on disk, checking integrity...");

        X509* certificate = loadCert();
        if(certificate == NULL){
            Logger::error("Certificate corrupted");
            integrityOk = false;
            goto end;
        }

        pubKey = getPublicKeyFromCertificate(certificate);
        pubKeyRSA = getRSAFromEVP_PKEY(pubKey);

        RSA* prvKey = getPrivateKeyFromFile();

        Logger::info("The RSA size is "+ QString::number( RSA_size(pubKeyRSA) * 8) + " bits length");
        if(prvKey == NULL){
            Logger::error("Private key corrupted");
            integrityOk = false;
            goto end;
        }

        if(!BN_cmp(pubKeyRSA->n, prvKey->n)){
            Logger::info("Keys pair modulus matches");
             integrityOk = true;
        }else{
            Logger::error("Keys pair modulus does not match");
        }

        RSA_free(prvKey);
        X509_free(certificate);

    }
end:
    if(!integrityOk && foundOnDisk){
        Logger::error("... Integrity problem, needs to regenerate");
        return false;
    }else if(!integrityOk && !foundOnDisk){
        Logger::info("Certificate generating");
        return false;
    }else{
        Logger::info("... Integrity OK");
        Logger::info("Server public key is : ");
        PEM_write_PUBKEY(stdout, pubKey);
        pubKey = NULL;
        RSA_free(pubKeyRSA);
        return true;
    }

}
