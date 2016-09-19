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
			ret = sock.connect(srvIp, port);
			if(ret == 0) break;
		}			
	}
	else 
		ret = sock.connect(srvIp, port);
	return ret;	
}

void TcpClient::initConn(const TcpClient::CallbackOnInitOp& initOpIn)
{
	initOpIn(conn);	
}

void TcpClient::start()
{
	CHECK(initOp && readOp && epollPtr);
	if(tryConnect()) 
	{
		LOGWARN("connect error");
		return;
	}
	connected = true;
	conn = std::make_shared<Connection>(sock.getSockfd(), epollPtr);
	conn->setReadOperation(readOp);
	conn->setInitOperation(std::bind(&TcpClient::initConn, this, initOp));
	conn->setCloseOperation(std::bind(&TcpClient::delConnection, this));
	conn->initiateChannel();		
}

// for connection closed by peer end
void TcpClient::delConnection()
{
	epollPtr->runNowOrLater(std::bind(&TcpClient::delConnectionInEpoll, this));		
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
	epollPtr->runNowOrLater(std::bind(&TcpClient::stopInEpoll, this));	
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
	epollPtr->runNowOrLater(std::bind(&Connection::shutdownNow, connPtrTmp.get()));		
}


