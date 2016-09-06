#ifndef HTTP_SERVER_H
#define HTTP_SERVER_H

#include "../network/TcpServer.h"
#include "../network/Connection.h"
#include "../network/Buffer.h"

#include <string>
#include <memory>

namespace network
{
namespace httpd
{
class HttpServer: private libbase::Noncopyable
{
public:
	typedef std::shared_ptr<network::SocketConnection::Connection> ConnectionPtr;
	explicit HttpServer(const std::string& ipStrIn, 
	uint16_t portIn = 8080,
	int threadSize = poller::WORKERSIZE + 1)
	: server(ipStrIn, portIn, threadSize) 
	{}

	void start();
private:
	void callbackOnRequest(ConnectionPtr& connPtr, ByteBuffer& buf);
	TcpServer server;	
}; 	
}
}
#endif
