#include "network/socket.h"
#include "network/socket_address.h"

#include <string>

#include <unistd.h>

void TestIpv6() {
  using namespace glue_network;
  int fd = Socket::NewSocket(AF_INET6, SOCK_STREAM, 0);
  Socket::EnableAddrReuse(fd, 1);
  Socket::EnablePortReuse(fd, 1);
  Socket::EnableNonBlock(fd, 0);
  SocketAddress server_addr("::1", 8080, 1);
  Socket::Connect(fd, server_addr);
  ::sleep(2);
  SocketAddress client_addr;
  Socket::GetSockName(fd, client_addr);
  LOG_INFO("\nSocket: %s connected to ::1:8080\n", client_addr.ToString().c_str());

  glue_libbase::ByteBuffer buf;
  int cnt = 10;
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

void TestIpv4() {
  using namespace glue_network;
  int fd = Socket::NewSocket();
  Socket::EnableAddrReuse(fd, 1);
  Socket::EnablePortReuse(fd, 1);
  Socket::EnableNonBlock(fd, 0);
  SocketAddress server_addr("127.0.0.1", 8080);
  Socket::Connect(fd, server_addr);
  ::sleep(2);
  SocketAddress client_addr;
  Socket::GetSockName(fd, client_addr);
  LOG_INFO("\nSocket: %s connected to 127.0.0.1:8080\n", client_addr.ToString().c_str());

  glue_libbase::ByteBuffer buf;
  int cnt = 10;
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

int main() {
  TestIpv4();
  
  return 0;
}
