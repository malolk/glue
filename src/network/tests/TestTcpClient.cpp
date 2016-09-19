#include <network/TcpClient.h>

#include <iostream>
#include <string>

using namespace network;
using namespace libbase;

int cnt = 10;
ByteBuffer data;
poller::Epoll* epollPtr = NULL;

void readCallback(std::shared_ptr<SocketConnection::Connection>& conn, 
ByteBuffer& buf)
{
	std::cout << (std::string("Received: ") + 
	std::to_string(buf.readableBytes()) + std::string("-bytes ") +
	buf.toString()) << std::endl;
	conn->sendData(buf);
	buf.reset();			
	if(--cnt < 0)
	{
		epollPtr->stop();
	}
}

void initCallback(std::shared_ptr<SocketConnection::Connection>& conn)
{
	conn->sendData(data);		
} 

int main()
{
	std::string ball = "give you, give me back";
	data.appendBytes(ball);
	
	poller::Epoll epollInst;
	epollInst.epollInitialize();
	epollPtr = &epollInst;
	
	TcpClient client("127.0.0.1", 8080);
	client.setReadOp(readCallback);
	client.setInitOp(initCallback);
	client.setEpoll(&epollInst);
	
	client.start();			
	epollInst.runEpoll();

	return 0;
}


