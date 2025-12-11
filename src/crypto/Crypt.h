#ifndef SSL_H
#define SSL_H

#include <cstdio>
#include <string>
#include <openssl/x509.h>
#include <QFile>
#include <QMutex>
#include "defines.h"

class Crypt : public QObject
{

	Q_OBJECT

	public:
		static RSA* getPrivateKeyFromFile(std::string const& filename);
		static RSA* getPublicKeyFromFile(std::string const& filename);
		static int encrypt(RSA* public_key, const unsigned char* message, int len, unsigned char* encrypted);
		static int decrypt(RSA* private_key, const unsigned char* encrypted, int len, unsigned char* message);
		static EVP_PKEY* genRSA(int keyLength);
		void writePrivateKey(EVP_PKEY* x, char* password = NULL) const;
		void writePublicKey(EVP_PKEY* x) const;
		static void genAES(AESSIZE length, unsigned char* p);
		static void genRandomIV(unsigned char* p);
		static void abort();
		static bool isPaused();
		static void setPaused(bool value);
		static bool isAborted();
		static void setAborted(bool value);
		int aes_decrypt(const unsigned char *encrypted, unsigned encrypted_size, unsigned char *uncrypted, const unsigned char* key, unsigned char* iv, bool threaded_mode = true);
		void aes_crypt(const unsigned char *uncrypted, unsigned uncrypted_size, unsigned char *encrypted, const unsigned char* key, unsigned char* iv, bool threaded_mode = true);
		int aes_decrypt(QFile* file, QFile* tmpFile, const unsigned char* key, unsigned char* iv);
		void aes_crypt(QFile* file, QFile* tmpFile, const unsigned char* key, unsigned char* iv);
		static EVP_PKEY* getPublicKeyFromCertificate(X509* cert);
		static RSA* getRSAFromEVP_PKEY(EVP_PKEY* pKey);
		static void init();
		static QByteArray userCrypt(const char *aes);
		static QByteArray userDecrypt(const char *crypted_aes, size_t size);
	private:
		static bool paused;
		static bool aborted;
		static QMutex s_mutex;
	Q_SIGNALS:
		void aes_decrypt_updated(qint32 progress);
		void aes_encrypt_updated(qint32 progress);
};

#endif // SSL_H
