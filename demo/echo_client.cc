#include "network/tcp_client.h"

#include <iostream>
#include <string>

using namespace network;
using namespace libbase;

void ReadCallback(std::shared_ptr<SocketConnection::Connection> conn, ByteBuffer& buf) {
  std::cout << buf.ToString() << std::endl;
  buf.Reset();
  std::string data;
  std::cin >> data;
  buf.AppendString(data);
  conn->Send(buf);  
  std::cout << buf.ReadableBytes() << std::endl;  
}

void InitCallback(std::shared_ptr<SocketConnection::Connection> conn) {
  std::string data;
  std::cin >> data;
  ByteBuffer tmp;
  tmp.AppendString(data);
  conn->Send(tmp);  
}

int main() {
  Epoll epoller;
  epoller.Initialize();
  
  SocketAddress server_addr("127.0.0.1", 8080);
  TcpClient client(server_addr, 10);
  client.Initialize(InitCallback, ReadCallback);
  client.Start(&epoller);
  epoller.Run();
  return 0;  
}


