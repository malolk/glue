#include <network/TcpServer.h>

#include <string>
#include <iostream>

using namespace network;
using namespace network::poller;
using namespace network::csocket;
using namespace network::SocketConnection;

namespace network
{
	const char pathOfIdleFd[] = "/dev/null";
	void defaultReadOp(std::shared_ptr<Connection>& conn, ByteBuffer& buf)
	{
		std::cout << (std::string("receive ") + std::to_string(buf.readableBytes()) + std::string(" bytes : ") + buf.toString()) << std::endl;
		conn->sendData(buf);
		buf.reset();
	}

}

void TcpServer::startServer()
{
	if (!started)
	{
		started = true;
		idleFd = ::open(pathOfIdleFd, O_CLOEXEC | O_NONBLOCK); 
		CHECKEXIT(idleFd >= 0); 

		if (threadNum <= 0) 
			LOGWARN("thread pool size should be greater than zero");
		else
		{
			Epoll epollMaster;
			epollMaster.epollInitialize();
			epollMasterPtr = &epollMaster;

			threadPool.setEpollMaster(&epollMaster);
			threadPool.start();

			srvSocket.bind(ipStr, port);
			//port reuse
			srvSocket.enablePortReuse(1); 
			srvSocket.listen();
			listenChannel.setEpoll(&epollMaster);
			listenChannel.setIoFd(srvSocket.getSockfd());
			listenChannel.setCallbackOnRead(std::bind(&TcpServer::acceptConn, this));
			//	listenChannel.setCallbackOnClose();
			listenChannel.addIntoEpoll();
			listenChannel.enableReading();

			epollMaster.runEpoll();
			threadPool.shutdown();
		}
	}
}


void TcpServer::setReadOp(const CallbackOnReadOp& cb)
{
	readOp = cb;
}

void TcpServer::shutdown()
{
	epollMasterPtr->runNowOrLater(std::bind(&Epoll::epollClose, epollMasterPtr));
}

void TcpServer::acceptConn()
{
	SocketAddress connAddr;

	while(1)
	{
		int ret = srvSocket.accept(connAddr);
		if (ret >= 0)
		{
			std::string connName = std::string("Connection-") + std::to_string(ret);
			CHECK(connCluster.find(connName) == connCluster.end());
			Epoll* epollPtr = threadPool.nextEpoll();
			CHECK(epollPtr);

			std::shared_ptr<Connection> connPtr = std::make_shared<Connection>(ret, epollPtr);
			CHECK(connPtr);
			connCluster[connName] = connPtr;
			connPtr->setReadOperation(readOp);
			connPtr->setCloseOperation(std::bind(&TcpServer::delConnection, this, connName));
			connPtr->initiateChannel();
		}
		else 
		{
			// note: in case the ddos attack, set a limit on one IO operation
			if(ret == NOCONN || ret == RETRY)
				return;
			else if (ret == LACKFD)
			{
				::close(idleFd);
				int tmpFd = srvSocket.accept(connAddr);
				// note: server could close the socket directly
				::close(tmpFd);
				idleFd = ::open(pathOfIdleFd, O_CLOEXEC | O_NONBLOCK); 
				CHECK(idleFd >= 0);
				LOGWARN("no enough fd");
				return;
			}
			else
			{
				LOGWARN("accept error");
				return;
			}
		}
	}
}


void TcpServer::delConnection(const std::string& key)
{
	epollMasterPtr->runLater(std::bind(&TcpServer::delConnectionInEpoll, this, key));
}

void TcpServer::delConnectionInEpoll(const std::string& key)
{
	std::map<std::string, std::shared_ptr<Connection>>::iterator 
		it = connCluster.find(key);
	if (it == connCluster.end())
		return;
	std::shared_ptr<Connection> conn = it->second;
	connCluster.erase(it);

	conn->getEpollPtr()->runNowOrLater(std::bind(
				&Connection::distroy, conn.get(), conn));
}
