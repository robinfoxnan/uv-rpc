#pragma once
#include "ITask.h"
#include "CharVector.h"

namespace robin
{
	class IEncoder
	{
	public:
		virtual IEncoder * instance() { return nullptr; }
		virtual void encodeTask(CharVector& buffer, TaskPtr task) {}
	};

	using IEncoderPtr = shared_ptr<IEncoder>;
}
