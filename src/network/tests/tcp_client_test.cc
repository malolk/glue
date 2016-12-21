#include "network/tcp_client.h"
#include "network/socket_address.h"
#include "network/connection.h"

#include <memory>

glue_network::TcpClient* client_ptr = NULL;
void ReadCallback(std::shared_ptr<glue_network::Connection> conn, glue_network::ByteBuffer& buf) {
  static int cnt = 0;
  LOG_INFO("client receive %d bytes on %d times", buf.ReadableBytes(), cnt++);
  conn->Send(buf);
  if (cnt == 10) {
    client_ptr->Close();
  }
}

void InitCallback(std::shared_ptr<glue_network::Connection> conn, glue_network::ByteBuffer& buf) {
  conn->Send(buf);
}

int main() {
  using namespace std::placeholders;
  const char content[] = "give you, give me back.";
  glue_network::ByteBuffer buf;
  buf.Append(content, sizeof(content));
  glue_network::SocketAddress server_addr;
  glue_network::TcpClient client(server_addr, 10);
  client_ptr = &client;
  client.Initialize(std::bind(InitCallback, _1, std::ref(buf)), 
                    ReadCallback);
  client.Start();
}
