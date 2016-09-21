#include <network/TcpClient.h>

#include <iostream>
#include <string>

using namespace network;
using namespace libbase;

void readCallback(std::shared_ptr<SocketConnection::Connection>& conn,
ByteBuffer& buf)
{
	std::cout << buf.toString() << std::endl;
	buf.reset();
	std::string data;
	std::cin >> data;
	buf.appendBytes(data);
	conn->sendData(buf);	
	std::cout << buf.readableBytes() << std::endl;	
//	buf.reset();
}

void initCallback(std::shared_ptr<SocketConnection::Connection>& conn)
{
	std::string data;
	std::cin >> data;
	ByteBuffer tmp;
	tmp.appendBytes(data);
	conn->sendData(tmp);	
}

int main()
{
	poller::Epoll epollInst;
	epollInst.epollInitialize();
	
	TcpClient client("127.0.0.1", 8080);
	client.setReadOp(readCallback);
	client.setInitOp(initCallback);
	client.setEpoll(&epollInst);
	
	client.start();
	epollInst.runEpoll();
	
	return 0;	
}


