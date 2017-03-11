#include "network/tcp_server.h"
#include "network/socket_address.h"
#include "network/connection.h"
#include "network/buffer.h"

#include <iostream>
#include <string>
#include <memory>

#include <unistd.h>  // for getopt
#include <stdlib.h>  // for atoi

using namespace network;

// your read-callback
void ReadCallback(std::shared_ptr<Connection>& conn, ByteBuffer& buf) {
  // echo the data back
  conn->Send(buf);  
  std::cout << buf.ReadableBytes() << std::endl;
}

int main(int argc, char* argv[]) {
  std::string ip_str = "127.0.0.1";
  uint16_t port = 8080;
  int thread_num = 4;
  int opt;
  while ((opt = getopt(argc, argv, "s:p:t:")) != -1) {
    switch (opt) {
      case 's': {
        ip_str = std::string(optarg);
        break;
      }
      case 'p': {
        port = static_cast<uint16_t>(atoi(optarg));
        break;
      }
      case 't': {
        thread_num = atoi(optarg);
        break;
      }
      default:  {
        std::cerr << "usage: " 
                     "-s <ip> -p <port> -t <thread num>"
                  << std::endl;
        return 1; 
      }
    }    
  }
  SocketAddress server_addr(ip_str, port);
  TcpServer srv(server_addr, thread_num, ReadCallback);
  std::cout << "===========echo server============" << std::endl;
  srv.Run();

  return 0;
}


