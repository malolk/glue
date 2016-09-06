#ifndef NETWORK_EPOLLTHREAD_H
#define NETWORK_EPOLLTHREAD_H

#include <network/Epoll.h>
#include <libbase/Noncopyable.h>
#include <libbase/MutexLock.h>
#include <libbase/Cond.h>
#include <libbase/Debug.h>

#include <functional>
#include <atomic>

#include <pthread.h>

namespace network
{
namespace poller
{
class EpollThread: private libbase::Noncopyable
{
public:
	EpollThread(): 
		mu(), 
		cond(mu), 
		epollPtr(nullptr),
		joined(false), 
		started(false), 
		running(false)
	{ }

	// EpollPtr no need to be deleted, Epoll sits on stack
	~EpollThread() 
	{
	//	if (running)
	//		epollPtr->epollClose();
		running = false;
		if (!joined && started)
		{
			CHECKX(pthread_detach(pthreadId) == 0, "pthread_detach failed");
		}
	}

	void threadFunc();
	Epoll* startThread();
	
	int join();
	Epoll* getEpollPtr();

private:
	libbase::MutexLock mu;
	libbase::Cond cond;
	Epoll* epollPtr;
	pthread_t pthreadId;
	bool joined;
	bool started;
	std::atomic<bool> running;
};
}
}

#endif
