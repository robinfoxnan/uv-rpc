/*
 * robin 2021-07-28
**/

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
		
		//std::string str = task_req->taskptr->getConnection()->getKey();
		//DEBUG_PRINT("WorkerPool::uvWork() is doing job from %s\n", str.c_str());

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

		// here or in func 'after work'
		/*TcpConnectionPtr ptr = task_req->taskptr->getConnection();
		if (ptr)
		{
			task_req->taskptr->getConnection()->sendMsg(task_req->taskptr);
		}*/
		

	}
	// will be called in  tcp connection loop, so send back response here is ok;
	void WorkerPool::uvAfterWork(uv_work_t *req, int status)
	{
		uv_work_task_t * task_req = (uv_work_task_t *)req;

		// for debug only
		//double ms1 = task_req->taskptr->markMid1();
		//FORMAT_DEBUG("%s task  afterwork=%f ms", task_req->taskptr->taskIdStr.c_str(), ms1);

		// note: when tcp is closed, but task is still in pool
		TcpConnectionPtr ptr = task_req->taskptr->getConnection();
		if (ptr)
		{
			task_req->taskptr->getConnection()->sendMsg(task_req->taskptr);
		}
		else
		{
			// conn is closed
		}
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