#pragma once
#include <memory>
#include <string>

using namespace std;
namespace robin
{


	class IWork
	{
	public:
		IWork() {}
		~IWork() {}

		virtual void doWork() {}
		virtual void afterWork() {}

		string getName() { return name;  }

	protected:
		string name = "basework";

	};
	using IWorkPtr = std::shared_ptr<IWork>;
}
