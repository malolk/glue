#include <network/TcpServer.h>
#include <network/Connection.h>
#include <network/Buffer.h>

#include <iostream>
#include <string>
#include <memory>

#include <unistd.h>  // for getopt
#include <stdlib.h>  // for atoi

using namespace network;


// your read-callback
void readCallback(std::shared_ptr<SocketConnection::Connection>& conn, ByteBuffer& buf)
{
	// echo the data back
	conn->sendData(buf);	
	std::cout << buf.readableBytes() << std::endl;
//	buf.reset(); 
}

int main(int argc, char* argv[])
{
	std::string ipStr = "127.0.0.1";
	uint16_t port = 8080;
	int threadNum = 4;
	int opt;
	while((opt = getopt(argc, argv, "s:p:t:")) != -1)
	{
		switch(opt)
		{
			case 's': ipStr = std::string(optarg);
					  break;
			case 'p': port = static_cast<uint16_t>(atoi(optarg));
					  break;
		    case 't': threadNum = atoi(optarg);
			          break;
			default:  std::cerr << "usage: " 
			          "-s <ip> -p <port> -t <thread num>"
					  << std::endl;
					  return 1;	
		}		
	}
	TcpServer srv(ipStr, port, threadNum);
	std::cout << "===========echo server============" << std::endl;
	srv.setReadOp(readCallback);
	srv.startServer();

	return 0;
}


