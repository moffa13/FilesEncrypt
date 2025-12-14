#include "Crypt.h"
#include <cstdio>
#include <string>
#include "openssl/rsa.h"
#include "openssl/evp.h"
#include "openssl/aes.h"
#include "openssl/rand.h"
#include "Logger.h"
#include <ctime>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <QCoreApplication>
#include <cassert>
#include <QtDebug>
#include <QMutexLocker>
#include <utilities.h>

#ifdef Q_OS_WIN
#include <windows.h>
#include <wincrypt.h>
#endif

using namespace std;

bool Crypt::paused = false;
bool Crypt::aborted = false;

QMutex Crypt::s_mutex;

EVP_PKEY* Crypt::genRSA(int keyLength)
{
    Logging::Logger::debug("Generating RSA keypair");

    EVP_PKEY* pkey = nullptr;
    EVP_PKEY_CTX *ctx = nullptr;

    ctx = EVP_PKEY_CTX_new_from_name(nullptr, "RSA", nullptr);
    if (!ctx)
        throw std::runtime_error("EVP_PKEY_CTX_new_from_name failed");

    if (EVP_PKEY_keygen_init(ctx) <= 0)
        throw std::runtime_error("EVP_PKEY_keygen_init failed");

    if (EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, keyLength) <= 0)
        throw std::runtime_error("EVP_PKEY_CTX_set_rsa_keygen_bits failed");

    if (EVP_PKEY_keygen(ctx, &pkey) <= 0)
        throw std::runtime_error("EVP_PKEY_keygen failed");

    EVP_PKEY_CTX_free(ctx);
    return pkey;
}

int Crypt::encrypt(EVP_PKEY* public_key, const unsigned char* message, int inlen, unsigned char* encrypted, size_t& outlen){

    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(public_key, nullptr);

    if (EVP_PKEY_encrypt_init(ctx) <= 0) {
        utilities::logOpenSSLErrors();
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        utilities::logOpenSSLErrors();
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_encrypt(ctx, encrypted, &outlen, message, inlen) <= 0) {
        utilities::logOpenSSLErrors();
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    EVP_PKEY_CTX_free(ctx);
    return 0;
}

int Crypt::decrypt(EVP_PKEY* private_key, const unsigned char* encrypted, int inlen, unsigned char* message, size_t &outlen){
    EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(private_key, nullptr);

    if (EVP_PKEY_decrypt_init(ctx) <= 0) {
        utilities::logOpenSSLErrors();
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_CTX_set_rsa_padding(ctx, RSA_PKCS1_OAEP_PADDING) <= 0) {
        utilities::logOpenSSLErrors();
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    if (EVP_PKEY_decrypt(ctx, message, &outlen, encrypted, inlen) <= 0) {
        utilities::logOpenSSLErrors();
        EVP_PKEY_CTX_free(ctx);
        return -1;
    }

    EVP_PKEY_CTX_free(ctx);
    return 0;
}

// Encrypt by buffer
void Crypt::aes_crypt(
		const unsigned char* uncrypted,
		unsigned uncrypted_size,
		unsigned char* encrypted,
		const unsigned char* key,
		unsigned char* iv,
		bool threaded_mode
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
		current_uncrypted_position += read;
		current_crypted_position += lastLength;
		++pass;
		readPass += read;
		if(pass >= 2048){
			if(threaded_mode){
				if(isAborted()) return;
				while(isPaused()){
					if(isAborted()) return;
					QThread::msleep(100);
				}
			}
            Q_EMIT aes_encrypt_updated(readPass);
			pass = 0;
			readPass = 0;
		}

	}while(read == AES_BLOCK_SIZE);

    Q_EMIT aes_encrypt_updated(readPass);

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
		++pass;
		readPass += read;
		if(pass >= 2048){
			if(isAborted()) return;
			while(isPaused()){
				if(isAborted()) return;
				QThread::msleep(100);
			}
            Q_EMIT aes_encrypt_updated(readPass);
			pass = 0;
			readPass = 0;
		}

		memset(buffer, 0, 16);
		memset(crypted, 0, 16);
	}

    Q_EMIT aes_encrypt_updated(readPass);

	// Writes the padding
	EVP_EncryptFinal(ctx, crypted, &lastLength);
	tmpFile->write(reinterpret_cast<char*>(crypted), lastLength);

	tmpFile->close();
	EVP_CIPHER_CTX_free(ctx);
	free(buffer);
	free(crypted);
}

