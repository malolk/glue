#include "network/socket_address.h"

typedef glue_network::SocketAddress::AddrType AddrType;
typedef glue_network::SocketAddress::AddrType4 AddrType4;

int main() {
  //local address
  glue_network::SocketAddress sa;
  
  LOG_INFO("SocketAddress: %s", sa.ToString().c_str());
  LOG_INFO("SocketAddress's length=%d", sa.Length());
  LOG_INFO("AddrType's length=%d", sizeof(AddrType));
  LOG_INFO("AddrType4's length=%d", sizeof(AddrType4));
  return 0;		
}


