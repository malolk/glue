#include "network/socket_address.h"

typedef network::SocketAddress::AddrType AddrType;
typedef network::SocketAddress::AddrType4 AddrType4;

int main() {
  //local address
  network::SocketAddress sa;
  
  LOG_INFO("SocketAddress: %s", sa.ToString().c_str());
  LOG_INFO("SocketAddress's length=%d", sa.Length());
  LOG_INFO("AddrType's length=%d", sizeof(AddrType));
  LOG_INFO("AddrType4's length=%d", sizeof(AddrType4));
  return 0;		
}


