#ifndef GLUE_NETWORK_TCPCLIENT_H_
#define GLUE_NETWORK_TCPCLIENT_H_

#include "buffer.h"
#include "socket.h"
#include "socket_address.h"
#include "epoll.h"
#include "connection.h"
#include "connector.h"
#include "../libbase/noncopyable.h"
#include "../libbase/logger.h"
#include "../libbase/mutexlock.h"

#include <memory>
#include <functional>

namespace glue_network {
class TcpClient: private glue_libbase::Noncopyable {
 public:
  TcpClient(const SocketAddress& server_addr, int max_runs)
    : max_runs_(max_runs), sockfd_(-1), server_addr_(server_addr) {
    LOG_CHECK(max_runs_ >= 0, "");
  }
	
  ~TcpClient() {
  }

  void Initialize(const Connection::CallbackInitType& init_cb, 
                  const Connection::CallbackReadType& read_cb); 
  void Start();
  void Close();
  void Stop();

 private:
  /* When connect failed, we'll try max_runs to connect the server before exit. */
  void DeleteInLoop(std::shared_ptr<Connection> conn_shared_ptr);
  int max_runs_; 
  int sockfd_;
  SocketAddress server_addr_;
  Connection::CallbackInitType init_cb_;
  Connection::CallbackReadType read_cb_;
  Connection::CallbackCloseType close_cb_;
  std::shared_ptr<Connection> conn_;
  Epoll* epoll_ptr_;
};	
} // namespace glue_network
#endif // GLUE_NETWORK_TCPCLIENT_H_
