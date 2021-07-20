#pragma once
#include "../../../include/ITask.h"



namespace robin
{
	class PathlossTask : public ITask
	{
	public:
		PathlossTask() : ITask() {}
		virtual ~PathlossTask(){}

		// count mili secs
		
		uint64_t  num;
		
	};
}