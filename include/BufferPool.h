#pragma once
#include "CommonHeader.h"

namespace robin
{
// by default: one packet is shorter than this
	class ITask;

	// define a new type for uv_write, 
	// We can reuse it
	typedef struct write_req_vec : uv_write_t
	{
		write_req_vec() :vecBuf(DEFAULT_VEC_SZ)
		{

		}
		uv_buf_t buf;
		CharVector vecBuf;
		std::shared_ptr<ITask> taskPtr;
	} write_req_vec_t;

	// a very simple buffer pool
	class BufferPool
	{
	public:
		BufferPool();
		~BufferPool();
		char * getReadBuffer(unsigned long  & len);
		void   putReadBuffer(char * buf);

		write_req_vec_t * getWriteBuffer();
		void   putWriteBuffer(write_req_vec_t *buf);

		void   printPool();

		static BufferPool * instance();
	private:
		// statistic used
		uint64_t getNumRead;
		uint64_t putNumRead;
		atomic<uint64_t> newNumRead;

		uint64_t getNumWrite;
		uint64_t putNumWrite;
		atomic<uint64_t> newNumWrite;

		// thread safe, for alloc
		std::mutex bufMutex;
		std::deque<char *> bufQueue;

		// thread safe, for uv_write
		std::mutex reqMutex;
		std::deque< write_req_vec_t *> reqQueue;
	};
}

