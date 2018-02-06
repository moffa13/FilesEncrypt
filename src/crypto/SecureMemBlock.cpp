#include "SecureMemBlock.h"
#include <QtGlobal>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif

#ifdef Q_OS_LINUX
#include <gcrypt.h>
#include <openssl/sha.h>
#include "Crypt.h"
#include "FilesEncrypt.h"
#endif

#ifdef Q_OS_LINUX
std::unique_ptr<unsigned char, std::function<void(unsigned char*)>> SecureMemBlock::_aes{
	nullptr,
	[](unsigned char* ptr){
        gcry_free(ptr);
	}
};
QMap<QByteArray, QByteArray> SecureMemBlock::_ivs;
#endif


SecureMemBlock::SecureMemBlock(const unsigned char *data, size_t len, bool encrypted) : _len(len), _encrypted(true){

#ifdef Q_OS_LINUX
	// Create an encryption key valid until the program's lifetime ends
	if(_aes == nullptr){
        _aes.reset(reinterpret_cast<unsigned char*>(gcry_malloc_secure(32)));
		Crypt::genAES(AESSIZE::S256, _aes.get());
	}
	// Allocate an IV for each block
	_iv = reinterpret_cast<unsigned char*>(malloc(AES_BLOCK_SIZE));
#endif

#ifdef Q_OS_WIN
	const size_t alloc_size = getMultipleSize(_len, CRYPTPROTECTMEMORY_BLOCK_SIZE);
#else
	const size_t alloc_size = encrypted ? len : FilesEncrypt::getEncryptedSize(len);
#endif

	_enc_data = reinterpret_cast<unsigned char*>(malloc(alloc_size));

	memcpy(reinterpret_cast<void*>(_enc_data), reinterpret_cast<const void*>(data), _len);

	if(!encrypted){
#ifdef Q_OS_WIN
		CryptProtectMemory(_enc_data, alloc_size, CRYPTPROTECTMEMORY_SAME_PROCESS);
#else
		// Create an IV for each block
		Crypt::genRandomIV(_iv);
		secure();
		_ivs[getMD5(_enc_data, FilesEncrypt::getEncryptedSize(len))] = QByteArray{reinterpret_cast<char*>(_iv), AES_BLOCK_SIZE};
#endif
	}else{
#ifndef Q_OS_WIN
		memcpy(_iv, _ivs[getMD5(data, len)].constData(), AES_BLOCK_SIZE);
#endif
	}
}

#ifndef Q_OS_WIN
QByteArray SecureMemBlock::getMD5(const unsigned char* p, size_t len){
	unsigned char* sha1 = reinterpret_cast<unsigned char*>(malloc(SHA_DIGEST_LENGTH));
	SHA1(p, len, sha1);
	QByteArray res{reinterpret_cast<const char*>(p), SHA_DIGEST_LENGTH};
	free(sha1);
	return res;
}
#endif

SecureMemBlock::~SecureMemBlock(){
#ifdef Q_OS_WIN
	SecureZeroMemory(_enc_data, getMultipleSize(_len, CRYPTPROTECTMEMORY_BLOCK_SIZE));
#else
	memset(_enc_data, 0, _len);
	free(_iv);
#endif

	free(_enc_data);
}

#ifdef Q_OS_WIN
size_t SecureMemBlock::getMultipleSize(size_t len, size_t multiple){
	const uint8_t mod = len % multiple;
	return mod ? len + multiple - mod : len;
}
#endif

const unsigned char* SecureMemBlock::getData(){
	if(_encrypted){
#ifdef Q_OS_WIN
		CryptUnprotectMemory(_enc_data, getMultipleSize(_len, CRYPTPROTECTMEMORY_BLOCK_SIZE), CRYPTPROTECTMEMORY_SAME_PROCESS);
#else
		Crypt c;
		unsigned char* uncrypted = reinterpret_cast<unsigned char*>(gcry_malloc_secure(_len));
		_len = c.aes_decrypt(_enc_data, _len, uncrypted, _aes.get(), _iv, false);
		memcpy(_enc_data, uncrypted, _len);
		memset(uncrypted, 0, _len);
		gcry_free(uncrypted);
#endif
	}
	_encrypted = false;
	return _enc_data;
}

const unsigned char *SecureMemBlock::getDataNoAction() const{
	return _enc_data;
}

void SecureMemBlock::secure(){
#ifdef Q_OS_WIN
	CryptProtectMemory(_enc_data, getMultipleSize(_len, CRYPTPROTECTMEMORY_BLOCK_SIZE), CRYPTPROTECTMEMORY_SAME_PROCESS);
#else
	Crypt c;
	size_t futureSize = FilesEncrypt::getEncryptedSize(_len);
	unsigned char* encrypted = reinterpret_cast<unsigned char*>(gcry_malloc_secure(futureSize));
	c.aes_crypt(_enc_data, _len, encrypted, _aes.get(), _iv, false);
	_len = futureSize;
	memcpy(_enc_data, encrypted, _len);
	memset(encrypted, 0, _len);
	gcry_free(encrypted);
#endif
	_encrypted = true;
}
