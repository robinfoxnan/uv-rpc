/*
 * robin 2021-07-02
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
	printf(" readGet:%ju\n readPut:%ju\n newRead:%ju\n", getNum, putNum, newNum.load());
}

BufferPool::BufferPool() : getNum(0), putNum(0), newNum(0)
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

// free the buf to memory pool after read_callback��
char * BufferPool::getReadBuffer(unsigned long  & len)
{
	// domain of unlock
	{
		std::lock_guard<std::mutex> lockGuard(bufMutex);
		getNum++;
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
	newNum++;
	len = DEFAUTL_BUF_SZ;
	return buf;
}

void  BufferPool::putReadBuffer(char * buf)
{
	if (buf == nullptr)
		return;

	std::lock_guard<std::mutex> lockGuard(bufMutex);
	putNum++;
	bufQueue.push_back(buf);
}

write_req_vec_t * BufferPool::getWriteBuffer()
{
	{
		std::lock_guard<std::mutex> guard(this->reqMutex);
		if (reqQueue.size() > 0)
		{
			write_req_vec_t * buf = reqQueue[0];
			reqQueue.pop_front();
			return buf;
		}
	}

	LOG_DEBUG("malloc write buffer 1 times.");
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

	//LOG_DEBUG("put write buffer 1 times.");
	std::lock_guard<std::mutex> guard(this->reqMutex);
	buf->taskPtr.reset();
	reqQueue.push_back(buf);
	
}

