#include "SecureMemBlock.h"
#include <QtGlobal>
#ifdef Q_OS_WIN
#include <Windows.h>
#endif


SecureMemBlock::SecureMemBlock(const unsigned char *data, size_t len, bool encrypted) : _len(len), _encrypted(true){

#ifdef Q_OS_WIN
	const size_t alloc_size = getMultipleSize(_len, CRYPTPROTECTMEMORY_BLOCK_SIZE);
#else
	const size_t alloc_size = len;
#endif

	_enc_data = reinterpret_cast<unsigned char*>(malloc(alloc_size));

	memcpy(reinterpret_cast<void*>(_enc_data), reinterpret_cast<const void*>(data), _len);

	if(!encrypted){
#ifdef Q_OS_WIN
		CryptProtectMemory(_enc_data, getMultipleSize(_len, CRYPTPROTECTMEMORY_BLOCK_SIZE), CRYPTPROTECTMEMORY_SAME_PROCESS);
#endif
	}
}

SecureMemBlock::~SecureMemBlock(){
#ifdef Q_OS_WIN
	SecureZeroMemory(_enc_data, getMultipleSize(_len, CRYPTPROTECTMEMORY_BLOCK_SIZE));
#else
	memset(_enc_data, 0, _len);
#endif
	free(_enc_data);
}

size_t SecureMemBlock::getMultipleSize(size_t len, size_t multiple){
	const uint8_t mod = len % multiple;
	return mod ? len + multiple - mod : len;
}

const unsigned char* SecureMemBlock::getData(){

	if(_encrypted){
#ifdef Q_OS_WIN
		CryptUnprotectMemory(_enc_data, getMultipleSize(_len, CRYPTPROTECTMEMORY_BLOCK_SIZE), CRYPTPROTECTMEMORY_SAME_PROCESS);
#endif
	}
	return _enc_data;
}

void SecureMemBlock::secure(){
#ifdef Q_OS_WIN
	CryptProtectMemory(_enc_data, getMultipleSize(_len, CRYPTPROTECTMEMORY_BLOCK_SIZE), CRYPTPROTECTMEMORY_SAME_PROCESS);
#endif
}
