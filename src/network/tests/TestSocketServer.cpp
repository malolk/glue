#include <network/Socket.h>
#include <network/SocketAddress.h>

#include <string>

using namespace network::csocket;
using namespace network;
using namespace libbase;

int main()
{
	Socket serverSocket(true);
	LOGINFO(std::to_string(serverSocket.isServerSocket()));
	serverSocket.enableNonBlock(0);
	serverSocket.enableAddrReuse(1);
	serverSocket.enablePortReuse(1);
	LOGINFO("Blocking IO");
	serverSocket.bind("127.0.0.1", 8080);
	LOGINFO("Bind success");
	serverSocket.listen();
	LOGINFO("Listen...");
	
	while(1)
	{
		SocketAddress clientAddr;
		SocketAddress localAddr;
		int cliFd = serverSocket.accept(clientAddr);
		if (cliFd >= 0)
		{
			LOGINFO(std::string("Receive Connect from client: ") + clientAddr.toString());
			Socket connSocket(cliFd);
			connSocket.enableNonBlock(0);
			connSocket.getPeerName(clientAddr);
			connSocket.getSockName(localAddr);
			LOGINFO(std::string("Local address: ") + localAddr.toString()
			+ std::string("Remote address: ") + clientAddr.toString());
			while(1)
			{
				ByteBuffer buf;
				ssize_t readNum = connSocket.recvBytes(buf);
				std::cout << "received : " << buf.toString() << std::endl;
				if (readNum == 0)
				{
					connSocket.close();
					LOGINFO("peer closed");
					break;
				}else if(readNum > 0)
				{
					buf.appendBytes(std::string(" : echo from") + localAddr.toString());
					connSocket.sendBytes(buf);
				}
				else 
				{
					connSocket.close();
					LOGINFO("read error");
				}
			}
		}
	}	
}
