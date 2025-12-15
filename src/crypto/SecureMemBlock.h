#ifndef SECUREMEMBLOCK_H
#define SECUREMEMBLOCK_H

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <QtGlobal>

#ifndef Q_OS_WIN
#include <QMap>
#include <memory>
#include <functional>
#endif

/**
 * Allocates a safe memory block to store sensitive data
 * On Windows, it secures memory using CryptProtectMemory/CryptUnProtectMemory
 * With linux, it creates one single aes key for all the program's lifetime
 * which is allocated using gcry_malloc_secure and encrypts the data with this key
 * using a random iv stored in a static List until the program exits
 * @brief The SecureMemBlock class
 */
class SecureMemBlock
{
	public:
		SecureMemBlock(const unsigned char* data, size_t len, bool encrypted = false);
		~SecureMemBlock();
		void secure();
		const unsigned char* getData();
		const unsigned char* getDataNoAction() const;
		inline size_t getLen() const {return _len;}
private:
		unsigned char* _enc_data;
#ifdef Q_OS_LINUX
		static QByteArray getMD5(const unsigned char *p, size_t len);
		static std::unique_ptr<unsigned char, std::function<void(unsigned char*)>> _aes;
		static QMap<QByteArray, QByteArray> _ivs;
		unsigned char* _iv;
#else
		static size_t getMultipleSize(size_t len, size_t multiple);
#endif
        size_t _len;
		bool _encrypted;
};

#endif // SECUREMEMBLOCK_H
