#include "../include/log4wraper.h"
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/OstreamAppender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Priority.hh>

using namespace robin;

log4wraper::~log4wraper()
{

}
log4wraper::log4wraper() : catRef_(log4cpp::Category::getRoot()), catConfRoot(nullptr)
{
	loadConf();
	if (catConfRoot == nullptr)
	{
		log4cpp::PatternLayout *pPtnLyt1 =
			new log4cpp::PatternLayout;
		pPtnLyt1->setConversionPattern("%d: %p %c %x:%m%n");

		log4cpp::PatternLayout *pPtnLyt2 =
			new log4cpp::PatternLayout;
		pPtnLyt2->setConversionPattern("%d: %p %c %x:%m%n");

		log4cpp::OstreamAppender *pOsAppder =
			new log4cpp::OstreamAppender("osAppender", &std::cout);
		pOsAppder->setLayout(pPtnLyt1);

		log4cpp::FileAppender * pFileAppder =
			new log4cpp::FileAppender("fileAppender", "./buildinlog.txt");
		pFileAppder->setLayout(pPtnLyt2);

		catRef_.setPriority(log4cpp::Priority::DEBUG);
		//catRef_.addAppender(pOsAppder);
		catRef_.addAppender(pFileAppder);

		catRef_.info("build-in log appender create success");
	}
}

void log4wraper::setPriority(Priority priority)
{
	switch(priority)
	{
	case (ERROR):
		catRef_.setPriority(log4cpp::Priority::ERROR);
		break;
	case (WARN):
		catRef_.setPriority(log4cpp::Priority::WARN);
		break;
	case (INFO):
		catRef_.setPriority(log4cpp::Priority::INFO);
		break;
	case (DEBUG):
		catRef_.setPriority(log4cpp::Priority::DEBUG);
		break;
	default:
		catRef_.setPriority(log4cpp::Priority::DEBUG);
		break;
	}
}




