#include "network/connector.h"
#include "network/socket.h"
#include "network/socket_address.h"

#include <string>

#include <unistd.h>

void TestCase(const char* ip_str, uint16_t port, bool is_ipv6 = 0) {
  using namespace glue_network;
  SocketAddress server_addr(ip_str, port, is_ipv6);
  /* Blocking I/O. */ 
  int fd = Connector::GetConnectedSocket(10, server_addr, 0); 
  SocketAddress client_addr;
  Socket::GetSockName(fd, client_addr);

  glue_libbase::ByteBuffer buf;
  int cnt = 2;
  while (cnt-- > 0)	{
	buf.AppendString(client_addr.ToString());
    LOG_INFO("client send content %s", buf.ToString().c_str());
	ssize_t sent_num = Socket::Send(fd, buf);
	LOG_INFO("Client sent %d bytes", sent_num);
	ssize_t recv_num = Socket::Receive(fd, buf);
	LOG_INFO("Client recv %d bytes", recv_num);
	buf.Reset();
	sleep(1);
  }
  Socket::Close(fd);
}

void TestIpv4(const char* ip_str, uint16_t port) {
  TestCase(ip_str, port, 0);
}

void TestIpv6(const char* ip_str, uint16_t port) {
  TestCase(ip_str, port, 1);
}

int main() {
  TestIpv4("127.0.0.1", 8080);
  // TestIpv6("::1", 8080);
  return 0;
}
