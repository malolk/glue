#include <network/TcpServer.h>

using namespace network;

int main()
{
	TcpServer srv("127.0.0.1", 8080, 1);
	srv.startServer();

	return 0;
}
