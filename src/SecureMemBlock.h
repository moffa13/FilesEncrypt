#ifndef SECUREMEMBLOCK_H
#define SECUREMEMBLOCK_H

#include <cstddef>
#include <cstdint>
#include <cstring>

class SecureMemBlock
{
	public:
		SecureMemBlock(const unsigned char* data, size_t len, bool encrypted = true);
		~SecureMemBlock();
		void secure();
		const unsigned char* getData();
		const unsigned char* getDataNoAction() const;
	private:
		unsigned char* _enc_data;
		bool _encrypted;
		size_t _len;
		size_t _enc_len;
		static size_t getMultipleSize(size_t len, size_t multiple);
};

#endif // SECUREMEMBLOCK_H
