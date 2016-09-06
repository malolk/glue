#include <network/Connection.h>

using namespace network;
using namespace network::SocketConnection;
using namespace network::csocket;
using namespace network::poller;
using namespace libbase;

namespace network
{
namespace SocketConnection
{
	// default idle connection timeout 75s
	const int DEFAULT_TIME_OUT = 75;
}	
}
// should be in epoll loop thread
void Connection::setTimer()
{
	epollPtr->assertInEpollThread();
	if(timerSet)
		epollPtr->delTimer(timeOut);
	else
		timerSet = true;	
	TimeStamp now;
	now.addInterval(DEFAULT_TIME_OUT);
	timeOut = epollPtr->addTimer(now, std::bind(&Connection::shutdown, this));
}

void Connection::unsetTimer()
{
	epollPtr->assertInEpollThread();
	if(timerSet)
	{
		epollPtr->delTimer(timeOut);
		timerSet = false;
	}	
}

void Connection::setReadOperation(const CallbackOnRead& cb)
{
	LOGTRACE();
	readCb = cb;
}

void Connection::setCloseOperation(const CallbackOnClose& cb)
{
	LOGTRACE();
	closeCb = cb;
}

void Connection::sendData(BUF& data)
{
	LOGTRACE();
	// io operation in single thread, safe
	epollPtr->runNowOrLater(std::bind(&Connection::sendDataInEpollThread,
	this, data));
}

void Connection::sendDataInEpollThread(BUF& data)
{
	LOGTRACE();
	ssize_t sentBytes = 0;
	size_t sendableBytes = data.readableBytes();
	if (sendBuffer.readableBytes() == 0)
	{
		sentBytes = sock.sendBytes(data);
		if (sentBytes == WRERR)
		{
			state = CLOSED;
			epollPtr->runLater(std::bind(&EventChannel::handleClose, &channel));
			return;
		}
		if (static_cast<size_t>(sentBytes) == sendableBytes)
		{
			channel.disableWriting();
			// maybe in closing state
			if (state == CLOSING)
				shutdown();
		}
	}
	
	if(sendableBytes > static_cast<size_t>(sentBytes))
	{
		sendBuffer.appendBytes(data.addrOfRead() + sentBytes, 
		sendableBytes - sentBytes);
		data.movePosOfRead(sendableBytes);
		if (!channel.isNotifyOnWrite())
			channel.enableWriting();
	}
}

void Connection::writeData()
{
	LOGTRACE();
	ssize_t sentBytes = sock.sendBytes(sendBuffer);
	if (sentBytes == WRERR)
	{
		state = CLOSED;
		epollPtr->runLater(std::bind(&EventChannel::handleClose, &channel));
		return;
	}
	sendBuffer.movePosOfRead(sentBytes);
	if (sendBuffer.readableBytes() == 0)
	{	
		channel.disableWriting();
		if (state == CLOSING)
			shutdown();
	}
}

void Connection::readData()
{
	LOGTRACE();
	ssize_t recvBytes = sock.recvBytes(recvBuffer);
	if (recvBytes > 0)
		readCb(recvBuffer);   
	else if (recvBytes == NODATA)
		return;
	else if (recvBytes == 0)
	{
		state = CLOSED;
		epollPtr->runNowOrLater(std::bind(&EventChannel::handleClose, &channel));
	}	
	else
	{
		state = CLOSED; 
		epollPtr->runNowOrLater(std::bind(&EventChannel::handleClose, &channel));
		LOGWARN("readData error");
	}
}

void Connection::close()
{
	LOGTRACE();
	CHECK(closeCb);
	channel.disableAll();
	state = CLOSED;
	closeCb();
}

void Connection::distroy(const std::shared_ptr<Connection> conn)
{
	LOGTRACE();
	CHECK(state == CLOSED);
	channel.disableAll();
	epollPtr->delChannel(&channel);
}

// could be used across threads
void Connection::shutdown()
{
	LOGTRACE();
	state = CLOSING;
	epollPtr->runLater(std::bind(&Connection::stopWrite, this));
}

void Connection::stopWrite()
{
	LOGTRACE();
	CHECK(state == CLOSING);
	if (!channel.isNotifyOnWrite())
		sock.stopWrite();
}
