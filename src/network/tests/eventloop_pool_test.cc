#include "network/socket_address.h"
#include "network/socket.h"
#include "network/event_channel.h"
#include "network/epoll.h"
#include "network/connection.h"
#include "network/acceptor.h"
#include "network/eventloop.h"
#include "network/eventloop_pool.h"
#include "libbase/buffer.h"

#include <unordered_map>
#include <algorithm>
#include <functional>

typedef network::Connection Connection;
typedef std::shared_ptr<Connection> ConnectionType;
typedef std::unordered_map<Connection*, ConnectionType> ConnectionPoolType;

/* Data processing logic should be put here. */ 
void ReadConnection(ConnectionType conn_ptr, 
                    libbase::ByteBuffer& recv_buf) {
  conn_ptr->Send(recv_buf);  
}

void ReadListenSocket(network::Acceptor* acceptor,
                      network::EventLoopPool* eventloop_pool) {
  network::SocketAddress conn_addr;
  while (true) {
    int ret = acceptor->Accept();
	if (ret >= 0) {
        network::EventLoop* eventloop_ptr = eventloop_pool->NextEventLoop();
        network::Epoll* epoll_ptr = eventloop_ptr->EpollPtr();
        LOG_INFO("Accept connection on fd=%d, and transfer to epoll in thread=%d", ret, epoll_ptr->ThreadId());
        epoll_ptr->RunNowOrLater(std::bind(&network::EventLoop::NewConnection, eventloop_ptr, ret, ReadConnection));
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

void RunServer() {
  network::Epoll epoller;
  epoller.Initialize();
  network::EventLoopPool eventloop_pool(2);
  eventloop_pool.Start();
  network::SocketAddress server_addr; /* Default server address: 127.0.0.1:8080. */
  network::Acceptor acceptor(server_addr); /* Make listen socket non-blocking. */
  network::EventChannel acceptor_chann(&epoller, acceptor.Fd());
  acceptor_chann.Initialize(std::bind(ReadListenSocket, &acceptor, &eventloop_pool), 
                            WriteListenSocket, CloseListenSocket);
  acceptor_chann.AddIntoLoopWithRead();
  epoller.Run();
}

int main() {
  RunServer();
  return 0;	
}
