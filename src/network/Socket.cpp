#include <network/Socket.h>
#include <libbase/Debug.h>

#include <sys/uio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

using namespace network;
using namespace network::csocket;


namespace 
{

class IgnoreSigpipe
{
public:
	IgnoreSigpipe()
	{
		struct sigaction act;
		memset(&act, '\0', sizeof(act));
		act.sa_handler = SIG_IGN;
		act.sa_flags = 0;
		CHECK(::sigaction(SIGPIPE, &act, NULL) == 0);
	}
};

const IgnoreSigpipe sigpipeInit;

}


void Socket::commonCtor(bool beServerIn, bool connectedIn, int domainIn, int typeIn, int protocolIn)
{
	LOGTRACE();
	CHECKEXIT(sockfd >= 0);
	beServer = beServerIn;
	domain = domainIn;
	type = typeIn;
	protocol = protocolIn;
	closed = false;
	binded = false;
	connected = connectedIn;
	enableNonBlock(1);
}

void Socket::bind(const std::string &ipStr, uint16_t port)
{
	LOGTRACE();
	CHECK(beServer);
	SocketAddress sa(ipStr, port);

	CHECK(::bind(sockfd, sa.getAddrTypeVoid(), sa.getAddrLength()) == 0);
	binded = true;
}

void Socket::listen(int backlog)
{
	LOGTRACE();
	CHECK(beServer);
	CHECK(binded);
	CHECK(::listen(sockfd, backlog) == 0);
}

int Socket::accept(SocketAddress &sa)
{
	LOGTRACE();
	CHECK(binded);
	int ret = 0;
	socklen_t addrLen = sizeof(SocketAddress::AddrType);
	ret = ::accept4(sockfd, sa.getAddrTypeVoid(), &addrLen, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (ret < 0)
	{
		int err = errno;
		switch (err)
		{
			case EAGAIN:
				return NOCONN;
			case EINTR:
				LOGWARN("accept4(), EINTR, try again");
				return NOCONN;
			case EBADF:
			case ECONNABORTED:
			case EFAULT:
			case EINVAL:
			case ENOMEM:
			case ENOBUFS:
			case ENOTSOCK:
			case EOPNOTSUPP:
			case EPROTO:
				LOGWARN("accept4() failed");
				return ACCERR;
			case EMFILE:
			case ENFILE:
				LOGWARN("accept4() no enough fd");
				return LACKFD;
			default:
				LOGWARN("accept4() failed");
				return ACCERR;
		}
	}else
	{
		return ret;
	}
}


void Socket::connect(const std::string &ipStr, uint16_t port)
{
	LOGTRACE();
	CHECK(!beServer && !connected);
	SocketAddress sa(ipStr, port);
	int ret = 0;
	while(1)
	{
		ret = ::connect(sockfd, sa.getAddrTypeVoid(), sa.getAddrLength());
		if (ret < 0)
		{
			int err = errno;
			if (err == EAGAIN)  continue;
			else LOGERROR(err);
		}else 
			break;
	}
	setConnected();	
}

void Socket::getPeerName(SocketAddress& sa)
{
	LOGTRACE();
	CHECK(connected);
	socklen_t addrLen = sizeof(SocketAddress::AddrType);

	CHECK(::getpeername(sockfd, sa.getAddrTypeVoid(), &addrLen) == 0);
} 

void Socket::getSockName(SocketAddress &sa)
{
	LOGTRACE();
	CHECK(connected || binded);
	socklen_t addrLen = sizeof(SocketAddress::AddrType);

	CHECK(::getsockname(sockfd, sa.getAddrTypeVoid(), &addrLen) == 0);
}

namespace network
{
	const int ANOTHER_BUF_SIZE = 64*1024;
}

ssize_t Socket::recvBytes(ByteBuffer& buf)
{
	LOGTRACE();
	CHECK(!closed);
	char anotherBuf[ANOTHER_BUF_SIZE];     
	struct iovec vecOfBuf[2];
	const size_t baseBufSize = buf.writableBytes();
	vecOfBuf[0].iov_base = buf.addrOfWrite();     
	vecOfBuf[0].iov_len = baseBufSize;
	vecOfBuf[1].iov_base = anotherBuf;
	vecOfBuf[1].iov_len = ANOTHER_BUF_SIZE;

	ssize_t numOfReceived = ::readv(sockfd, vecOfBuf, 2);
	if(numOfReceived < 0)
	{
		int err = errno;
		if (err == EAGAIN || err == EWOULDBLOCK || err == EINTR)
		{
			LOGINFO("no data in buffer");
			return NODATA;
		}else
		{
			LOGERROR(err);
			return RDERR;
		}
	}else if(numOfReceived > 0)
	{
		if(static_cast<size_t>(numOfReceived) > baseBufSize)
		{
			buf.movePosOfWrite(baseBufSize);
			buf.appendBytes(anotherBuf, static_cast<size_t>(numOfReceived - baseBufSize));
		}else
			buf.movePosOfWrite(static_cast<size_t>(numOfReceived));
	}

	return numOfReceived;
}


// Note: handle sigpipe error?
ssize_t Socket::sendBytes(ByteBuffer& buf)
{
	LOGTRACE();
	CHECK(!closed);
	const size_t sendableBytes = buf.readableBytes();
	ssize_t sentBytes = ::write(sockfd, buf.addrOfRead(), sendableBytes);
	if (sentBytes < 0)
	{
		int err = errno;
		if (err == EAGAIN || err == EWOULDBLOCK)
		{
			LOGINFO("no avaliable space to write");
			return 0;
		}
		else if (err == EPIPE)
		{
		//	LOGERROR(err);
			return WRERR;
		}
		else
		{
			LOGERROR(err);
			return WRERR;
		}
	}
	else
	{
		LOGTRACE();
		if (sentBytes > 0)
			buf.movePosOfRead(static_cast<size_t>(sentBytes));
		LOGTRACE();
	}
	return sentBytes;
}
