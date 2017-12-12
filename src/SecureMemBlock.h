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
#endif
		bool _encrypted;
		size_t _len;
		static size_t getMultipleSize(size_t len, size_t multiple);
};

#endif // SECUREMEMBLOCK_H
