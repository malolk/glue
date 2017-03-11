#ifndef GLUE_NETWORK_EVENTLOOP_H_
#define GLUE_NETWORK_EVENTLOOP_H_

#include "network/epoll.h"
#include "network/connection.h"
#include "network/timer.h"
#include "network/timer_queue.h"
#include "libbase/noncopyable.h"
#include "libbase/loggerutil.h"
#include "libbase/thread.h"
#include "libbase/mutexlock.h"
#include "libbase/condvar.h"

#include <atomic>
#include <memory>
#include <unordered_map>
#include <functional>

namespace network {
class EventLoop: private libbase::Noncopyable {
 public:
  EventLoop() : running_(false), epoll_ptr_(NULL), 
                thread_(), mu_(), condvar_(mu_) {
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

  /* Thread-safe. */
  void Stop() {
    LOG_CHECK(epoll_ptr_, "");  
    epoll_ptr_->Stop();
  }

  int Join() {
    LOG_CHECK(!thread_.IsJoined(), "");
    return thread_.Join();
  }

  /* Usage: when master thread accepted a connection, 
   * it would wrap this function as a callback to 
   * transfet a new connection to the corresponding epoll. */
  void NewConnection(int fd, const Connection::CallbackReadType& read_cb);
  void NewConnectionOfClient(int fd, const Connection::CallbackReadType& read_cb,
                                     const Connection::CallbackInitType& init_cb);
  void DeleteConnection(Connection* conn_ptr);
  void DeleteConnectionInLoop(std::shared_ptr<Connection> conn_shared_ptr);
  
 protected:
  void Routine();
  std::atomic<bool> running_;
  Epoll* epoll_ptr_;
  libbase::Thread thread_;
  libbase::MutexLock mu_;
  libbase::CondVar condvar_;
  /* Here, every eventloop contains its own connection pool. */
  typedef std::unordered_map<Connection*, std::shared_ptr<Connection>> ConnectionPoolType;
  ConnectionPoolType conn_pool_;
};
} // namespace network
#endif // GLUE_NETWORK_EVENTLOOP_H_
