#ifndef GLUE_NETWORK_TCPSERVER_H_
#define GLUE_NETWORK_TCPSERVER_H_

#include "network/socket.h"
#include "network/socket_address.h"
#include "network/epoll.h"
#include "network/connection.h"
#include "network/eventloop_pool.h"
#include "network/acceptor.h"
#include "libbase/buffer.h"
#include "libbase/mutexlock.h"
#include "libbase/condvar.h"
#include "libbase/noncopyable.h"
#include "libbase/loggerutil.h"

#include <string>
#include <memory>
#include <functional>
#include <map>
#include <vector>
#include <atomic>

#include <unistd.h>

namespace glue_network {
class TcpServer: public EventLoop {
 public:
  explicit TcpServer(const SocketAddress& server_addr, 
                     int pool_size, const Connection::CallbackReadType read_cb) 
    : dispatch_counter_(0), pool_size_(pool_size), running_(false), mu_(), condvar_(mu_), read_cb_(read_cb), 
      server_addr_(server_addr), acceptor_(server_addr), eventloop_pool_(pool_size - 1) {
    LOG_CHECK(pool_size_ > 0, "");
    if (!read_cb) {
      LOG_FATAL("Connection read callback should be valide");
    }
  }
	
  ~TcpServer() {
  }
  /* Start in current thread. */
  void Run() {
    Routine(); 
  }

  /* Start in another thread. */
  void StartInThread();
  /* Stop the server in another thread. */
  void Shutdown();
  
 private:
  void Routine();
  void ReadListenSocket();
  static void WriteListenSocket() {
  }
  static void CloseListenSocket() {
  }
  
  int dispatch_counter_;
  const int pool_size_;
  std::atomic<bool> running_;
  glue_libbase::MutexLock mu_;
  glue_libbase::CondVar condvar_; /* For synchronously exiting. */
  Connection::CallbackReadType read_cb_;
  SocketAddress server_addr_;
  Acceptor acceptor_;
  EventLoopPool eventloop_pool_;
};
} // namespace glue_network
#endif // GLUE_NETWORK_TCPSERVER_H_
