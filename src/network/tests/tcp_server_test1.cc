#include "network/socket_address.h"
#include "network/socket.h"
#include "network/event_channel.h"
#include "network/epoll.h"
#include "network/connection.h"
#include "network/acceptor.h"
#include "network/eventloop.h"
#include "network/eventloop_pool.h"
#include "network/tcp_server.h"
#include "libbase/buffer.h"

#include "unistd.h"

#include <unordered_map>
#include <algorithm>
#include <functional>

typedef network::Connection Connection;
typedef std::shared_ptr<Connection> ConnectionType;
typedef std::unordered_map<Connection*, ConnectionType> ConnectionPoolType;

/* Data processing logic should be put here. */ 
void ReadConnection(ConnectionType conn_ptr, 
                    libbase::ByteBuffer& recv_buf) {
  LOG_INFO("sent %d bytes on fd=%d", recv_buf.ReadableBytes(), conn_ptr->Fd());
  conn_ptr->Send(recv_buf); 
}

void RunServer() {
  network::SocketAddress server_addr; /* Default server address: 127.0.0.1:8080. */
  network::TcpServer server(server_addr, 3, ReadConnection);
  server.StartInThread();
  sleep(10);
  server.Shutdown();
}

int main() {
  RunServer();
  return 0;	
}
