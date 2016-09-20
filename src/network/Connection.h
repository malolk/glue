#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

#include <network/Socket.h>
#include <network/Epoll.h>
#include <network/Buffer.h>
#include <network/EventChannel.h>
#include <libbase/Noncopyable.h>
#include <libbase/TimeStamp.h>
#include <libbase/Debug.h>

#include <functional>
#include <memory>
#include <atomic>

namespace network
{

namespace SocketConnection
{

enum
{
	CONNECTED = 2,
	CLOSING,
	CLOSED, 
};

typedef csocket::Socket SO;
typedef network::ByteBuffer BUF;
class Connection: private libbase::Noncopyable,
				  public std::enable_shared_from_this<Connection>
{
public:
	typedef std::function<void(std::shared_ptr<Connection>&, BUF& )> CallbackOnRead;
	//typedef std::function<void(BUF*)> CallbackOnWrite;
	typedef std::function<void(std::shared_ptr<Connection>&)> CallbackOnInit;
	typedef std::function<void()> CallbackOnClose;

	// default constructor is banded
	Connection(int sockfd, poller::Epoll* ep)
	  :	sock(sockfd), 
		epollPtr(ep), 
		channel(ep, sockfd),
		state(CONNECTED),
		keepAlived(false),
		timerSet(false)
	{ }
	
	~Connection() 
	{
		LOGWARN(std::string("Connection on fd: ") + std::to_string(sock.getSockfd()));
	}

	void setReadOperation(const CallbackOnRead& cb);
	//void setWriteOperation(const CallbackOnWrite& cb);
	void setCloseOperation(const CallbackOnClose& cb);
	void setInitOperation(const CallbackOnInit& cb);
	void sendData(BUF& data);
	void sendDataInEpollThread(BUF& data);
	void writeData();
	void readData();
	void close();
	void shutdown();
	void shutdownNow(const std::shared_ptr<Connection>&);
	void distroy(const std::shared_ptr<Connection>&);
	void setKeepAlived() { keepAlived = true; }
	bool isKeepAlived() const { return keepAlived; }
	poller::Epoll* getEpollPtr() const { return epollPtr; } 
	void initiateChannel()
	{
		CHECK(readCb && closeCb);
		channel.setCallbackOnRead(std::bind(&Connection::readData, this));
		channel.setCallbackOnWrite(std::bind(&Connection::writeData, this));
		channel.setCallbackOnClose(std::bind(&Connection::close, this));
		// note: when and how to close the channel?, defer to the timeout design ^_^
		channel.addIntoEpoll();
		channel.enableReading();
		if(initCb) 
		{
			std::shared_ptr<Connection> tmpPtr = shared_from_this();	
			initCb(tmpPtr);
		}
	}
	void setTimer();
	void unsetTimer();

private:
	void stopWrite();

	SO sock;
	poller::Epoll* epollPtr;
	poller::EventChannel channel;
	// state should be Atomic variable
	std::atomic<int> state;
	bool keepAlived;
	bool timerSet;
	timer::Timer* timeOut; 
	BUF sendBuffer;
	BUF recvBuffer;
	CallbackOnRead readCb;
	CallbackOnClose closeCb;
	CallbackOnInit initCb;
};

}
}
#endif // NETWORK_CONNECTION_H
