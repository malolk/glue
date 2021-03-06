#include "network/socket_address.h"
#include "network/socket.h"
#include "network/event_channel.h"
#include "network/epoll.h"
#include "network/connection.h"
#include "network/acceptor.h"
#include "libbase/buffer.h"

#include <unordered_map>
#include <algorithm>
#include <functional>

typedef network::Connection Connection;
typedef std::shared_ptr<Connection> ConnectionType;
typedef std::unordered_map<Connection*, ConnectionType> ConnectionPoolType;

void DeleteConnectionInLoop(ConnectionType conn_ptr) {
  conn_ptr->DestroyedInLoop(conn_ptr);  /* Delete channel from epoll. */  
}

/* When connection is closing, it will be invoked. 
 * Note: we can't pass the shared_ptr of connection to this callback, otherwise, 
 * it won't be destroyed since it exits in the connection itself. */
void CloseConnection(network::Epoll* epoll_ptr, ConnectionPoolType& conn_pool, Connection* conn) {
  ConnectionPoolType::iterator it = conn_pool.find(conn);
  LOG_CHECK(it != conn_pool.end(), "");
  ConnectionType conn_backup = it->second;
  conn_pool.erase(it);
  /* Use RunLater not RunNowOrLater here, since it could ensure that all 
   * the events of this connection occured before this point will be processed 
   * before the current connection's destruction. */
  epoll_ptr->RunLater(std::bind(DeleteConnectionInLoop, conn_backup));
}

/* Data processing logic should be put here. */ 
void ReadConnection(ConnectionType conn_ptr, 
                    libbase::ByteBuffer& recv_buf) {
  conn_ptr->Send(recv_buf);  
}

void ReadListenSocket(network::Epoll* epoll_ptr, 
                      network::Acceptor* acceptor,
                      ConnectionPoolType& conn_pool) {
  network::SocketAddress conn_addr;
  while (true) {
    int ret = acceptor->Accept();
	if (ret >= 0) {
      ConnectionType conn_ptr(new network::Connection(ret, epoll_ptr));
	  conn_pool[conn_ptr.get()] = conn_ptr;
      conn_ptr->SetReadOperation(ReadConnection);
      conn_ptr->SetCloseOperation(std::bind(&CloseConnection, epoll_ptr, std::ref(conn_pool), conn_ptr.get()));
      conn_ptr->Initialize();
    } else {
      if (ret == network::Socket::kRETRY) {
        /* Already accept all the connection. */
	    break;
      } else {
        /* Error occured. Maybe there is no enough file descriptors. */ 
        LOG_ERROR("error occured when reading listen socket");
        return;
      }
	}
  }
}


void WriteListenSocket() {
  /* Nothing. */
}
void CloseListenSocket() {
  /* Nothing to do. When server finishes, acceptor destructor would close the listen socket. */
}

void RunSingleThreadEpollServer() {
  network::Epoll epoller;
  epoller.Initialize();
  ConnectionPoolType conn_pool;
  network::SocketAddress server_addr; /* Default server address: 127.0.0.1:8080. */
  network::Acceptor acceptor(server_addr); /* Make listen socket non-blocking. */
  network::EventChannel acceptor_chann(&epoller, acceptor.Fd());
  acceptor_chann.Initialize(std::bind(ReadListenSocket, &epoller, &acceptor, std::ref(conn_pool)), 
                            WriteListenSocket, CloseListenSocket);
  acceptor_chann.AddIntoLoopWithRead();
  epoller.Run();
}

int main() {
  RunSingleThreadEpollServer();
  return 0;	
}
