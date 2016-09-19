#ifndef NETWORK_EPOLLTHREADPOOL_H
#define NETWORK_EPOLLTHREADPOOL_H

#include <network/Epoll.h>
#include <network/EpollThread.h>
#include <network/EventChannel.h>
#include <libbase/Debug.h>

#include <vector>
#include <memory>

namespace network
{
namespace poller
{

extern const int WORKERSIZE;
enum RunningState
{
	UNSTARTED = 0,
	RUNNING,
	LEAVING,
	STOP
};

class EpollThreadPool: private libbase::Noncopyable
{
public:
	explicit EpollThreadPool(int threadSize = WORKERSIZE)
		: dispatchId(0),
		  threadCount(threadSize), 
		  runningState(UNSTARTED)
	{ }

	~EpollThreadPool() {}

	void start();
	void shutdown();
	void setEpollMaster(Epoll* epollIn)
	{
		epollMaster = epollIn;	
	}
	Epoll* nextEpoll();

private:
	int dispatchId;
	int threadCount;  
	bool runningState;
	Epoll* epollMaster;
	std::vector<std::shared_ptr<EpollThread>> workerPtrList;
};

}
}
#endif // NETWORK_EPOLLTHREADPOOL_H
