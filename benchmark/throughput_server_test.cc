#include <network/TcpServer.h>
#include <network/Connection.h>
#include <network/Buffer.h>

#include <iostream>
#include <memory>
#include <string>

#include <unistd.h>
#include <stdlib.h>

using namespace network;

void pingpong(std::shared_ptr<SocketConnection::Connection>& conn, ByteBuffer& buf)
{
	conn->sendData(buf);
	buf.reset();	
}

int main(int argc, char* argv[])
{
	std::string ipStr = "127.0.0.1";
	uint16_t port = 8080;
	int thread = 2;

	int opt;
	while((opt = getopt(argc, argv, "s:p:t:")) != -1)
	{
		switch(opt)
		{
			case 's': ipStr = std::string(optarg);
					  break;
#pragma GCC diagnostic ignored "-Wold-style-cast"
			case 'p': port = (uint16_t)atoi(optarg);
					  break;	
#pragma GCC diagnostic error "-Wold-style-cast"
			case 't': thread = atoi(optarg);
					  break;
			default:  std::cerr << "usage: <ip> <port> <thread>" << std::endl;
			          return 1;
		}	
	}

	std::cout << "==================pingpong=================" << std::endl;
	TcpServer srv(ipStr, port, thread, pingpong);
	srv.startServer();

	return 0;
}


