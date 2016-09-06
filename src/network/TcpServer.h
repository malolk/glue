#ifndef NETWORK_TCPSERVER_H
#define NETWORK_TCPSERVER_H

#include <network/Buffer.h>
#include <network/Socket.h>
#include <network/Epoll.h>
#include <network/Connection.h>
#include <network/EpollThreadPool.h>
#include <libbase/Noncopyable.h>
#include <libbase/Debug.h>

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <vector>

#include <unistd.h>

namespace network
{


void defaultReadOp(std::shared_ptr<SocketConnection::Connection>& conn, ByteBuffer& buf);
class TcpServer: private libbase::Noncopyable
{
public:
	typedef std::function<void(std::shared_ptr<SocketConnection::Connection>&, ByteBuffer&)> CallbackOnReadOp;
	explicit TcpServer(const std::string& ipStrIn, 
			  uint16_t portIn = 8080,
			  int threadSize = poller::WORKERSIZE + 1,
			  CallbackOnReadOp readOpIn = defaultReadOp) : 
		idleFd(-1),
		dispatchId(0),
		started(false),
		port(portIn),
		ipStr(ipStrIn),
		srvSocket(true), 
		listenChannel(),
		readOp(readOpIn),
		threadNum(threadSize),
		threadPool()
	{ }
	
	~TcpServer() {}

	void startServer();
	void setReadOp(const CallbackOnReadOp&);
	void shutdown();

private:
	poller::Epoll* getNextEpoll();
	void acceptConn();
	void readConn(const CallbackOnReadOp&, const std::string&, ByteBuffer&);
	void delConnection(const std::string&);
	void delConnectionInEpoll(const std::string&);

	int idleFd;
	int dispatchId;
	bool started;
	const uint16_t port;
	const std::string ipStr;
	csocket::Socket srvSocket;
	poller::EventChannel listenChannel;
	//Epoll* epollBasePtr;
	std::map<std::string, std::shared_ptr<SocketConnection::Connection>> connCluster;
	CallbackOnReadOp readOp;
	const int threadNum;
	poller::EpollThreadPool threadPool;
	std::vector<poller::Epoll*> listOfEpollPtr;
};
}
#endif
