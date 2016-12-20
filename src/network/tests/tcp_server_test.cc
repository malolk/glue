#include "../socket_address.h"
#include "../socket.h"
#include "../event_channel.h"
#include "../epoll.h"
#include "../connection.h"
#include "../acceptor.h"
#include "../buffer.h"
#include "../eventloop.h"
#include "../eventloop_pool.h"
#include "../tcp_server.h"

#include <unordered_map>
#include <algorithm>
#include <functional>

typedef glue_network::Connection Connection;
typedef std::shared_ptr<Connection> ConnectionType;
typedef std::unordered_map<Connection*, ConnectionType> ConnectionPoolType;

/* Data processing logic should be put here. */ 
void ReadConnection(ConnectionType conn_ptr, 
                    glue_network::ByteBuffer& recv_buf) {
  LOG_INFO("sent %d bytes on fd=%d", recv_buf.ReadableBytes(), conn_ptr->Fd());
  conn_ptr->Send(recv_buf); 
}

void RunSingleThreadEpollServer() {
  glue_network::SocketAddress server_addr; /* Default server address: 127.0.0.1:8080. */
  glue_network::TcpServer server(server_addr, 3, ReadConnection);
  server.Run();
}

int main() {
  RunSingleThreadEpollServer();
  return 0;	
}
