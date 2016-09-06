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

void EpollThreadPool::start(int workerNum)
{
	CHECK(runningState == UNSTARTED);
	CHECK(workerNum >= 0);
	workerSize = workerNum;
	if (workerNum <= 0)
		return;
	workerPtrList.reserve(workerNum);
	for (int index = 0; index < workerNum; ++index)
	{
		workerPtrList.push_back(std::shared_ptr<EpollThread>(new EpollThread()));
		workerPtrList[index]->startThread();
	}
	runningState = RUNNING;
}

void EpollThreadPool::shutdown()
{
	for (int index = 0; index < workerSize; ++index)
	{
		Epoll* epollPtr = workerPtrList[index]->getEpollPtr();
		epollPtr->runLater(std::bind(&Epoll::epollClose, epollPtr));
	}
	
	for (int index = 0; index < workerSize; ++index)
	{
		workerPtrList[index]->join();
	}	
}

