#ifndef NETWORK_TCPCLIENT_H
#define NETWORK_TCPCLIENT_H

#include <network/Buffer.h>
#include <network/Socket.h>
#include <network/SocketAddress.h>
#include <network/Epoll.h>
#include <network/Connection.h>
#include <libbase/Noncopyable.h>
#include <libbase/Debug.h>
#include <libbase/MutexLock.h>

#include <string>
#include <memory>
#include <functional>

namespace network
{
class TcpClient: private libbase::Noncopyable
{
public:
	typedef std::function<void(std::shared_ptr<SocketConnection::Connection>&, ByteBuffer&)>  CallbackOnReadOp;	
	typedef std::function<void(std::shared_ptr<SocketConnection::Connection>&)>  CallbackOnInitOp;
	
	TcpClient(const std::string& srvIpIn, uint16_t portIn, bool retryIn = true)
		: srvIp(srvIpIn), 
		  port(portIn), 
		  retry(retryIn), 
		  connected(false),
		  mu(),
		  sock(false) 
	{ }
	
	explicit TcpClient(const csocket::SocketAddress& srvAddr, bool retryIn = true)
		: srvIp(srvAddr.getIpStr()), 
		  port(srvAddr.getPort()), 
		  retry(retryIn), 
		  connected(false),
		  mu(),
		  sock(false) 
	{ }

	~TcpClient() {}

	void start();
	void setReadOp(const CallbackOnReadOp&);
	void setInitOp(const CallbackOnInitOp&);
	void setEpoll(poller::Epoll*);
	void stop();	

	bool isConnected() { return connected; }	
private:
	
	int tryConnect();
	void initConn(const CallbackOnInitOp&);
	void delConnection();
	void delConnectionInEpoll();
	void stopInEpoll();
	
	std::string srvIp;
	uint16_t port;
	bool retry;
	bool connected;	
	libbase::MutexLock mu;
	csocket::Socket sock;
	std::shared_ptr<SocketConnection::Connection> conn;
	CallbackOnInitOp initOp;
	CallbackOnReadOp readOp;
	poller::Epoll* epollPtr;
};	
}
#endif
