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
		virtual void doWork(TaskPtr task) override 
		{ 
			//printf("worker AddNum\n");
			/*uint64_t sum = 0I64;
			for (int i=0; i<1000000; i++)
			{
				sum += 1I64;
			}*/


			//printf("finished£¬AddNum\n");
		}

		virtual void afterWork(TaskPtr task) override {}
	};
}