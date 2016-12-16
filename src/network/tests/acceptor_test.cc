#include "../acceptor.h"
#include "../socket.h"
#include "../socket_address.h"

#include <string>

void TestCase(const char* ip_str, uint16_t port, bool is_ipv6) {
  using namespace glue_network;
  SocketAddress server_addr(ip_str, port, is_ipv6);
  Acceptor acceptor(server_addr, 0); /* Make listen_fd_ blocking I/O in this test. */
  LOG_INFO("Listen on %s ...", server_addr.ToString().c_str());
  while (true) {
    SocketAddress client_addr;
    /* Default I/O is non-blocking, see Socket::Accept() for details. */
    int cli_fd = acceptor.Accept();
    Socket::EnableNonBlock(cli_fd, 0); /* Make it blocking for this test. */
    if (cli_fd >= 0) {
      Socket::GetSockName(cli_fd, client_addr);
      LOG_INFO("Receive Connect from client: %s ", client_addr.ToString().c_str());
      ByteBuffer buf;
      while (true) {
        ssize_t read_num = Socket::Receive(cli_fd, buf);  
        if (read_num == 0) {
          Socket::Close(cli_fd);
          LOG_INFO("peer: %s closed", client_addr.ToString().c_str());
          break;
        } else if (read_num > 0) {
          LOG_INFO("Received data from peer: %s ", buf.ToString().c_str());
          Socket::Send(cli_fd, buf);
        } else {
          Socket::Close(cli_fd);
          LOG_ERROR("Error on receiving from peer : %s on sockfd=%d ", 
                    client_addr.ToString().c_str(), cli_fd);
          break;
        }
        buf.Reset();
      }
    }
  }	
}

void TestIpv4(const char* ip_str, uint16_t port) {
  TestCase(ip_str, port, 0);
}

void TestIpv6(const char* ip_str, uint16_t port) {
  TestCase(ip_str, port, 1);
}

int main() {
//  TestIpv4("127.0.0.1", 8080);
  TestIpv6("::1", 8080);
  return 0;
}

