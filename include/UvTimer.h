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

		UvTimer(EventLoop* loop, uint64_t timeout, uint64_t repeat, TimerCallback callback);
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

		TimerClosedCallback cbClosed;

	private:
		void onTimer();
		void onClosed();

		// used in by uv, then call onTimer()
		static void uvOnTimer(uv_timer_t* handle);

	};

}
#endif
