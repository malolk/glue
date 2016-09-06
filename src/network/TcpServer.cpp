#include <network/TcpServer.h>

#include <string>

using namespace network;
using namespace network::poller;
using namespace network::csocket;
using namespace network::SocketConnection;

namespace network
{
	const char pathOfIdleFd[] = "/dev/null";
	void defaultReadOp(std::shared_ptr<Connection>& conn, ByteBuffer& buf)
	{
		LOGTRACE();
		LOGINFO(std::string("read") + std::to_string(buf.readableBytes()) + std::string("bytes"));
		conn->sendData(buf);
		buf.reset();
	}

}

void TcpServer::startServer()
{
	LOGTRACE();
	if (!started)
	{
		started = true;
		idleFd = ::open(pathOfIdleFd, O_CLOEXEC | O_NONBLOCK); 
		CHECKEXIT(idleFd >= 0); 

		if (threadNum <= 0) 
			LOGWARN("thread pool size should be greater than zero");
		else
		{
			threadPool.start(threadNum - 1);
			Epoll epollMaster;
			epollMaster.epollInitialize();
			listOfEpollPtr.push_back(&epollMaster);
			for (int index = 0; index < threadNum - 1; ++index)
			{
				std::shared_ptr<EpollThread> threadPtr = threadPool.getThreadPtr(index);
				CHECK(threadPtr);
				listOfEpollPtr.push_back(threadPtr->getEpollPtr());
			}
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
	LOGTRACE();
	readOp = cb;
}

void TcpServer::shutdown()
{
	LOGTRACE();
	listOfEpollPtr[0]->runLater(std::bind(&Epoll::epollClose, listOfEpollPtr[0]));
}

Epoll* TcpServer::getNextEpoll() 
{
	LOGTRACE();
	CHECK(static_cast<unsigned int>(dispatchId) < listOfEpollPtr.size());
	Epoll* ret = listOfEpollPtr[dispatchId++];
	if (dispatchId == threadNum)
		dispatchId = 0;
	return ret;
}

void TcpServer::readConn(const TcpServer::CallbackOnReadOp& readOpIn, const std::string& connName, ByteBuffer& buf)
{
	LOGTRACE();
	CHECK(connCluster.find(connName) != connCluster.end());
	readOpIn(connCluster[connName], buf);
}

void TcpServer::acceptConn()
{
	LOGTRACE();
	SocketAddress connAddr;

	while(1)
	{
		int ret = srvSocket.accept(connAddr);
		if (ret >= 0)
		{
			std::string connName = std::string("Connection-") + std::to_string(ret);
			CHECK(connCluster.find(connName) == connCluster.end());
			connCluster[connName] = std::shared_ptr<Connection>(new Connection(ret, getNextEpoll()));
			connCluster[connName]->setReadOperation(std::bind(&TcpServer::readConn, this, readOp, connName, std::placeholders::_1));
			connCluster[connName]->setCloseOperation(std::bind(&TcpServer::delConnection, this, connName));
			connCluster[connName]->initiateChannel();
		}
		else 
		{
			// note: in case the ddos attack, set a limit on one IO operation
			if (ret == NOCONN || ret == RETRY)
				return;
			else if (ret == LACKFD)
			{
				::close(idleFd);
				int tmpFd = srvSocket.accept(connAddr);
				// note: server could close the socket directly
				::close(tmpFd);
				idleFd = ::open(pathOfIdleFd, O_CLOEXEC | O_NONBLOCK); 
				continue;
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
	LOGTRACE();
	// fix
	std::map<std::string, std::shared_ptr<Connection>>::iterator 
		it = connCluster.find(key);
	if(it == connCluster.end()) return;

	it->second->getEpollPtr()->runLater(std::bind(
				&TcpServer::delConnectionInEpoll, this, key));
}

void TcpServer::delConnectionInEpoll(const std::string& key)
{
	LOGTRACE();
	std::map<std::string, std::shared_ptr<Connection>>::iterator 
		it = connCluster.find(key);
	if (it == connCluster.end())
		return;
	std::shared_ptr<Connection> conn = it->second;
	connCluster.erase(it);
	conn->getEpollPtr()->runNowOrLater(std::bind(
				&Connection::distroy, conn.get(), conn));
}
