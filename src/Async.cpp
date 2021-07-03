/*
   Copyright © 2017-2019, orcaer@yeah.net  All rights reserved.
   Author: orcaer@yeah.net
   Last modified: 2019-9-11
   Description: https://github.com/wlgq2/uv-cpp
   Memo:changed something by robin 2021-07-02
*/

#include "../include/Async.h"
#include "../include/EventLoop.h"

using namespace robin;

Async::Async(EventLoop * loop) : refLoop(loop), cbOnClosed(nullptr)
{
}

void Async::init()
{
	if (uvHandle.data != static_cast<void*>(this))
	{
		uv_async_init((uv_loop_t*)refLoop->handle(), &uvHandle, Async::uvCallback);
		uvHandle.data = static_cast<void*>(this);
		uv_async_send(&uvHandle);
	}
	
}

// nothing to free
Async::~Async()
{

}

void Async::runInLoop(DefaultCallback callback)
{
    {
        std::lock_guard<std::mutex> lock(queMutex);
        cbQue.push_back(callback);
    }
	// if init called, then schedule
	if (uvHandle.data == static_cast<void*>(this))
	{
		uv_async_send(&uvHandle);
	}
	else
	{

	}
}

void Async::realProcess()
{
	// for a shorten lock, use a temp que;
    std::deque<DefaultCallback> tmpCallbacks;
    {
        std::lock_guard<std::mutex> lock(queMutex);
        cbQue.swap(tmpCallbacks);
    }
	// then others can use queue as ever
    while (!tmpCallbacks.empty())
    {
        auto func = tmpCallbacks[0];
        func();
		tmpCallbacks.pop_front();
    }
}

void Async::close(Async::OnClosedCallback callback)
{
    this->cbOnClosed = callback;
    if (uv_is_closing((uv_handle_t*)&uvHandle) == 0)
    {
        uv_close((uv_handle_t*)&uvHandle, [](uv_handle_t* handle)
        {
            auto ptr = static_cast<Async*>(handle->data);

			// callback is called in onClosed()
            ptr->onClosed();
			// set to before inited
			handle->data = nullptr;
        });
    }
}

// callback is called in onClosed()
void Async::onClosed()
{
	if (nullptr != cbOnClosed)
	{
		cbOnClosed(this);
	}
}

EventLoop* Async::Loop()
{
    return this->refLoop;
}

void Async::uvCallback(uv_async_t* handle)
{
    auto async = static_cast<Async*>(handle->data);
	if (async)
		async->realProcess();
}


