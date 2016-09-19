#include <network/EventChannel.h>
#include <network/Epoll.h>
#include <libbase/Debug.h>

using namespace network;
using namespace network::poller;

void EventChannel::setEpoll(Epoll* ePtr)
{   
	CHECK(ePtr && epollPtr == nullptr);
	epollPtr = ePtr;
}   

void EventChannel::setIoFd(int fd) 
{   
	CHECK(ioFd == -1 && fd >= 0); 
	ioFd = fd; 
}
   
void EventChannel::handleRead() 
{  
	CHECK(status == ADDED); 
	CHECK(cbRead);
	cbRead();
}
   
void EventChannel::handleWrite()
{   
	CHECK(status == ADDED); 
	CHECK(cbWrite);
	cbWrite();
}
   
void EventChannel::handleError()
{   
	LOGWARN(std::string("channel error on fd: ") + std::to_string(ioFd));
	handleClose();
}
   
void EventChannel::handleClose()
{
	CHECK(status == ADDED); 
//	CHECK(cbClose);
	if (cbClose)
		cbClose();
}   

void EventChannel::addIntoEpoll()
{
	if(status == ADDED) return;
	CHECK(status == UNADDED);
	CHECK(epollPtr);
	CHECK(ioFd != -1);
	status = ADDING;
	epollPtr->runNowOrLater(std::bind(&Epoll::addChannel, epollPtr, this));
}

void EventChannel::deleteInEpoll()
{
	epollPtr->assertInEpollThread();
	status = UNADDED;		
}

void EventChannel::addedAlready()
{
	CHECK(status == UNADDED || status == ADDING);
	status = ADDED;
}

void EventChannel::enableReading() 
{ 
	enableRDWR_(true, EPOLLIN | EPOLLRDHUP); 
	notifyOnRead = true; 
}

void EventChannel::disableReading() 
{ 
	enableRDWR_(false, EPOLLIN | EPOLLRDHUP); 
	notifyOnRead = false; 
}

void EventChannel::enableWriting() 
{ 
	enableRDWR_(true, EPOLLOUT); 
	notifyOnWrite = true; 
}

void EventChannel::disableWriting() 
{ 
	enableRDWR_(false, EPOLLOUT); 
	notifyOnWrite = false; 
}

void EventChannel::enableAll() 
{ 
	enableRDWR_(true, EPOLLIN | EPOLLRDHUP | EPOLLOUT); 
	notifyOnWrite = true; 
	notifyOnRead = true; 
}

void EventChannel::disableAll() 
{ 
	enableRDWR_(false, EPOLLIN | EPOLLRDHUP | EPOLLOUT); 
	notifyOnWrite = false; 
	notifyOnRead = false; 
}

void EventChannel::enableRDWR_(bool flag, uint32_t setBit)
{
	CHECK(status == ADDING || status == ADDED);
	eventFlag = (flag ? (eventFlag | setBit) : (eventFlag & (~setBit)));
	epollPtr->runNowOrLater(std::bind(&Epoll::updateChannel, epollPtr, this));
}

