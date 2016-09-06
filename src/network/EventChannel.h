#ifndef NETWORK_EVENT_H
#define NETWORK_EVENT_H

#include <libbase/Noncopyable.h>
#include <libbase/Debug.h>

#include <memory>
#include <string>
#include <functional>
#include <atomic>

namespace network
{
// just a interface class for epoll
namespace poller
{

typedef std::function<void()> CallbackOnRead;
typedef std::function<void()> CallbackOnWrite;
typedef std::function<void()> CallbackOnClose;

enum state
{
	UNADDED = 1,
	ADDING,
	ADDED
};

class Epoll;

class EventChannel: private libbase::Noncopyable
{
public:
	EventChannel(): 
		ioFd(-1), 
		eventFlag(0),
		epollPtr(nullptr), 
		status(UNADDED), 
		notifyOnWrite(false),
		notifyOnRead(false)
	{ }

	EventChannel(Epoll* ep, int fd): 
		ioFd(fd), 
		eventFlag(0),
		epollPtr(ep), 
		status(UNADDED), 
		notifyOnWrite(false),
		notifyOnRead(false)
	{ }

	~EventChannel() {
		LOGTRACE();
	}

	void setCallbackOnRead(const CallbackOnRead& cb) { cbRead = cb; }
	void setCallbackOnWrite(const CallbackOnWrite& cb) { cbWrite = cb; }
	void setCallbackOnClose(const CallbackOnClose& cb) { cbClose = cb; }

	//is a design problem ?
	void setEpoll(Epoll* ePtr);
	void setIoFd(int fd);
	void handleRead();
	void handleWrite();
	void handleError();
	void handleClose();

	void addIntoEpoll();
	void addedAlready();
	void enableAll();
	void disableAll();
	void enableReading();
	void disableReading();
	void enableWriting();
	void disableWriting();
	bool isNotifyOnWrite() { return notifyOnWrite; }
	bool isNotifyOnRead() { return notifyOnRead; }
	
	// note: need to check channel has been initialized
	int getEventFlag() const { return eventFlag; }
	int iofd() const { return ioFd; }
	Epoll* getEpollPtr() const { return epollPtr; } 
private:
	void enableRDWR_(bool flag, uint32_t setBit);
	int ioFd;
	int eventFlag;
	Epoll* epollPtr;
	std::atomic<int> status;
	bool notifyOnWrite;
	bool notifyOnRead;
	CallbackOnRead cbRead;
	CallbackOnWrite cbWrite;
	CallbackOnClose cbClose;
};
}
}
#endif // NETWORK_EVENT_H
