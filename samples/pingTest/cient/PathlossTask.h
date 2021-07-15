#pragma once
#include "../../../include/ITask.h"
#include "../../../utils/Timer.h"
#include <chrono>
#include <ratio>


namespace robin
{
	class PathlossTask : public ITask
	{
	public:
		PathlossTask() : ITask() {}
		virtual ~PathlossTask(){}

		// count mili secs
		double delta() { msecs = duration<double, Timer::ms>(high_resolution_clock::now() - startTime).count(); return msecs; }

		uint64_t  num;
		double msecs;
		time_point<high_resolution_clock>  startTime;
		time_point<high_resolution_clock>  endTime;
	};
}