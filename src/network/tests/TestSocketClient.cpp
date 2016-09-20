#include <network/Socket.h>
#include <network/SocketAddress.h>

#include <string>
#include <iostream>

#include <unistd.h>

using namespace network::csocket;
using namespace network;
using namespace libbase;

int main()
{
	while(1)
	{
		Socket clientSocket(false);
		SocketAddress sa;
		clientSocket.enableAddrReuse(1);
		clientSocket.enablePortReuse(1);
		clientSocket.enableNonBlock(0);
		clientSocket.connect("127.0.0.1", 8080, clientSocket.getSockfd());
		sleep(5);
		clientSocket.getSockName(sa);
		LOGINFO("\n\n\nconnect normally");
		ByteBuffer buf;
		while(1)
		{
			buf.appendBytes(sa.toString());
			ssize_t sentBytes = clientSocket.sendBytes(buf);
			LOGINFO(std::string("sent bytes ") + std::to_string(sentBytes));
			(void)sentBytes;
			clientSocket.recvBytes(buf);
			std::cout << "received : " << buf.toString() << "\n\n\n";
			buf.reset();
			sleep(2);
		}
		clientSocket.close();
	}
	return 0;	
}
