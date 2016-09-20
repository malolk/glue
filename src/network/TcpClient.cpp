#include <network/TcpClient.h>

using namespace network;
using namespace network::poller;
using namespace network::SocketConnection;
using namespace libbase;

namespace
{
	const int RETRY_LIMIT = 50;	
}

void TcpClient::setReadOp(const CallbackOnReadOp& op)
{
	readOp = op;	
}

void TcpClient::setInitOp(const CallbackOnInitOp& op)
{
	initOp = op;	
}

void TcpClient::setEpoll(poller::Epoll* e)
{
	epollPtr = e;	
}

int TcpClient::tryConnect()
{
	int ret = 0;
	if(retry)
	{
		for(int i = 0; i < RETRY_LIMIT; ++i)
		{
			ret = csocket::Socket::connect(srvIp, port, fd);
			if(ret == 0) break;
		}			
	}
	else 
		ret = csocket::Socket::connect(srvIp, port, fd);
	return ret;	
}

void TcpClient::start()
{
	epollPtr->runNowOrLater(std::bind(&TcpClient::startInEpoll, this));
}

void TcpClient::startInEpoll()
{
	CHECK(initOp && readOp && epollPtr);
	if(tryConnect()) 
	{
		LOGWARN("connect error");
		return;
	}
	conn = std::make_shared<Connection>(fd, epollPtr);
	conn->setReadOperation(readOp);
	conn->setInitOperation(initOp);
	conn->setCloseOperation(std::bind(&TcpClient::delConnection, this));
	conn->initiateChannel();		
	connected = true;
}

// for connection closed by peer end
void TcpClient::delConnection()
{
	epollPtr->runLater(std::bind(&TcpClient::delConnectionInEpoll, this));		
}

void TcpClient::delConnectionInEpoll()
{
	std::shared_ptr<Connection> connPtrTmp;
	{
		connPtrTmp = conn;
		MutexLockGuard m(mu);
		conn.reset();	
	}
	epollPtr->runNowOrLater(std::bind(&Connection::distroy, 
	connPtrTmp.get(), connPtrTmp));	
}

// for connection closed by client actively
void TcpClient::stop()
{
	epollPtr->runLater(std::bind(&TcpClient::stopInEpoll, this));	
}

void TcpClient::stopInEpoll()
{
	std::shared_ptr<Connection> connPtrTmp;
	{
		MutexLockGuard m(mu);
		if(!conn) return;
		connPtrTmp = conn;
		conn.reset();				
	}
	epollPtr->runNowOrLater(std::bind(&Connection::shutdownNow, connPtrTmp.get(), connPtrTmp));		
}


