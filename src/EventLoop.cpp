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

EventLoop::EventLoop() : EventLoop(EventLoop::Mode::New)
{
}

EventLoop::EventLoop(EventLoop::Mode mode):loop(nullptr),async(nullptr),status(NotRun)
{
    if (mode == EventLoop::Mode::New)
    {
        loop = new uv_loop_t();
        uv_loop_init(loop);
    }
    else
    {
        loop = uv_default_loop();
    }
    async = std::make_shared<Async>(this);
}

EventLoop::~EventLoop()
{
	// new loop
    if (loop != uv_default_loop())
    {   
        uv_loop_close(loop);
        delete loop;
    }
	else  // default static loop in libuv 
	{
		uv_loop_close(loop);
	}
}

EventLoop* EventLoop::DefaultLoop()
{
    static EventLoop defaultLoop(EventLoop::Mode::Default);
    return &defaultLoop;
}

uv_loop_t* EventLoop::handle()
{
    return loop;
}

// normally you should call this run() in yours work thread
// and should call stop() to 
int EventLoop::run()
{
    if (status == Status::NotRun || status == Status::Stop)
    {
        async->init();
        loopThreadId = std::this_thread::get_id();
        status = Status::Running;
        int ret = ::uv_run(loop, UV_RUN_DEFAULT);
        status = Status::Stop;
        return ret;
    }

    return -1;
}

int EventLoop::runNoWait()
{
    if (status == Status::NotRun)
    {
        async->init();
        loopThreadId = std::this_thread::get_id();
        status = Status::Running;
        auto rst = ::uv_run(loop, UV_RUN_NOWAIT);
        status = Status::NotRun;
        return rst;
    }
    return -1;
}

// should close async first
// uv_stop must called in loop thread
int EventLoop::stop()
{
    if (status == Status::Running)
    {
		// after sync, the close
		runInLoop([this]()
		{
			async->close([](Async* ptr)
			{
				//printf("asyncall close\n");
				::uv_stop(ptr->Loop()->handle());
			});
		});
        
		/*runInLoop([this]()
		{
			uv_stop(loop);
		});*/
		
        return 0;
    }
    return -1;
}

bool EventLoop::isStoped()
{
    return status == Status::Stop;
}

EventLoop::Status EventLoop::getStatus()
{
    return status;
}


bool EventLoop::isRunInLoopThread()
{
    if (status == Status::Running)
    {
        return std::this_thread::get_id() == loopThreadId;
    }
    //EventLoop未运行.
    return false;
}

// since loop is not running, something will not callback, like uv_write
bool EventLoop::runInLoop(const DefaultCallback func)
{
    if (nullptr == func)
        return false;

    if (isRunInLoopThread() || isStoped())
    {
        func();
        return false;
    }
    async->runInLoop(func);
	return true;
}

//const char* EventLoop::GetErrorMessage(int status)
//{
//    if (WriteInfo::Disconnected == status)
//    {
//        static char info[] = "the connection is disconnected";
//        return info;
//    }
//    return uv_strerror(status);
//}