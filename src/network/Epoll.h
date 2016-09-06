#ifndef NETWORK_EPOLL_H
#define NETWORK_EPOLL_H

#include <network/Socket.h>
#include <network/EventChannel.h>
#include <network/TimerQueue.h>
#include <libbase/Noncopyable.h>
#include <libbase/Debug.h>
#include <libbase/MutexLock.h>
#include <libbase/TimeStamp.h>
#include <libbase/CurrentThread.h>

#include <vector>
#include <memory>
#include <functional>
#include <atomic>

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <errno.h>

namespace network 
{
namespace poller
{

typedef std::function<void()> FuncVoid;
const int EVENT_NUM_INIT = 1024;
class Epoll: private libbase::Noncopyable
{
public:
	Epoll():
		epfd(-1),
		wakeupFd(-1),
		wakeChannel(),
		timerQueue(this),
		threadId(-1),
		running(false), 
		eventsHandling(false),
		events(EVENT_NUM_INIT)
	{ }

	~Epoll() 
	{ 	
		::close(epfd); 
		::close(wakeupFd);
	}

	void epollInitialize();
	void assertInEpollThread() 
	{ 
		CHECK(isInEpollThread()); 
		if(!isInEpollThread())
		{
			std::cout << "Epoll ThreadID: " << threadId << " " <<
			"Current ThreadID: " << libbase::tid() << std::endl;	
		}
	}
	bool isInEpollThread() { return (libbase::tid() == threadId); }
	void stop();
	void epollClose();
	void runNowOrLater(const FuncVoid& req);
	void runLater(const FuncVoid& req);

	void runEpoll();
	bool hasChannel(EventChannel* );
	void addChannel(EventChannel* );
	void delChannel(EventChannel* );
	void updateChannel(EventChannel* );
	
	network::timer::Timer* addTimer(const libbase::TimeStamp& tm, const network::timer::CallbackOnTimeout& cb, time_t interval = 0);
	void delTimer(network::timer::Timer* );

private:
	void stopInEpoll();
	void opEpoll(EventChannel*, int);
	void handleEvents(int);
	void handleEventsImpl(int, uint32_t);
	void handleReqs();
	void wakeup();
	void wakeupChannelClose();
	void wakeupChannelRead();

	int epfd;		
	int wakeupFd;
	EventChannel wakeChannel;	
	network::timer::TimerQueue timerQueue;
	pid_t threadId;
	std::atomic<bool> running;
	std::atomic<bool> eventsHandling;
	std::vector<struct epoll_event> events; 
	std::vector<EventChannel*> addedEventChannels;
	std::vector<FuncVoid> requests; 
	libbase::MutexLock mu;
};
	
}
}
#endif // NETWORK_EPOLL_H
