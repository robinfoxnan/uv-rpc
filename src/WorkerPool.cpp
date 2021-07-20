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

		// find the work matched by task type name
		IWorkPtr work = GlobalConfig::getWorkType(task_req->taskptr->taskTypeName);
		if (work)
		{
			work->doWork(task_req->taskptr);
		}
		else
		{
			printf("can't find worker %s...\n", task_req->taskptr->taskTypeName.c_str());
		}

		// 
		//task_req->taskptr->getConnection()->sendMsg(task_req->taskptr);
		

	}
	// in which tcp connection loop
	void WorkerPool::uvAfterWork(uv_work_t *req, int status)
	{
		uv_work_task_t * task_req = (uv_work_task_t *)req;

		// for debug only
		double ms1 = task_req->taskptr->markMid1();
		char buf[260];
		snprintf(buf, 260, "%s task  afterwork=%f ms", task_req->taskptr->taskIdStr.c_str(), ms1);
		LOG_DEBUG(buf);

		task_req->taskptr->getConnection()->sendMsg(task_req->taskptr);
		delete task_req;
	}

	void WorkerPool::addToWorkQueue(std::shared_ptr<ITask>& taskptr)
	{
		uv_loop_t * loop = taskptr->getConnection()->getLoop();
		uv_work_task_t * work_req = new uv_work_task_t(taskptr);
	
		// send work to worker thread, then notify 
		uv_queue_work(loop, work_req, WorkerPool::uvWork, WorkerPool::uvAfterWork);
	}
}