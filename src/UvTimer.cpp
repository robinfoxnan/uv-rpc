/*
   Copyright © 2017-2020, orcaer@yeah.net  All rights reserved.
   Author: orcaer@yeah.net
   Last modified: 2019-9-10
   Description: uv-cpp
*/

#include "../include/UvTimer.h"
#include "../include/EventLoop.h"

using namespace robin;

UvTimer::UvTimer(EventLoop * loop, uint64_t timeout_, uint64_t repeat_, TimerCallback callback)
    :bStarted(false),
    timeout(timeout_),
    repeat(repeat_),
	cbOnTimer(callback),
    cbClosed(nullptr)
{
    uvHandle.data = static_cast<void*>(this);
    int ret=  uv_timer_init(loop->handle(), &uvHandle);
	if (ret)
	{

	}
}

UvTimer::~UvTimer()
{
}

void UvTimer::start()
{
    if (false == bStarted)
    {
        bStarted = true;
        uv_timer_start(&uvHandle, UvTimer::uvOnTimer, timeout, repeat);
    }
}
void UvTimer::stop()
{
	if (uv_is_active((uv_handle_t*)&uvHandle))
	{
		uv_timer_stop(&uvHandle);
	}
	bStarted = false;
}

void UvTimer::uvOnTimer(uv_timer_t * handle)
{
	auto ptr = static_cast<UvTimer*>(handle->data);
	ptr->onTimer();
}

void UvTimer::onTimer()
{
	if (cbOnTimer)
	{
		cbOnTimer(this);
	}
}

void UvTimer::close(TimerClosedCallback callback)
{
    cbClosed = callback;
	stop();

    if (uv_is_closing((uv_handle_t*)&uvHandle) == 0)
    {
        uv_close((uv_handle_t*)&uvHandle,
            [](uv_handle_t* handle)
        {
            auto ptr = static_cast<UvTimer*>(handle->data);
            ptr->onClosed();
        });
    }
    else
    {
		onClosed();
    }
}

void UvTimer::onClosed()
{
	if (cbClosed)
		cbClosed(this);
}

void UvTimer::setRepeat(uint64_t times)
{
    repeat = times;
    ::uv_timer_set_repeat(&uvHandle, times);
}






