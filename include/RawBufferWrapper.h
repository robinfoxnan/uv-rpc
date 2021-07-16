#pragma once
#include <assert.h>

class RawBufferWrapper
{
public:
	typedef char Ch;

	RawBufferWrapper(char * buffer, size_t n) : buf(buffer), nCap(n), nSize(0)
	{

	}
	~RawBufferWrapper() {}

	Ch Peek() const { assert(false); return '\0'; }
	Ch Take() { assert(false); return '\0'; }
	size_t Tell() const { return nSize; }

	Ch* PutBegin() { assert(false); return 0; }
	void Put(Ch c) { buf[nSize++] = c; }                     // 1
	void Flush() {  }                                        // 2
	size_t PutEnd(Ch* c) { buf[nCap - 1] = *c; return nSize; }

	void InitBuffer(size_t n)
	{
		nSize = 0;
	}
private:
	RawBufferWrapper(const RawBufferWrapper&) = delete;
	RawBufferWrapper& operator=(const RawBufferWrapper&) = delete;

	char * buf;
	size_t nCap;
	size_t nSize;
};