#pragma once

namespace robin
{
	// Interface for read callback
	class IDispatcher
	{
	public:
		virtual void onMessage(void * client, char *buf, unsigned long len) {};
	};
}