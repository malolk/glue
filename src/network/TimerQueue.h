#ifndef NETWORK_TIMERQUEUE_H
#define NETWORK_TIMERQUEUE_H

#include <network/Timer.h>
#include <network/EventChannel.h>
#include <libbase/Heap.h>
#include <libbase/Noncopyable.h>
#include <libbase/TimeStamp.h>

#include <utility>
#include <memory>
#include <set>

#include <sys/timerfd.h>
#include <unistd.h>

namespace network
{

namespace poller
{
class Epoll;	
}
namespace timer
{

typedef std::pair<libbase::TimeStamp, Timer*> TimerKey;
bool compareOnTimer(const TimerKey&, const TimerKey&);

class TimerQueue: private libbase::Noncopyable
{
public:
	explicit TimerQueue(poller::Epoll* ep): 
		timerList(true, 4, compareOnTimer), 
		timerFd(-1),
		initialized(false),
		closed(true),
		onTimeout(false),
		epollPtr(ep)
	{ }

	~TimerQueue() 
	{
		if (!closed && initialized)
			::close(timerFd);
	}

	void initialize();
	Timer* addTimer(const libbase::TimeStamp& tm, const CallbackOnTimeout& cb, time_t interval = 0);
	void delTimer(Timer*);

private:
	void addTimerInEpoll(Timer* );
	void delTimerInEpoll(Timer* );
	void readTimerChannel();
	void resetTimerFd(std::vector<TimerKey>& );
	void getExpiredTimer(std::vector<TimerKey>& expiredTimer);

	libbase::Heap<TimerKey> timerList;
	std::set<TimerKey> deletedTimerList;
	std::set<TimerKey> deletedOnTimeout;
	int timerFd;
	bool initialized;
	bool closed;
	bool onTimeout;
	poller::Epoll* epollPtr;
	poller::EventChannel timerChannel;
};
}
}
#endif
