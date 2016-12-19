#ifndef GLUE_NETWORK_EVENTLOOP_H_
#define GLUE_NETWORK_EVENTLOOP_H_

#include "epoll.h"
#include "connection.h"
#include "../libbase/noncopyable.h"
#include "../libbase/logger.h"
#include "../libbase/thread.h"
#include "../libbase/mutexlock.h"
#include "../libbase/condvar.h"

#include <atomic>
#include <memory>
#include <unordered_map>
#include <functional>

namespace glue_network {
class EventLoop: private glue_libbase::Noncopyable {
 public:
  EventLoop() : epoll_ptr_(NULL), thread_(), 
                mu_(), condvar_(mu_), running_(false) {
  }
	
  /* epoll_ptr_ no need to be deleted, epoll sits on stack */
  ~EventLoop() {
    if (running_ && !thread_.IsJoined()) {
	  Join();
	}
  }

  Epoll* Start();
  Epoll* EpollPtr() const {
    return epoll_ptr_;
  }
  int Join() {
    LOG_CHECK(!thread_.IsJoined(), "");
    return thread_.Join();
  }


  /* Usage: when master thread accepted a connection, 
   * it would wrap this function as a callback to 
   * transfet a new connection to the corresponding epoll.
   * This version will be used in server. */
  void NewConnectionInClient(int fd, const Connection::CallbackReadType& read_cb,
                             const Connection::CallbackInitType& init_cb);
  /* This version will be used in server. */
  void NewConnection(int fd, const Connection::CallbackReadType& read_cb);
  void DeleteConnection(Connection* conn_ptr);
  void DeleteConnectionInLoop(std::shared_ptr<Connection> conn_shared_ptr);
 protected:
  void Routine();
  Epoll* epoll_ptr_;
  glue_libbase::Thread thread_;
  glue_libbase::MutexLock mu_;
  glue_libbase::CondVar condvar_;
  std::atomic<bool> running_;
  /* Here, every eventloop contains its own connection pool. */
  typedef std::unordered_map<Connection*, std::shared_ptr<Connection>> ConnectionPoolType;
  ConnectionPoolType conn_pool_;
};
} // namespace glue_network
#endif // GLUE_NETWORK_EVENTLOOP_H_
