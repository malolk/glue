#ifndef GLUE_NETWORK_SOCKETADDRESS_H_
#define GLUE_NETWORK_SOCKETADDRESS_H_

#include "libbase/loggerutil.h"

#include <string>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

namespace glue_network {

/* Copyable, default copy constructor and assignment operator is ok. */
class SocketAddress {
 public:
  typedef struct sockaddr_storage 	AddrType;
  typedef struct sockaddr 			AddrTypeVoid;
  typedef struct sockaddr_in 		AddrType4;
  typedef struct sockaddr_in6 		AddrType6;

  SocketAddress() {
	*this = SocketAddress(std::string("127.0.0.1"), 8080);	
  }

  SocketAddress(const AddrType& addr): addr_(addr){
  }

#pragma GCC diagnostic ignored "-Wold-style-cast"
  SocketAddress(const AddrTypeVoid& addr): addr_(*(AddrType*)(&addr)) {
  }

  SocketAddress(const AddrType4& addr): addr_(*(AddrType*)(&addr)) {
  }
  
  SocketAddress(const AddrType6& addr): addr_(*(AddrType*)(&addr)) {
  }

  SocketAddress(const std::string& ip_str, uint16_t port, bool is_ipv6 = false) {
    memset(&addr_, 0, sizeof(addr_));
    if (!is_ipv6) {
	  AddrType4 addr4;
	  addr4.sin_family = AF_INET;
	  addr4.sin_port = htons(port);
	  inet_pton(AF_INET, ip_str.c_str(), &(addr4.sin_addr));
	  addr_ = *(AddrType*)(&addr4);
	  LOG_CHECK(addr_.ss_family == AF_INET, "");
    } else {
      AddrType6 addr6;
      addr6.sin6_family = AF_INET6;
      addr6.sin6_port = htons(port);
      inet_pton(AF_INET6, ip_str.c_str(), &(addr6.sin6_addr));
      LOG_CHECK(sizeof(addr_) >= sizeof(addr6), "");
      addr_= *(AddrType*)(&addr6);
      LOG_CHECK(addr_.ss_family == AF_INET6, "");
    }
  }

  ~SocketAddress() {
  }

  uint16_t Port() const {
	LOG_CHECK(addr_.ss_family == AF_INET || addr_.ss_family == AF_INET6, "");
	uint16_t ret = 0;
	if (addr_.ss_family == AF_INET) {
	  AddrType4 *addr4 = (AddrType4*)(&addr_);
	  ret = ntohs(addr4->sin_port);
	} else if (addr_.ss_family == AF_INET6) {
	  AddrType6 *addr6 = (AddrType6*)(&addr_);
	  ret = ntohs(addr6->sin6_port);	
	}	
	return ret;
  }

  std::string IpStr() const {
	LOG_CHECK(addr_.ss_family == AF_INET || addr_.ss_family == AF_INET6, "");
	char addr_buf[INET6_ADDRSTRLEN];
	if (Family() == AF_INET) {
	  inet_ntop(addr_.ss_family, &(ToAddrType4()->sin_addr), addr_buf, INET6_ADDRSTRLEN);	
    } else {
	  inet_ntop(addr_.ss_family, &(ToAddrType6()->sin6_addr), addr_buf, INET6_ADDRSTRLEN);	
    }
	return std::string(addr_buf);
  }

  AddrTypeVoid* ToAddrTypeVoid() {
	LOG_CHECK(addr_.ss_family == AF_INET || addr_.ss_family == AF_INET6, "");
	return (AddrTypeVoid*)(&addr_);	
  }

  const AddrTypeVoid* ToAddrTypeVoid() const {
	LOG_CHECK(addr_.ss_family == AF_INET || addr_.ss_family == AF_INET6, "");
	return (const AddrTypeVoid*)(&addr_);	
  }

  const AddrType4* ToAddrType4() const {
	LOG_CHECK(addr_.ss_family == AF_INET, "");
	return (AddrType4*)(&addr_);	
  }

  const AddrType6* ToAddrType6() const {
	LOG_CHECK(addr_.ss_family == AF_INET6, "");
	return (AddrType6*)(&addr_);	
  }

  socklen_t Length() const {
	LOG_CHECK(addr_.ss_family == AF_INET || addr_.ss_family == AF_INET6, "");
	if (addr_.ss_family == AF_INET) {
	  return sizeof(*(AddrType4*)(&addr_));	
    } else {
	  return sizeof(*(AddrType6*)(&addr_));	
    }
  }
#pragma GCC diagnostic error "-Wold-style-cast"

  uint16_t Family() const {
	return addr_.ss_family;	
  }

  void Clear() {
    memset(&addr_, 0, sizeof(addr_));
  }
  std::string ToString() const {
	uint16_t port = Port();
	std::string ip_str = IpStr();
	return (ip_str + std::string(":") + std::to_string(port));
  }
 private:
  AddrType addr_;		
};
}  // namespace glue_network
#endif // GLUE_NETWORK_SOCKETADDRESS_H_

