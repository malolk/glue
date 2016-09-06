#include <httpd/HttpServer.h>

using namespace network::httpd;

int main()
{
	HttpServer httpServer("127.0.0.1", 8080, 4);
	httpServer.start();

	return 0;
}


