#include "../include/WorkerPool.h"
#include "../include/GlobalConfig.h"
#include "../include/ITask.h"
#include "../include/TcpConnection.h"
#include "../include/CommonHeader.h"

namespace robin
{
	// in worker thread loop
	void WorkerPool::uvWork(uv_work_t *req)
	{
		uv_work_task_t * task_req = (uv_work_task_t *)req;
		std::string str = task_req->taskptr->getConnection()->getKey();
		//printf("WorkerPool::uvWork() is doing job from %s\n", str.c_str());

		// find the work matched by tasktypename
		IWorkPtr work = GlobalConfig::getWorkType(task_req->taskptr->taskTypeName);
		if (work)
		{
			work->doWork(task_req->taskptr);
		}
		else
		{
			printf("can't find worker %s...\n", task_req->taskptr->taskTypeName.c_str());
		}

		// , 
		task_req->taskptr->getConnection()->sendMsg(task_req->taskptr);

	}
	// in which thread?
	void WorkerPool::uvAfterWork(uv_work_t *req, int status)
	{
		uv_work_task_t * task_req = (uv_work_task_t *)req;
		
		delete req;
	}

	void WorkerPool::addToWorkQueue(std::shared_ptr<ITask>& taskptr)
	{
		uv_loop_t * loop = taskptr->getConnection()->getLoop();
		uv_work_task_t * work_req = new uv_work_task_t(taskptr);
		//work_req->taskptr = taskptr;

		// send work to worker thread, then notify 
		uv_queue_work(loop, work_req, WorkerPool::uvWork, WorkerPool::uvAfterWork);
	}
}