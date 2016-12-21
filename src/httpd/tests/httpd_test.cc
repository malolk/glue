#include "httpd/http_server.h"
#include "network/socket_address.h"

#include <string>

#include <unistd.h>
#include <stdio.h>

int main(int argc, char* argv[]) {
  std::string ip_str = "127.0.0.1";
  uint16_t port = 8080;
  int thread_num = 4;

  int opt;
  while ((opt = ::getopt(argc, argv, "h:p:t:")) != -1) {
    switch (opt) {
      case 'h' : {
        ip_str = std::string(optarg);
        break;  
      }
#pragma GCC diagnostic ignored "-Wold-style-cast"
      case 'p' : { 
        port = (uint16_t)atoi(optarg);
        break;  
      }
#pragma GCC diagnostic error "-Wold-style-cast"
      case 't' : {
        thread_num = atoi(optarg);
        break;  
      }
      default  : {
        std::cerr << "usage: <ip> <port> <thread count>" << std::endl;
        return 1; 
      }     
    }
  }
  glue_network::SocketAddress server_addr(ip_str, port);
  glue_httpd::HttpServer http_server(server_addr, thread_num);
  http_server.Start();

  return 0;
}


