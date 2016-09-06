#include <network/EpollThread.h>

using namespace network;
using namespace network::poller;

namespace 
{
// C wrapper
void* threadRoutine(void *thread)
{
	EpollThread* threadPtr = static_cast<EpollThread*>(thread);
	threadPtr->threadFunc();
	return static_cast<void*>(0);
}

}

void EpollThread::threadFunc()
{
	Epoll epollInst;
	epollInst.epollInitialize();
	{
		libbase::MutexLockGuard m(mu);
		epollPtr = &epollInst;
		cond.notifyOne();
	}
	running = true;
	epollInst.runEpoll();
	running = false;
	epollInst.epollClose();
}

Epoll* EpollThread::startThread()
{
	int ret = pthread_create(&pthreadId, NULL, threadRoutine, this);
	CHECKXEXIT(ret == 0, "pthread_create failed");
	(void)ret;
	{
		libbase::MutexLockGuard m(mu);
		while(epollPtr == nullptr)
			cond.wait();
	}
	started = true;
	return epollPtr;
}

int EpollThread::join()
{
	CHECK(!joined);
	joined = true;
	int ret = pthread_join(pthreadId, NULL);
	CHECKX(ret == 0, "pthread_join failed");
	return ret;
}

Epoll* EpollThread::getEpollPtr()
{
	CHECK(epollPtr != nullptr);
	return epollPtr;
}

