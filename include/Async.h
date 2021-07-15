/*
   Copyright © 2017-2019, orcaer@yeah.net  All rights reserved.
   Author: orcaer@yeah.net
   Last modified: 2019-9-11
   Description: https://github.com/wlgq2/uv-cpp
   Memo:changed something by robin 2021-07-02
*/

#ifndef UV_ASYNC_HPP
#define UV_ASYNC_HPP

#include "CommonHeader.h"


namespace robin
{
	class EventLoop;

	class Async : public std::enable_shared_from_this<Async>
	{
	public:
		using OnClosedCallback = std::function<void(Async*)>;

		Async(EventLoop* loop);
		void  init();
		virtual ~Async();

		// called by EventLoop, other class should not
		void runInLoop(DefaultCallback callback);

		void close(OnClosedCallback callback);
		static void onAfterClose(uv_handle_t* handle);

		EventLoop* Loop();
	private:
		// just a reference back to loop, not own it; loop owns the async
		EventLoop  * refLoop;

		// used for uv
		uv_async_t  uvHandle;
		static void uvCallback(uv_async_t * handle);
		void realProcess();


		// All async notify pushed to this queue, and wait for loop call for async_callback, 
		// then pop and call one by one;
		std::deque<DefaultCallback> cbQue;
		std::mutex queMutex;

		// callback is called in onClosed() 
		void onClosed();
		OnClosedCallback cbOnClosed;
	};

	using AsyncPtr = std::shared_ptr<Async>;

}
#endif
