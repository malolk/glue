#include <httpd/HttpServer.h>

#include <string>
#include <iostream>

#include <unistd.h>
#include <stdio.h>

using namespace network::httpd;

int main(int argc, char* argv[])
{
	std::string ipStr = "127.0.0.1";
	uint16_t port = 8080;
	int threadCount = 4;

	int opt;
	while((opt = getopt(argc, argv, "h:p:t:")) != -1)
	{
		switch(opt)
		{
			case 'h' : ipStr = std::string(optarg);
					   break;	
#pragma GCC diagnostic ignored "-Wold-style-cast"
			case 'p' : port = (uint16_t)atoi(optarg);
					   break;	
#pragma GCC diagnostic error "-Wold-style-cast"
			case 't' : threadCount = atoi(optarg);
					   break;	
			default  : std::cerr << "usage: <ip> <port> <thread count>" << std::endl;
					   return 1; 
					
		}	
	}
	HttpServer httpServer(ipStr, port, threadCount);
	httpServer.start();

	return 0;
}


