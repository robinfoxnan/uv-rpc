#pragma once
#include <uv.h>
#include "ITask.h"

namespace robin
{

	typedef struct uv_work_task :uv_work_t
	{
		uv_work_task(TaskPtr ptr) :taskptr(ptr) {}
		TaskPtr taskptr;
	}uv_work_task_t;

class WorkerPool
{
public :
	static void uvWork(uv_work_t *req);
	static void uvAfterWork(uv_work_t *req, int status);

	static void addToWorkQueue(std::shared_ptr<ITask>& taskptr);
};
}

