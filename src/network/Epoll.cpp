#include <network/Epoll.h>
#include <network/EventChannel.h>

#include <functional>
#include <algorithm>
#include <string>
#include <sstream>

#include <pthread.h>

using namespace network;
using namespace network::poller;
using namespace network::timer;
using namespace libbase;

void Epoll::epollInitialize()
{
	CHECKEXIT((epfd = ::epoll_create1(EPOLL_CLOEXEC)) >= 0); 
	CHECKEXIT((wakeupFd = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK)) >= 0); 
	threadId = tid();
	wakeChannel.setEpoll(this);
	wakeChannel.setIoFd(wakeupFd);
	wakeChannel.setCallbackOnRead(std::bind(&Epoll::wakeupChannelRead, this));
	wakeChannel.setCallbackOnClose(std::bind(&Epoll::wakeupChannelClose, this));
	addChannel(&wakeChannel);
	wakeChannel.enableReading();
	timerQueue.initialize();
}

Timer* Epoll::addTimer(const TimeStamp& tm, const CallbackOnTimeout& cb, time_t interval)
{
	return timerQueue.addTimer(tm, cb, interval);	
}

void Epoll::delTimer(Timer* timerAgent)
{
	timerQueue.delTimer(timerAgent);
}

void Epoll::stop()
{
	runLater(std::bind(&Epoll::stopInEpoll, this));	
}

void Epoll::stopInEpoll()
{
	assertInEpollThread();
	running = false;      
	wakeup();
}

void Epoll::epollClose()
{
	assertInEpollThread();
	running = false;       
	wakeup();
}

void Epoll::runNowOrLater(const FuncVoid& req)
{   
	if (isInEpollThread())
		req();
	else    
		runLater(req);
}           

void Epoll::runLater(const FuncVoid& req)
{
	{
		libbase::MutexLockGuard m(mu);
		requests.push_back(req);    
	}
	if (!isInEpollThread() || !eventsHandling)
		wakeup();
}

void Epoll::addChannel(EventChannel* ev)
{
	assertInEpollThread();
	if(hasChannel(ev))
		return;
	opEpoll(ev, EPOLL_CTL_ADD);
	addedEventChannels.insert(ev);
	ev->addedAlready();
}

void Epoll::updateChannel(EventChannel* ev)
{
	assertInEpollThread();
	if(hasChannel(ev))
		opEpoll(ev, EPOLL_CTL_MOD);
}

void Epoll::delChannel(EventChannel* ev)
{
	assertInEpollThread();

	if(!hasChannel(ev)) 
		return;
	ev->disableAll();
	opEpoll(ev, EPOLL_CTL_DEL);
	if(hasChannel(ev))
	{
		ev->deleteInEpoll();
		addedEventChannels.erase(ev);
	}
}


void Epoll::opEpoll(EventChannel* ev, int op)
{
	struct epoll_event event;
	event.events = ev->getEventFlag();
	event.data.ptr = ev;
	int ret = epoll_ctl(epfd, op, ev->iofd(), &event);
	if(ret) 
		LOGERROR(errno);	
}

void Epoll::runEpoll()
{
	assertInEpollThread();
	CHECK(running == false);
	running = true;
	while(running)
	{
		int ret = epoll_wait(epfd, &(*events.begin()), static_cast<int>(events.size()), -1);
		CHECK(ret > 0);
		if (static_cast<size_t>(ret) == events.size())
			events.resize(2 * events.size());
		eventsHandling = true;
		handleEvents(ret);
		eventsHandling = false;
		handleReqs();
	}	
}

bool Epoll::hasChannel(EventChannel* ev)
{
	return addedEventChannels.count(ev);	
//	return ev->isAdded();
}

void Epoll::handleEvents(int numOfEvents)
{
	assertInEpollThread();
	for (int index = 0; index < numOfEvents; ++index)
	{
		EventChannel* chanPtr = static_cast<EventChannel*>(events[index].data.ptr);
		uint32_t retEvent = events[index].events;
		handleEventsImpl(chanPtr, retEvent);
	}
}

void Epoll::handleEventsImpl(EventChannel* chanPtr, uint32_t rEvent)
{
#ifndef NDEBUG
	CHECK(hasChannel(chanPtr));
#endif

	if (rEvent & EPOLLIN || rEvent & EPOLLRDHUP || rEvent & EPOLLPRI)
	{
		chanPtr->handleRead();
	}else if (rEvent & EPOLLOUT)
	{
		chanPtr->handleWrite();
	}else if (rEvent & EPOLLERR)
	{
		LOGWARN("epoll internal error");
		chanPtr->handleError();
	}
	else if (!(rEvent & EPOLLIN) && (rEvent & EPOLLHUP))
	{
		LOGWARN(std::string("Hub on fd: ") + std::to_string(chanPtr->iofd()));
		chanPtr->handleClose();
	}
}

void Epoll::handleReqs()
{
	std::vector<FuncVoid> rcvReqs;
	{
		MutexLockGuard m(mu);
		std::swap(rcvReqs, requests);
	}

	for(std::vector<FuncVoid>::iterator it = rcvReqs.begin();
			it != rcvReqs.end(); ++it)
	{
		(*it)();
	}
}

void Epoll::wakeup()
{
	uint64_t buf = 1;
	ssize_t ret = ::write(wakeupFd, &buf, sizeof(uint64_t));
	if (ret < 0)
	{
		int err = errno;
		if (err == EINTR || err == EAGAIN)
		{
			ret = ::write(wakeupFd, &buf, sizeof(uint64_t));
			if (ret < 0)
				LOGWARN("eventfd write error: EINTR or EAGAIN");
		}
		else
			LOGERROR(err);		
	}
}

// TODO: make read IO generic
void Epoll::wakeupChannelRead()
{
	uint64_t buf;
	ssize_t ret = ::read(wakeupFd, &buf, sizeof(uint64_t));
	if (ret < 0)
	{
		int err = errno;
		if (err == EINTR)
		{
			ret = ::read(wakeupFd, &buf, sizeof(uint64_t));
			if (ret < 0)
				LOGWARN("event read error: EINTR");
		}
		if (err == EAGAIN)
			LOGWARN("event read: EAGAIN");
		else
			LOGERROR(err);		
	}
}

void Epoll::wakeupChannelClose()
{
	wakeChannel.disableAll();
}

