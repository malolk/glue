#ifndef NETWORK_SOCKET_H
#define NETWORK_SOCKET_H

#include <network/Buffer.h>
#include <network/SocketAddress.h>
#include <libbase/Debug.h>
#include <libbase/Noncopyable.h>

#include <string>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

namespace network
{

namespace csocket
{

using namespace libbase;

enum 
{
	RETRY = -10,
	NOCONN,
	LACKFD,
	ACCERR,
	NODATA,
	RDERR,
	WRERR,
};

const int MAX_QUEUE_SIZE =	100; 

class Socket: private Noncopyable
{
public:
	// beServerIn == true, then the socket would be a listen socket, 
	// otherwise, it would be a common socket.
	explicit Socket(bool beServerIn = false, int domainIn = AF_INET, int typeIn = SOCK_STREAM, int protocolIn = IPPROTO_IP):
		sockfd(Socket::allocate(domainIn, typeIn, protocolIn))
	{
		commonCtor(beServerIn, false, domainIn, typeIn, protocolIn);
	}
	
	explicit Socket(int fd, bool connectedIn = true, int domainIn = AF_INET, int typeIn = SOCK_STREAM, int protocolIn = IPPROTO_IP): sockfd(fd)
	{
		commonCtor(false, connectedIn, domainIn, typeIn, protocolIn);	
		CHECK(connected);
	}
	~Socket()
	{
		//LOGINFO(std::string("Closed on fd: ") + std::to_string(sockfd));
		if (!closed)
			close();
	}
	
	bool isServerSocket() { return beServer; } 	
	bool isClosed() { return closed; }
	void stopWrite() { shutdown(SHUT_WR); }
	void stopRead()  { shutdown(SHUT_RD); }
	void close();

	void bind(const std::string &ipStr, uint16_t port);
	void listen(int backlog = MAX_QUEUE_SIZE);
	
	// should check the ret, EMFILE or ENFILE ?
	int accept(SocketAddress &sa);
	int getSockfd() { return sockfd; }
	void getPeerName(SocketAddress &sa);
	void getSockName(SocketAddress &sa);
    ssize_t recvBytes(ByteBuffer& buf);
    ssize_t sendBytes(ByteBuffer& buf);
	static int connect(const std::string&, uint16_t, int );
	static int allocate(int domainIn = AF_INET, int typeIn = SOCK_STREAM, int protocolIn = IPPROTO_IP);
	void enableAddrReuse(int flag)
	{
		CHECK(setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, static_cast<void *>(&flag), sizeof flag) == 0);
	}

	void enablePortReuse(int flag)
	{
		CHECK(setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, static_cast<void *>(&flag), sizeof flag) == 0);
	}

	void enableKeepAlive(int flag)
	{
		CHECK(setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, static_cast<void *>(&flag), sizeof flag) == 0);
	}

	void enableTcpNoDelay(int flag)
	{
		CHECK(setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, static_cast<void *>(&flag), sizeof flag) == 0);
	}

	void enableNonBlock(int flag)
	{
		int val = fcntl(sockfd, F_GETFL, 0);
		CHECK((val >= 0));
		val = (flag == 1 ? (val | O_NONBLOCK) : (val & (~O_NONBLOCK)));
		CHECK((fcntl(sockfd, F_SETFL, val) >= 0)); 
	}
private:
	void commonCtor(bool beServer, bool connected, int domain, int type, int protocol);

	void setConnected() { CHECK(!connected); connected = true; }
	int shutdown(int how) 
	{  
		int ret = ::shutdown(sockfd, how);
		if(ret) LOGERROR(errno);
		return ret;
	}

	int sockfd;		
	int domain;
	int type;
	int protocol;
	bool closed; 
	bool connected;   // for nomal socket
	bool binded;      // for listener
	bool beServer;
};

inline
void Socket::close()
{
	CHECK(!closed);
	CHECK(::close(sockfd) == 0);	
	closed = true;
}

}

}
#endif // NETWORK_SOCKET_H
