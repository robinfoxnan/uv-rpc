#pragma once
#include "../../../include/IWork.h"

namespace robin
{
	class WorkAddNum :public IWork
	{
	public:
		WorkAddNum()
		{
			name = "IWork";
		}
		~WorkAddNum()
		{

		}
		virtual void doWork(TaskPtr &pTask) override
		{ 
			//printf("worker AddNum\n");
			/*uint64_t sum = 0I64;
			for (int i=0; i<1000000; i++)
			{
				sum += 1I64;
			}*/
			// for debug only
			double msecs = pTask->markMid1();
			char buf[260];
			snprintf(buf, 260, "%s task£ºdowork =%f ms", pTask->taskIdStr.c_str(), msecs);
			LOG_DEBUG(buf);

			//printf("finished£¬AddNum\n");
		}

		virtual void afterWork(TaskPtr &pTask) override {}
	};
}