// Decrypt by stream
int Crypt::aes_decrypt(QFile* file, QFile* tmpFile, const unsigned char* key, unsigned char* iv){

	unsigned char* buffer = reinterpret_cast<unsigned char*>(malloc(16));
	unsigned char* uncrypted = reinterpret_cast<unsigned char*>(malloc(32));

	EVP_CIPHER_CTX* ctx = EVP_CIPHER_CTX_new();
	EVP_DecryptInit(ctx, EVP_aes_256_cbc(), key, iv);

	qint64 read{0};
	int lastLength{0};
	unsigned pass{0};
	unsigned readPass{0};
	unsigned uncrypted_size{0};

	while((read = file->read(reinterpret_cast<char*>(buffer), 16)) > 0){

		EVP_DecryptUpdate(ctx, uncrypted, &lastLength, buffer, read);
		uncrypted_size += lastLength;
		tmpFile->write(reinterpret_cast<char*>(uncrypted), lastLength);
		++pass;
		readPass += read;
		if(pass >= 2048){
			if(isAborted()) return -1;
			while(isPaused()){
				if(isAborted()) return -1;
				QThread::msleep(100);
			}
            Q_EMIT aes_decrypt_updated(readPass);
			pass = 0;
			readPass = 0;
		}

		memset(buffer, 0, 16);
		memset(uncrypted, 0, 16);
	}

    Q_EMIT aes_decrypt_updated(readPass);

	// Writes the padding
	EVP_DecryptFinal(ctx, uncrypted, &lastLength);
	uncrypted_size += lastLength;
	tmpFile->write(reinterpret_cast<char*>(uncrypted), lastLength);

	tmpFile->close();

	free(buffer);
	free(uncrypted);
	EVP_CIPHER_CTX_free(ctx);

	return uncrypted_size;
}

// Decrypt by buffer
int Crypt::aes_decrypt(
		const unsigned char* encrypted,
		unsigned encrypted_size,
		unsigned char* uncrypted,
		const unsigned char* key,
		unsigned char* iv,
		bool threaded_mode
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
		current_crypted_position += read;
		current_uncrypted_position += lastLength;
		++pass;
		readPass += read;
		if(pass >= 2048){
			if(threaded_mode){
				if(isAborted()) return -1;
				while(isPaused()){
					if(isAborted()) return -1;
					QThread::msleep(100);
				}
			}
            Q_EMIT aes_decrypt_updated(readPass);
			pass = 0;
			readPass = 0;
		}
	}while(read == AES_BLOCK_SIZE);

    Q_EMIT aes_decrypt_updated(readPass);

	// Writes the padding
	EVP_DecryptFinal(ctx, uncrypted + current_uncrypted_position, &lastLength);
	EVP_CIPHER_CTX_free(ctx);

	return current_uncrypted_position + lastLength;
}

void Crypt::genAES(AESSIZE length, unsigned char* p){
	RAND_bytes(p, length);
}

void Crypt::genRandomIV(unsigned char* p){
	memset(p, 0, AES_BLOCK_SIZE);
	RAND_bytes(p, AES_BLOCK_SIZE);
}

void Crypt::abort(){
	aborted = true;
}

bool Crypt::isPaused(){
	QMutexLocker locker{&s_mutex};
	return paused;
}

// SHOULD not be used in multi-threaded
void Crypt::setPaused(bool value){
	paused = value;
}

bool Crypt::isAborted(){
	QMutexLocker locker{&s_mutex};
	return aborted;
}

// SHOULD not be used in multi-threaded
void Crypt::setAborted(bool value){
	aborted = value;
}

QByteArray Crypt::userCrypt(const char* aes){
#ifdef Q_OS_WIN
	DATA_BLOB aes_blob;
	DATA_BLOB aes_crypted;
	aes_blob.cbData = 32;
	aes_blob.pbData = (BYTE*)aes;
	auto res = CryptProtectData(&aes_blob, NULL, NULL, NULL, NULL, 0, &aes_crypted);
	if(res){
		return QByteArray{reinterpret_cast<const char*>(aes_crypted.pbData), static_cast<int>(aes_crypted.cbData)};
	}else{
		throw std::runtime_error((QString{"Error user-decrypting the key "} +  QString::number(GetLastError())).toStdString().c_str());
	}
#endif
throw std::runtime_error("Not working under non-win OS");
}

QByteArray Crypt::userDecrypt(const char* crypted_aes, size_t size){
#ifdef Q_OS_WIN
	DATA_BLOB aes_blob;
	DATA_BLOB aes_crypted;
	aes_crypted.cbData = size;
	aes_crypted.pbData = (BYTE*)crypted_aes;
	auto res = CryptUnprotectData(&aes_crypted, NULL, NULL, NULL, NULL, 0, &aes_blob);
	if(res){
		return QByteArray{reinterpret_cast<const char*>(aes_blob.pbData), static_cast<int>(aes_blob.cbData)};
	}else{
		throw std::runtime_error((QString{"Error user-decrypting the key "} + QString::number(GetLastError())).toStdString().c_str());
	}
#endif
throw std::runtime_error("Not working under non-win OS");
}
