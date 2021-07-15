#pragma once
#include "CommonHeader.h"

namespace robin
{
	// Interface for read callback
	class IDispatcher
	{
	public:
		virtual void onMessage(void * client, char *buf, ssize_t len) {};
	};
}