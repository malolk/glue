#include <network/TcpClient.h>
#include <network/Buffer.h>
#include <network/SocketAddress.h>
#include <network/Connection.h>
#include <network/Epoll.h>
#include <network/EpollThreadPool.h>
#include <libbase/Noncopyable.h>
#include <libbase/Debug.h>
#include <libbase/TimeStamp.h>

#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <memory>

#include <unistd.h>
#include <stdlib.h>

using namespace network;
using namespace libbase;

ByteBuffer ball;

class Client : private Noncopyable
{
public:
	explicit Client(const csocket::SocketAddress& srvAddr)
		: cli(srvAddr), recvBytes(0) 
	{}

	~Client() {}

	void start() 
	{
		cli.setReadOp(std::bind(&Client::readCallback, this, std::placeholders::_1, std::placeholders::_2));
		cli.setInitOp(std::bind(&Client::initCallback, this, std::placeholders::_1)); 
		cli.start(); 
	}
	void stop() { cli.stop(); }
	void setEpoll(poller::Epoll* e) { cli.setEpoll(e); }
    size_t getSentBytesNum() const  { return recvBytes; }

private:
	void readCallback(std::shared_ptr<SocketConnection::Connection>& conn,
	ByteBuffer& buf);
	void initCallback(std::shared_ptr<SocketConnection::Connection>& conn);
	TcpClient cli;
	size_t recvBytes;
};

void Client::readCallback(std::shared_ptr<SocketConnection::Connection>& conn,
ByteBuffer& buf)
{
	LOGTRACE();
	recvBytes += buf.readableBytes();
	conn->sendData(buf);	
	LOGTRACE();
}

void Client::initCallback(std::shared_ptr<SocketConnection::Connection>& conn)
{
	LOGTRACE();
	ByteBuffer cpOfBall;
	cpOfBall.appendBytes(ball);
	conn->sendData(cpOfBall);		
	LOGTRACE();
}

// just like TcpServer
class ClientCluster : private Noncopyable
{
public:
	ClientCluster(const csocket::SocketAddress& srvAddrIn, 
				  int clientNumIn,  int timeout, int threadSize)
		: srvAddr(srvAddrIn),
		  clientNum(clientNumIn),
		  timeInSecs(timeout),
	      threadNum(threadSize),
		  epollMasterPtr(NULL),
		  threadPool(threadSize)
	{ }

	~ClientCluster() {}

	void start() 
	{
		if(threadNum <= 0)
		{
			LOGWARN("thread pool size should be greater than zero");
			return;	
		}	
		else
		{
			poller::Epoll epollMaster;
			epollMaster.epollInitialize();
			epollMasterPtr = &epollMaster;
			
			threadPool.setEpollMaster(&epollMaster);
			threadPool.start();
			
			TimeStamp now;
			now.addInterval(timeInSecs);
			epollMaster.addTimer(now, std::bind(&ClientCluster::timeoutHandler, this), 0);
			LOGTRACE();
			
			for(int index = 0; index < clientNum; ++index)
			{
				cliCluster.push_back(std::unique_ptr<Client>(new Client(srvAddr)));
				cliCluster[index]->setEpoll(threadPool.nextEpoll());
				cliCluster[index]->start();
			}			
			LOGTRACE();
			
			epollMaster.runEpoll();
			LOGTRACE();
			threadPool.shutdown();		
			LOGTRACE();
			
			calcThroughput();		
		}
	}
	
private:
	void timeoutHandler()
	{
		LOGTRACE();
		for(int index = 0; index < clientNum; ++index)
			cliCluster[index]->stop();				
		LOGTRACE();
		epollMasterPtr->stop();
		LOGTRACE();
	}

	void calcThroughput()
	{
		long double sum = 0;
		for(int index = 0; index < clientNum; ++index)	
			sum += cliCluster[index]->getSentBytesNum();
		std::cout << "Running Time=" << timeInSecs 
		<< " " << "Received Bytes="<< sum 
		<< " " << "Throughput="<< (static_cast<double>(sum)/static_cast<double>(1024 * 1024 * timeInSecs)) << " MiB/s" << std::endl;
	}
	csocket::SocketAddress srvAddr;
	std::vector<std::unique_ptr<Client>> cliCluster;
	const int clientNum;
	const int timeInSecs;
	const int threadNum;
	poller::Epoll* epollMasterPtr;
	poller::EpollThreadPool threadPool;
};


int main(int argc, char* argv[])
{
	int kBytes = 16;
	int client = 20000;
	int time = 30;
	int thread = 1;
	std::string ipStr = "127.0.0.1";
	uint16_t port = 8080;
	int opt;
	while((opt = getopt(argc, argv, "s:p:c:k:t:T:")) != -1)
	{
		switch(opt)
		{
			case 's': ipStr = std::string(optarg);
					  break;	
#pragma GCC diagnostic ignored "-Wold-style-cast"					  
			case 'p': port = (uint16_t)atoi(optarg);
					  break;	
#pragma GCC diagnostic error "-Wold-style-cast"					  
			case 'c': client = atoi(optarg);
					  break;	
			case 'k': kBytes = atoi(optarg);
					  break;	
			case 't': time = atoi(optarg);
					  break;	
			case 'T': thread = atoi(optarg);
					  break;
			default:  std::cerr << "usage:" 
					  "<ip> <port> <clients> <chunk size>" 
					  "<time> <thread>" << std::endl;
					  return 1;	
		}				
	}	
	
	const std::string unit = "0123456789";
	for(int cnt = 0; cnt < kBytes*100; ++cnt)
	{
		ball.appendBytes(unit);	
	}
	
	csocket::SocketAddress srvAddr(ipStr, port);	
	ClientCluster cluster(srvAddr, client, time, thread);
	cluster.start();
	
	return 0;	
}
