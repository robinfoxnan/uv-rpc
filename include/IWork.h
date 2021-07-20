#pragma once
#include <memory>
#include <string>
#include "ITask.h"

using namespace std;
namespace robin
{


	class IWork
	{
	public:
		IWork() {}
		~IWork() {}

		virtual void doWork(TaskPtr& task) {}
		virtual void afterWork(TaskPtr & task) {}

		string getName() { return name;  }

	protected:
		string name = "ITask";

	};
	using IWorkPtr = std::shared_ptr<IWork>;
}
