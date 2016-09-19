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
			  int threadSize = poller::WORKERSIZE,
			  CallbackOnReadOp readOpIn = defaultReadOp) : 
		idleFd(-1),
		started(false),
		port(portIn),
		ipStr(ipStrIn),
		srvSocket(true), 
		listenChannel(),
		readOp(readOpIn),
		threadNum(threadSize),
		epollMasterPtr(NULL),
		threadPool(threadSize)
	{ }
	
	~TcpServer() {}

	void startServer();
	void setReadOp(const CallbackOnReadOp&);
	void shutdown();

private:
	void acceptConn();
	void delConnection(const std::string&);
	void delConnectionInEpoll(const std::string&);

	int idleFd;
	bool started;
	const uint16_t port;
	const std::string ipStr;
	csocket::Socket srvSocket;
	poller::EventChannel listenChannel;
	//Epoll* epollBasePtr;
	std::map<std::string, std::shared_ptr<SocketConnection::Connection>> connCluster;
	CallbackOnReadOp readOp;
	const int threadNum;
	poller::Epoll* epollMasterPtr;
	poller::EpollThreadPool threadPool;
};
}
#endif
