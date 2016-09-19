#include <network/EpollThreadPool.h>

using namespace network;
using namespace network::poller;

namespace network
{
namespace poller
{
	const int WORKERSIZE = 4;	
}
}

Epoll* EpollThreadPool::nextEpoll()
{
	Epoll* ret;
	if(threadCount == 1) 
		ret = epollMaster;
	else 
	{
		if(dispatchId == 0) 
			ret = epollMaster;
		else
			ret = workerPtrList[dispatchId - 1]->getEpollPtr();
		if(++dispatchId == threadCount)
			dispatchId = 0;
	}	
	return ret;
}

void EpollThreadPool::start()
{
	CHECK(runningState == UNSTARTED && epollMaster);
	if(threadCount == 1) return;
	CHECK(threadCount > 1);
	workerPtrList.reserve(threadCount - 1);
	for (int index = 0; index < threadCount - 1; ++index)
	{
		workerPtrList.push_back(std::shared_ptr<EpollThread>(new EpollThread()));
		workerPtrList[index]->startThread();
	}
	runningState = RUNNING;
}

void EpollThreadPool::shutdown()
{
	for (int index = 0; index < threadCount - 1; ++index)
	{
		Epoll* epollPtr = workerPtrList[index]->getEpollPtr();
		epollPtr->runNowOrLater(std::bind(&Epoll::epollClose, epollPtr));
	}
	
	for (int index = 0; index < threadCount - 1; ++index)
	{
		workerPtrList[index]->join();
	}	
}

