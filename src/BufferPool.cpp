/*
 * robin 2021-07-28
**/

#include "../include/BufferPool.h"

using namespace robin;

// singleton
BufferPool * BufferPool::instance()
{
	static BufferPool pool;
	return &pool;
}

void BufferPool::printPool()
{
	printf(" readGet: %ju\n readPut: %ju\n newRead: %ju\n", getNumRead, putNumRead, newNumRead.load());
	printf(" writeGet:%ju\n writePut:%ju\n newWrite:%ju\n", getNumWrite, putNumWrite, newNumWrite.load());
}

BufferPool::BufferPool() : getNumRead(0), putNumRead(0), newNumRead(0),
getNumWrite(0), putNumWrite(0), newNumWrite(0)
{
	// prepare some for speed
	{
		std::lock_guard<std::mutex> lockGuard(bufMutex);
		for (int i = 0; i < 1000; i++)
		{
			char * buf = new char[DEFAUTL_BUF_SZ];
			bufQueue.push_back(buf);
		}
	}

	std::lock_guard<std::mutex> guard(this->reqMutex);
	for (int i = 0; i < 1000; i++)
	{
		write_req_vec_t * buf = new write_req_vec_t();
		reqQueue.push_back(buf);
	}
	

}
BufferPool::~BufferPool()
{
	std::lock_guard<std::mutex> lockGuard(bufMutex);
	for (auto p : bufQueue)
	{
		delete[] p;
	}
	bufQueue.clear();
}

// free the buf to memory pool after read_callback
char * BufferPool::getReadBuffer(unsigned long  & len)
{
	// domain of unlock
	{
		std::lock_guard<std::mutex> lockGuard(bufMutex);
		getNumRead++;
		if (!bufQueue.empty())
		{
			char * buf = bufQueue[0];
			bufQueue.pop_front();
			len = DEFAUTL_BUF_SZ;
			return buf;
		}
	}

	//LOG_DEBUG("malloc read buffer 1 times.");
	// need to alloc
	char * buf = new char[DEFAUTL_BUF_SZ];
	newNumRead++;
	len = DEFAUTL_BUF_SZ;
	return buf;
}

void  BufferPool::putReadBuffer(char * buf)
{
	if (buf == nullptr)
		return;

	std::lock_guard<std::mutex> lockGuard(bufMutex);
	putNumRead++;
	bufQueue.push_back(buf);
}

write_req_vec_t * BufferPool::getWriteBuffer()
{
	{
		std::lock_guard<std::mutex> guard(this->reqMutex);
		if (reqQueue.size() > 0)
		{
			getNumWrite++;
			write_req_vec_t * buf = reqQueue[0];
			reqQueue.pop_front();
			return buf;
		}
	}

	newNumWrite++;
	//DEBUG_PRINT("malloc write buffer 1 times.");
	write_req_vec_t * buf = new write_req_vec_t();
	// ensure that buf is at least 1024, for speed; use constructor instead!
	//buf->vecBuf.reserve(DEFAULT_VEC_SZ);
	assert(buf->vecBuf.capacity() >= DEFAULT_VEC_SZ);
	return buf;

}
void BufferPool::putWriteBuffer(write_req_vec_t *buf)
{
	if (buf == nullptr)
		return;

	
	//DEBUG_PRINT("put write buffer 1 times.");
	buf->taskPtr.reset();

	std::lock_guard<std::mutex> guard(this->reqMutex);
	putNumWrite++;
	reqQueue.push_back(buf);
	
}

