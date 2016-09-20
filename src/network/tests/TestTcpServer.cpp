#include <network/TcpServer.h>

#include <iostream>

using namespace network;

int main()
{
	TcpServer srv("127.0.0.1", 8080, 1);
	std::cout << "=====echo server=====" << std::endl;
	srv.startServer();

	return 0;
}
