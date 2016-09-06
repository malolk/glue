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
	EpollThreadPool(): 
		runningState(UNSTARTED)
	{ }

	~EpollThreadPool() {}

	void start(int workerNum = WORKERSIZE);
	void shutdown();

	std::shared_ptr<EpollThread> getThreadPtr(int index)
	{
		CHECK(index >= 0 && index < workerSize);
		CHECK(runningState == RUNNING);
		return workerPtrList[index]; 
	}

private:
	int workerSize;  // 1 epollMaster and (threadCount - 1) epollWorkers
	bool runningState;
	std::vector<std::shared_ptr<EpollThread>> workerPtrList;
};

}
}
#endif // NETWORK_EPOLLTHREADPOOL_H
