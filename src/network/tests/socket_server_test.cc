#include "network/socket.h"
#include "network/socket_address.h"

#include <string>

void TestIpv4() {
  using namespace glue_network;
  int fd = Socket::NewSocket();
  Socket::EnableNonBlock(fd, 0);
  Socket::EnableAddrReuse(fd, 1);
  Socket::EnablePortReuse(fd, 1);
  SocketAddress server_addr("127.0.0.1", 8080);
  Socket::BindAndListen(fd, server_addr, 100);

  LOG_INFO("Listen on %s ...", server_addr.ToString().c_str());
  while (true) {
    SocketAddress client_addr;
    int cli_fd = Socket::Accept(fd, &client_addr);
    Socket::EnableNonBlock(cli_fd, 0); // Blocking I/O
    if (cli_fd >= 0) {
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
          LOG_ERROR("Error on receiving from peer : %s on sockfd=%d ", client_addr.ToString().c_str(), cli_fd);
          break;
        }
        buf.Reset();
      }
    }
  }	
}

void TestIpv6() {
  using namespace glue_network;
  int fd = Socket::NewSocket(AF_INET6, SOCK_STREAM, 0);
  Socket::EnableNonBlock(fd, 0);
  Socket::EnableAddrReuse(fd, 1);
  Socket::EnablePortReuse(fd, 1);
  SocketAddress server_addr("::1", 8080, 1);
  Socket::BindAndListen(fd, server_addr, 100);

  LOG_INFO("Listen on %s ...", server_addr.ToString().c_str());
  while (true) {
    SocketAddress client_addr;
    int cli_fd = Socket::Accept(fd, &client_addr);
    Socket::EnableNonBlock(cli_fd, 0); // Blocking I/O
    if (cli_fd >= 0) {
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
          LOG_ERROR("Error on receiving from peer : %s on sockfd=%d ", client_addr.ToString().c_str(), cli_fd);
          if (read_num != Socket::kNODATA) {
            Socket::Close(cli_fd);
            break;
          }
        }
        buf.Reset();
      }
    }
  }	
}

int main() {
  TestIpv4();

  return 0;
}

