/*
   Copyright © 2017-2019, orcaer@yeah.net  All rights reserved.
   Author: orcaer@yeah.net
   Last modified: 2019-9-11
   Description: https://github.com/wlgq2/uv-cpp
   Memo:changed something by robin 2021-07-02
*/

#ifndef   UV_EVENT_LOOP_HPP
#define   UV_EVENT_LOOP_HPP

#include "CommonHeader.h"

namespace robin
{
	class Async;

	class EventLoop
	{
	public:
		enum Mode
		{
			Default,
			New
		};
		enum Status
		{
			NotRun,
			Running,
			Stop
		};
		EventLoop();
		~EventLoop();

		static EventLoop* DefaultLoop();

		int run();
		int runNoWait();
		int stop();
		bool isStoped();
		Status getStatus();
		bool isRunInLoopThread();
		bool runInLoop(const DefaultCallback func);
		bool runInLoopEn(const DefaultCallback func);
		uv_loop_t* handle();

		//static const char* GetErrorMessage(int status);

	private:
		EventLoop(Mode mode);   // user should use DefaultLoop() to get default_loop

		std::thread::id loopThreadId;
		uv_loop_t* loop;

		std::shared_ptr<Async> async;
		std::atomic<Status>  status;
	};

	using EventLoopPtr = std::shared_ptr<EventLoop>;
}
#endif

