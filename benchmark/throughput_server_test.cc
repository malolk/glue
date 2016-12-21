#include "network/tcp_server.h"
#include "network/socket_address.h"
#include "network/connection.h"
#include "network/buffer.h"

#include <iostream>
#include <memory>
#include <string>

#include <unistd.h>
#include <stdlib.h>

using namespace glue_network;

void pingpong(std::shared_ptr<Connection> conn, ByteBuffer& buf) {
  conn->Send(buf);
  buf.Reset();  
}

int main(int argc, char* argv[]) {
  std::string ip_str = "127.0.0.1";
  uint16_t port = 8080;
  int thread = 2;

  int opt;
  while ((opt = getopt(argc, argv, "s:p:t:")) != -1) {
    switch (opt) {
      case 's': {
        ip_str = std::string(optarg);
        break;
      }
#pragma GCC diagnostic ignored "-Wold-style-cast"
      case 'p': {
        port = (uint16_t)atoi(optarg);
        break; 
      }
#pragma GCC diagnostic error "-Wold-style-cast"
      case 't': {
        thread = atoi(optarg);
        break;
      }
      default: { 
        std::cerr << "usage: <ip> <port> <thread>" << std::endl;
        return 1;
      }
    }  
  }

  std::cout << "==================pingpong=================" << std::endl;

  SocketAddress server_addr(ip_str, port);
  TcpServer srv(server_addr, thread, pingpong);
  srv.Run();

  return 0;
}


