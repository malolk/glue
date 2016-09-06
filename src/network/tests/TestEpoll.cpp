#include <network/SocketAddress.h>
#include <network/Socket.h>
#include <network/EventChannel.h>
#include <network/Epoll.h>
#include <network/Buffer.h>

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <functional>

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

using namespace network;
using namespace network::csocket;
using namespace network::poller;

int idleFd = ::open("/dev/null", O_CLOEXEC | O_NONBLOCK);
std::vector<EventChannel*> listOfConn;
std::vector<Socket*> listOfSocket;

void removeConnection(EventChannel* chan, Socket* connSocket)
{
	LOGINFO("I'm reading the connection");
	Epoll* epollPtr = chan->getEpollPtr();
	epollPtr->delChannel(chan);
	std::vector<EventChannel*>::iterator it = std::find(listOfConn.begin(),
	listOfConn.end(), chan);
	if (it != listOfConn.end())
	{
		delete *it;
		listOfConn.erase(it);	
		connSocket->close();
	}
}

void closeConnection(EventChannel* chan, Socket* connSocket)
{
	LOGINFO("I'm reading the connection");
	connSocket->close();	
}

void writeConnection(EventChannel* chan, Socket* connSocket)
{
	LOGINFO("I'm reading the connection");
	ByteBuffer sendBuffer;
	sendBuffer.appendBytes(std::string("SEND: From ") + std::string("Server"));
	ssize_t sentNum = connSocket->sendBytes(sendBuffer);
	if (sentNum == WRERR)
	{
		chan->getEpollPtr()->runLater(std::bind(
		removeConnection, chan, connSocket));
	}
}

void readConnection(EventChannel* chan, Socket* connSocket)
{
	LOGINFO("I'm reading the connection");
	ByteBuffer recvBuf;
	ssize_t recvByteNum = connSocket->recvBytes(recvBuf);
	if (recvByteNum > 0)
	{
		LOGINFO(recvBuf.toString());
	}
	else
	{
		SocketAddress peerAddr;
		connSocket->getPeerName(peerAddr);
		if (recvByteNum == RDERR)
			LOGWARN(std::string("read error on ") +
			peerAddr.toString());
		else
			LOGINFO(std::string("closed by peer ") + 
			peerAddr.toString());

		chan->getEpollPtr()->runLater(std::bind(
		removeConnection, chan, connSocket));
	}
}

void readListenSocket(Epoll* epollPtr, Socket* srvSocket)
{
	SocketAddress connAddr;
	while(1)
	{
		int ret = srvSocket->accept(connAddr);
		if (ret > 0)
		{
			LOGINFO(std::string("New comming on fd: ") + std::to_string(ret));
			EventChannel* newConn = new EventChannel(epollPtr, ret);
			listOfConn.push_back(newConn);
			Socket* connSocket = new Socket(ret);
			listOfSocket.push_back(connSocket);
			newConn->setCallbackOnRead(std::bind(&readConnection, newConn, connSocket));
			newConn->setCallbackOnWrite(std::bind(&writeConnection, newConn, connSocket));
			newConn->setCallbackOnClose(std::bind(&closeConnection, newConn, connSocket));
			newConn->addIntoEpoll();
			newConn->enableReading();
		}
		else
		{
			if (ret == NOCONN)
				return;
			else if (ret == RETRY)
				return;
			else if (ret == LACKFD)
			{
				::close(idleFd);
				int tmpFd = srvSocket->accept(connAddr);
				::close(tmpFd);
				idleFd = ::open("/dev/null", O_CLOEXEC | O_NONBLOCK);
				continue; // or return now?
			}
		}
	}
}

void closeListenSocket(Epoll* epollPtr, Socket* srvSocket)
{
	epollPtr->runLater(std::bind(&Socket::close, srvSocket));
}

void runSingleThreadEpollServer()
{
	Epoll epollInst;
	epollInst.epollInitialize();
	SocketAddress srvAddr;
	Socket listenSocket(true);
	listenSocket.bind(srvAddr.getIpStr(), srvAddr.getPort());	
	listenSocket.listen();
		
	EventChannel* newConn = new EventChannel(&epollInst, listenSocket.getSockfd());
	listOfConn.push_back(newConn);
	newConn->setCallbackOnRead(std::bind(&readListenSocket, &epollInst, &listenSocket));
	newConn->setCallbackOnClose(std::bind(&closeListenSocket, &epollInst, &listenSocket));
	newConn->addIntoEpoll();
	newConn->enableReading();
	LOGINFO(std::string("listen fd: ") + std::to_string(listenSocket.getSockfd()));
	epollInst.runEpoll();
	epollInst.epollClose();
	for (std::vector<Socket*>::iterator it = listOfSocket.begin(); 
	it != listOfSocket.end(); ++it)
		delete *it;
}

int main()
{
	runSingleThreadEpollServer();
	
	return 0;	
}


