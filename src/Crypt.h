#ifndef SSL_H
#define SSL_H

#include <cstdio>
#include <string>
#include "openssl/x509.h"
#include <QFile>

typedef struct CertInfos CertInfos;
struct CertInfos{
    std::string country_code;
    std::string organisation_name;
    std::string website_url;
    std::string password;
};

enum AESSIZE{
    S128 = 16,
    S192 = 24,
    S256 = 32
};

class Crypt : public QObject
{

    Q_OBJECT

    public:
        Crypt();
        ~Crypt();
        bool checkCert();
        void genCert(CertInfos const &infos, int keyLength = 2048) const;
        RSA* getPublicKeyFromFile() const;
        RSA* getPrivateKeyFromFile() const;
        static RSA* getPrivateKeyFromFile(std::string const& filename);
        static RSA* getPublicKeyFromFile(std::string const& filename);
        static int encrypt(RSA* public_key, const unsigned char* message, int len, unsigned char* encrypted);
        static int decrypt(RSA* private_key, const unsigned char* encrypted, int len, unsigned char* message);
        static EVP_PKEY* genRSA(int keyLength);
        void writePrivateKey(EVP_PKEY* x, char* password = NULL) const;
        void writePublicKey(EVP_PKEY* x) const;
        static void genAES(AESSIZE length, unsigned char* p);
        static void genRandomIV(unsigned char* p);
        int aes_decrypt(const unsigned char *encrypted, int encrypted_size, unsigned char *uncrypted, const unsigned char* key, unsigned char* iv);
        void aes_crypt(const unsigned char *uncrypted, int uncrypted_size, unsigned char *encrypted, const unsigned char* key, unsigned char* iv);
        int aes_decrypt(QFile* file, QFile* tmpFile, const unsigned char* key, unsigned char* iv);
        void aes_crypt(QFile* file, QFile* tmpFile, const unsigned char* key, unsigned char* iv);
        std::string m_key;
        std::string m_cert;
        std::string m_pbc_key;
        bool m_certsExists;
        X509* loadCert() const;
        static EVP_PKEY* getPublicKeyFromCertificate(X509* cert);
        static RSA* getRSAFromEVP_PKEY(EVP_PKEY* pKey);
        static void init();
        static bool paused;

    Q_SIGNALS:
        void aes_decrypt_updated(qint32 progress);
        void aes_encrypt_updated(qint32 progress);


};

#endif // SSL_H
