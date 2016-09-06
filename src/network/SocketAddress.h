#ifndef NETWORK_SOCKETADDRESS_H
#define NETWORK_SOCKETADDRESS_H

#include <libbase/Debug.h>

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace network
{
namespace csocket
{
// copyable, default copy constructor and assignment operator is ok.
class SocketAddress
{
public:
	typedef struct sockaddr_storage 	AddrType;
	typedef struct sockaddr 			AddrTypeVoid;
	typedef struct sockaddr_in 			AddrType4;
	typedef struct sockaddr_in6 		AddrType6;

	SocketAddress() 
	{
		*this = SocketAddress(std::string("127.0.0.1"), 8080);	
	}

	SocketAddress(const AddrType& addr_in): addr(addr_in)
	{}

#pragma GCC diagnostic ignored "-Wold-style-cast"
	SocketAddress(const AddrTypeVoid& addr_in): 
	addr(*(AddrType*)(&addr_in))
	{}

	SocketAddress(const AddrType4& addr_in): 
	addr(*(AddrType*)(&addr_in))
	{}

	SocketAddress(const AddrType6& addr_in): 
	addr(*(AddrType*)(&addr_in))
	{}

	SocketAddress(const std::string& ipStr, uint16_t port)
	{
		AddrType4 addr4;
		addr4.sin_family = AF_INET;
		addr4.sin_port = htons(port);
		inet_pton(AF_INET, ipStr.c_str(), &(addr4.sin_addr));
		addr = *(AddrType*)(&addr4);
		CHECK(addr.ss_family == AF_INET);
	}

	~SocketAddress() {}

	uint16_t getPort() const
	{
		CHECK(addr.ss_family == AF_INET || addr.ss_family == AF_INET6);
		uint16_t ret = 0;
		if (addr.ss_family == AF_INET)
		{
			AddrType4 *addr4 = (AddrType4*)(&addr);
			ret = ntohs(addr4->sin_port);	
		}
		else if (addr.ss_family == AF_INET6)
		{
			AddrType6 *addr6 = (AddrType6*)(&addr);
			ret = ntohs(addr6->sin6_port);	
		}	
		return ret;
	}

	std::string getIpStr() const
	{
		CHECK(addr.ss_family == AF_INET || addr.ss_family == AF_INET6);
		char addrBuf[INET6_ADDRSTRLEN];
		if (getFamily() == AF_INET)
			inet_ntop(addr.ss_family, &(getAddrType4()->sin_addr), addrBuf, INET6_ADDRSTRLEN);	
		else
			inet_ntop(addr.ss_family, &(getAddrType6()->sin6_addr), addrBuf, INET6_ADDRSTRLEN);	

		return std::string(addrBuf);
	}

	AddrTypeVoid* getAddrTypeVoid()
	{
		CHECK(addr.ss_family == AF_INET || addr.ss_family == AF_INET6);
		return (AddrTypeVoid*)(&addr);	
	}

	const AddrTypeVoid* getAddrTypeVoid() const
	{
		CHECK(addr.ss_family == AF_INET || addr.ss_family == AF_INET6);
		return (AddrTypeVoid*)(&addr);	
	}

	const AddrType4* getAddrType4() const
	{
		CHECK(addr.ss_family == AF_INET);
		return (AddrType4*)(&addr);	
	}

	const AddrType6* getAddrType6() const
	{
		CHECK(addr.ss_family == AF_INET6);
		return (AddrType6*)(&addr);	
	}

	socklen_t getAddrLength() const
	{
		CHECK(addr.ss_family == AF_INET || addr.ss_family == AF_INET6);
		if (addr.ss_family == AF_INET)
			return sizeof(*(AddrType4*)(&addr));	
		else
			return sizeof(*(AddrType6*)(&addr));	
	}
#pragma GCC diagnostic error "-Wold-style-cast"

	uint16_t getFamily() const
	{
		return addr.ss_family;	
	}
	
	std::string toString() const
	{
		uint16_t port = getPort();
		std::string ipStr = getIpStr();
		return (ipStr + std::string(":") + std::to_string(port));
	}
private:
	AddrType addr;		
};
	
}
}
#endif // NETWORK_SOCKETADDRESS_H

