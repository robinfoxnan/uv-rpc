#pragma once
#include "ITask.h"
#include <vector>

namespace robin
{
	class IEncoder
	{
	public:
		virtual IEncoder * instance() { return nullptr; }
		virtual void encodeTask(std::vector<char>& buffer, TaskPtr task) {}
	};
}
