#include "HttpServer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

#include <functional>

using namespace network;
using namespace network::httpd;
using namespace network::SocketConnection;

void HttpServer::callbackOnRequest(ConnectionPtr& connPtr, ByteBuffer& buf)
{
	HttpRequest httpReq(connPtr);
	if (httpReq.isAlready(buf))
	{
		LOGTRACE();
		httpReq.doRequest(buf);
		LOGTRACE();
		connPtr->shutdown();
		LOGTRACE();
	}
}

void HttpServer::start()
{
	server.setReadOp(std::bind(&HttpServer::callbackOnRequest, this, std::placeholders::_1, std::placeholders::_2));
	server.startServer();
}


