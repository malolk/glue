#include "network/tcp_client.h"
#include "network/socket_address.h"
#include "network/connection.h"

#include <memory>

network::TcpClient* client_ptr = NULL;
void ReadCallback(std::shared_ptr<network::Connection> conn, libbase::ByteBuffer& buf) {
  static int cnt = 0;
  LOG_INFO("client receive %d bytes on %d times", buf.ReadableBytes(), cnt++);
  conn->Send(buf);
  if (cnt == 10) {
    client_ptr->Close();
  }
}

void InitCallback(std::shared_ptr<network::Connection> conn, libbase::ByteBuffer& buf) {
  conn->Send(buf);
}

int main() {
  using namespace std::placeholders;
  const char content[] = "give you, give me back.";
  libbase::ByteBuffer buf;
  buf.Append(content, sizeof(content));
  network::SocketAddress server_addr;
  network::TcpClient client(server_addr, 10);
  client_ptr = &client;
  client.Initialize(std::bind(InitCallback, _1, std::ref(buf)), 
                    ReadCallback);
  client.Start();
}
