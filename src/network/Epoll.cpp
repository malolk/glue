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
	LOGTRACE();
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
	LOGTRACE();
	return timerQueue.addTimer(tm, cb, interval);	
}

void Epoll::delTimer(Timer* timerAgent)
{
	LOGTRACE();
	timerQueue.delTimer(timerAgent);
}

void Epoll::stop()
{
	LOGTRACE();
	runLater(std::bind(&Epoll::stopInEpoll, this));	
}

void Epoll::stopInEpoll()
{
	LOGINFO("");
	assertInEpollThread();
	running = false;       
}

void Epoll::epollClose()
{
	LOGINFO("");
	LOGTRACE();
	assertInEpollThread();
	running = false;       
	wakeup();
}

void Epoll::runNowOrLater(const FuncVoid& req)
{   
	LOGTRACE();
	if (isInEpollThread())
		req();
	else    
		runLater(req);
}           

void Epoll::runLater(const FuncVoid& req)
{
	LOGTRACE();
	{
		libbase::MutexLockGuard m(mu);
		requests.push_back(req);    
	}
	if (!isInEpollThread() || !eventsHandling)
		wakeup();
}

void Epoll::addChannel(EventChannel* ev)
{
	LOGTRACE();
	LOGINFO("");
	assertInEpollThread();
	if(hasChannel(ev))
		return;
	opEpoll(ev, EPOLL_CTL_ADD);
	addedEventChannels.push_back(ev);
	ev->addedAlready();
}

void Epoll::updateChannel(EventChannel* ev)
{
	LOGTRACE();
	LOGINFO("");
	assertInEpollThread();
	if(hasChannel(ev))
		opEpoll(ev, EPOLL_CTL_MOD);
}

void Epoll::delChannel(EventChannel* ev)
{
	LOGTRACE();
	LOGINFO("");
	assertInEpollThread();

	if(!hasChannel(ev)) 
		return;
	ev->disableAll();
	opEpoll(ev, EPOLL_CTL_DEL);
	std::vector<EventChannel*>::iterator iter = std::find(
	addedEventChannels.begin(), addedEventChannels.end(), ev);
	if (iter != addedEventChannels.end())
		addedEventChannels.erase(iter);
}


void Epoll::opEpoll(EventChannel* ev, int op)
{
	LOGTRACE();
	struct epoll_event event;
	event.events = ev->getEventFlag();
	event.data.fd = ev->iofd();
	int ret = epoll_ctl(epfd, op, ev->iofd(), &event);
	if(ret) 
		LOGERROR(errno);	
}

void Epoll::runEpoll()
{
	LOGTRACE();
	std::stringstream numStr;
	numStr << threadId;
	LOGINFO(std::string("Epoll start running in thread ") + numStr.str());
	assertInEpollThread();
	CHECK(running == false);
	running = true;
	while(running)
	{
	//	LOGINFO(">>>waiting<<<");
		int ret = epoll_wait(epfd, &(*events.begin()), static_cast<int>(events.size()), -1);
		CHECK(ret > 0);
		if (static_cast<unsigned int>(ret) == events.size())
			events.resize(ret * 2);
		eventsHandling = true;
		handleEvents(ret);
		eventsHandling = false;
		handleReqs();
	}	
}

bool Epoll::hasChannel(EventChannel* ev)
{
	LOGTRACE();
	return (std::find(addedEventChannels.cbegin(),
				addedEventChannels.cend(), ev) != addedEventChannels.cend());	
}

void Epoll::handleEvents(int numOfEvents)
{
	LOGTRACE();
	LOGINFO("");
	assertInEpollThread();
	for (int index = 0; index < numOfEvents; ++index)
	{
		int iofd = events[index].data.fd;
		uint32_t retEvent = events[index].events;
		handleEventsImpl(iofd, retEvent);
	}
}

void Epoll::handleEventsImpl(int fd, uint32_t rEvent)
{
	LOGTRACE();
	std::vector<EventChannel*>::iterator chanIt = std::find_if(
			addedEventChannels.begin(), addedEventChannels.end(), 
			[fd](const EventChannel* chanPtr) -> bool { return (chanPtr->iofd() == fd); });
	CHECK(chanIt != addedEventChannels.end());
	EventChannel* chanPtr = *chanIt;
	CHECK(chanPtr->iofd() == fd);
	CHECK(chanPtr != nullptr);
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
		LOGWARN(std::string("Hub on fd: ") + std::to_string(fd));
		chanPtr->handleClose();
	}
}

void Epoll::handleReqs()
{
	LOGTRACE();
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
	LOGTRACE();
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
	LOGTRACE();
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
			LOGINFO("event read: EAGAIN");
		else
			LOGERROR(err);		
	}
}

void Epoll::wakeupChannelClose()
{
	LOGTRACE();
	wakeChannel.disableAll();
}

