#ifndef GLUE_NETWORK_EVENTLOOP_H_
#define GLUE_NETWORK_EVENTLOOP_H_

#include "epoll.h"
#include "connection.h"
#include "timer.h"
#include "timer_queue.h"
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
  EventLoop() : running_(false), epoll_ptr_(NULL), timer_queue_ptr_(NULL), 
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
  void DeleteConnection(Connection* conn_ptr);
  void DeleteConnectionInLoop(std::shared_ptr<Connection> conn_shared_ptr);
  
  void RunTimer(TimerQueue::TimerIdType* id, const Timer& timer);
  void CancelTimer(TimerQueue::TimerIdType* id);

 protected:
  void Routine();
  std::atomic<bool> running_;
  Epoll* epoll_ptr_;
  TimerQueue* timer_queue_ptr_;
  glue_libbase::Thread thread_;
  glue_libbase::MutexLock mu_;
  glue_libbase::CondVar condvar_;
  /* Here, every eventloop contains its own connection pool. */
  typedef std::unordered_map<Connection*, std::shared_ptr<Connection>> ConnectionPoolType;
  ConnectionPoolType conn_pool_;
};
} // namespace glue_network
#endif // GLUE_NETWORK_EVENTLOOP_H_
