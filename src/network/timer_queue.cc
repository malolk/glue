#include <network/TimerQueue.h>
#include <network/Epoll.h>
#include <libbase/Debug.h>

#include <string.h>

using namespace libbase;
using namespace network;
using namespace network::timer;

namespace network
{
namespace timer
{

bool compareOnTimer(const TimerKey& lhs, const TimerKey& rhs)
{
	return (lhs.first < rhs.first);
}

void setTimerFd(int fd, int64_t timeFromNow)
{
	struct itimerspec newVal;
	bzero(&newVal, sizeof newVal);
	newVal.it_value.tv_sec = timeFromNow/1000000;
	newVal.it_value.tv_nsec = (timeFromNow%1000000)*1000;
	CHECKX(timerfd_settime(fd, 0, &newVal, NULL) == 0,
	"timerfd_settime error");
}

void startTimerFd(int fd, const TimeStamp& when)
{
	TimeStamp now;
	int64_t diff = when.diffInMicroSecond(now);
	if (diff <= 0) diff = 100;
	setTimerFd(fd, diff);
}

void stopTimerFd(int fd)
{
	setTimerFd(fd, 0);
}

}
}

/* 
* across thread: delTimer won't affect the readTimerQueueChannel
* same thread: delTimer maybe affect the readTimerQueueChannel, 
* e.g. TimerA could delete TimerB while timeout, then TimerB 
* should't be restart when reset timerfd
*/
void TimerQueue::delTimer(Timer* timerId)
{
	epollPtr->runNowOrLater(std::bind(&TimerQueue::delTimerInEpoll, this, timerId));	
}

void TimerQueue::delTimerInEpoll(Timer* timerId)
{
	TimerKey deletedTimerKey = std::make_pair(timerId->getExpiration(), timerId);
	if(!timerList.isEmpty())
	{
		if(timerList.top().first > deletedTimerKey.first)
		{
			if(onTimeout)
			{
				deletedOnTimeout.insert(deletedTimerKey);	
			}
		}
		else
		{
			deletedTimerList.insert(deletedTimerKey);
		}
	}
	else if(onTimeout)
	{
		deletedOnTimeout.insert(deletedTimerKey);
	}
}
 
/*
* across thread 
*/
Timer* TimerQueue::addTimer(const TimeStamp& tm, const CallbackOnTimeout& cb, time_t interval)
{
	Timer* newTimer = new Timer(cb, tm, interval);
	epollPtr->runNowOrLater(std::bind(&TimerQueue::addTimerInEpoll, this, newTimer));
	return newTimer;
}

void TimerQueue::addTimerInEpoll(Timer* newTimer)
{
	TimerKey newTimerKey = std::make_pair(newTimer->getExpiration(), newTimer);
	bool isMin = true;
	if (!timerList.isEmpty()) 
	{
		TimerKey currentMin = timerList.top();
		if (newTimerKey.first > currentMin.first)
			isMin = false;
	}
	timerList.insert(newTimerKey);
	if (isMin)
		startTimerFd(timerFd, newTimerKey.first);
}

void TimerQueue::readTimerChannel()
{
	uint64_t buf;
	ssize_t ret = ::read(timerFd, &buf, sizeof(uint64_t));
	CHECKX(ret == static_cast<ssize_t>(sizeof(uint64_t)), "timerfd read error");

	std::vector<TimerKey> expiredTimer;
	getExpiredTimer(expiredTimer);
	deletedOnTimeout.clear();

	onTimeout = true;
	for (std::vector<TimerKey>::const_iterator it = expiredTimer.cbegin(); 
	it != expiredTimer.cend(); ++it)
	{
		if(deletedTimerList.find((*it)) == deletedTimerList.end())
			(*it).second->timeout();
	}
	onTimeout = false;

	resetTimerFd(expiredTimer);	
}

void TimerQueue::resetTimerFd(std::vector<TimerKey>& expiredTimer)
{
	for(std::vector<TimerKey>::iterator it = expiredTimer.begin();
	it != expiredTimer.end(); ++it)
	{
		std::set<TimerKey>::iterator itTimer = deletedTimerList.find(*it);
		if((*it).second->isRepeated() && (deletedOnTimeout.find(*it) == deletedOnTimeout.end()) && (itTimer == deletedTimerList.end()))	
		{
			(*it).second->restart();
			TimerKey newTimerKey = std::make_pair((*it).second->getExpiration(), (*it).second);
			timerList.insert(newTimerKey);	
		}
		else
		{
			delete (*it).second;
			if(itTimer != deletedTimerList.end())
				deletedTimerList.erase(itTimer);	
		}
	}

	if (timerList.isEmpty())
		stopTimerFd(timerFd);
	else
		startTimerFd(timerFd, timerList.top().first); 
}

void TimerQueue::getExpiredTimer(std::vector<TimerKey>& expiredTimer)
{
	TimeStamp now;
	while(!timerList.isEmpty())	
	{
		TimerKey minTimer = timerList.top();
		if (minTimer.first < now)
		{
			expiredTimer.push_back(minTimer);
			timerList.topAndPop();
		}
		else
		{
			break;
		}
	}
}

void TimerQueue::initialize()
{
	CHECK(!initialized);
	timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
	CHECK(timerFd >= 0);

	timerChannel.setEpoll(epollPtr);
	timerChannel.setIoFd(timerFd);
	timerChannel.setCallbackOnRead(std::bind(&TimerQueue::readTimerChannel, this));
	timerChannel.addIntoEpoll();
	timerChannel.enableReading();
	initialized = true; 
}
