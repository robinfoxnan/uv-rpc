/*
   Copyright © 2017-2019, orcaer@yeah.net  All rights reserved.
   Author: orcaer@yeah.net
   Last modified: 2019-9-11
   Description: https://github.com/wlgq2/uv-cpp
   Memo:changed something by robin 2021-07-02
*/
#ifndef UV_TIMER_H
#define UV_TIMER_H

#include "CommonHeader.h"

namespace robin
{
	class EventLoop;

	class UvTimer
	{
	public:
		using TimerCallback = std::function<void(UvTimer*)>;
		using TimerClosedCallback = std::function<void(UvTimer*)>;

		UvTimer(EventLoop* loop, uint64_t timeout_, uint64_t repeat_, TimerCallback callback);
		virtual ~UvTimer();

		void start();
		void stop();
		void close(TimerClosedCallback callback);
		void setRepeat(uint64_t times);

	private:
		bool bStarted;
		uv_timer_t  uvHandle;
		uint64_t    timeout;
		uint64_t    repeat;
		TimerCallback cbOnTimer;

		EventLoop* _loop;

		TimerClosedCallback cbClosed;

	private:
		void onTimer();
		void onClosed();

		// used in by uv, then call onTimer()
		static void uvOnTimer(uv_timer_t* handle);

	};

}


/*
EventLoop *loop = new EventLoop();
void testSend()
{
	UvTimer* uvTimer = new UvTimer(loop,
		3000,   // 设置3000毫秒定时器，
		3000,   //  下次也是3000毫秒
	[=](UvTimer* t)  // lamda表达式，定时器回调
	{
		DEBUG_PRINT("test send \n");
		if (bExit)
		{
			DEBUG_PRINT("end timer \n");
			t->stop();
			t->close([](UvTimer* t)
			{
				printf("delete\n");
				delete t;
				loop->stop();   // 停止循环
			});

		}
	});
	uvTimer->start();
	loop->run();  // 会阻塞在这里，直到退出

	DEBUG_PRINT("send thread end \n");
	delete loop;
}
*/
#endif
