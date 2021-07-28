#pragma once
#include <uv.h>
#include "../include/queue.h"

typedef struct mem_s
{
	mem_s()
	{
		buffer = new char[2048];
		nCap = 2048;
		nSize = 0;
	}
	~mem_s()
	{
		delete buffer;
	}
	void reset() { nSize = 0; }
	char *buffer;
	size_t nSize;
	size_t nCap;
	QUEUE node;
}mem_t;



class BufferQue
{
public:
	BufferQue()
	{
		QUEUE_INIT(&head);
		uv_mutex_init(&mutexQue);
	}

	~BufferQue()
	{
		uv_mutex_destroy(&mutexQue);
	}
	mem_t * getBuffer()
	{

		QUEUE * p;
		mem_t * ret;
		uv_mutex_lock(&mutexQue);

		p = QUEUE_HEAD(&head);
		if (p != &head)
		{
			QUEUE_REMOVE(p);
			ret = QUEUE_DATA(p, mem_t, node);
			assert(ret != nullptr);
			ret->reset();
		}
		else
		{
			ret = new mem_t();
		}
		uv_mutex_unlock(&mutexQue);
		return ret;
	}

	bool putBuffer(mem_t * buf)
	{

		if (buf == nullptr)
			return false;
		uv_mutex_lock(&mutexQue);
		QUEUE_INSERT_TAIL(&head, &(buf->node));
		uv_mutex_unlock(&mutexQue);
		return true;
	}

	private:
		QUEUE head;
		uv_mutex_t mutexQue;	
};

/*
void testBufferQue()
{
	BufferQue bufPool;
	mem_t * p = bufPool.getBuffer();
	bufPool.putBuffer(p);
	Timer timer;
	timer.start();
	for (int i = 0; i < 2000; i++)
	{
		mem_t * p = bufPool.getBuffer();
		bufPool.putBuffer(p);
	}

	double t = timer.stop_delta<Timer::ms>();
	cout << t << endl;
}
*/