#include "eventloop.h"

namespace glue_network {
void EventLoop::Routine() {
  Epoll epoller;
  epoller.Initialize();
  TimerQueue timer_queue(&epoller);
  timer_queue.Initialize();
  {
    glue_libbase::MutexLockGuard m(mu_);
	epoll_ptr_ = &epoller;
    timer_queue_ptr_ = &timer_queue;
	condvar_.NotifyOne();
  }
  running_ = true;
  epoller.Run();
  running_ = false;
}

Epoll* EventLoop::Start() {
  thread_.Start();
  thread_.Schedule(std::bind(&EventLoop::Routine, this));
  {
    glue_libbase::MutexLockGuard m(mu_);
	while (epoll_ptr_ == NULL) {
	  condvar_.Wait();
    }
  }
  return epoll_ptr_;
}

void EventLoop::RunTimer(TimerQueue::TimerIdType* id, const Timer& timer) {
  LOG_CHECK(timer_queue_ptr_, "");
  timer_queue_ptr_->AddTimer(id, timer);
} 

void EventLoop::CancelTimer(TimerQueue::TimerIdType* id) {
  LOG_CHECK(timer_queue_ptr_, "");
  timer_queue_ptr_->DelTimer(id); 
}

void EventLoop::NewConnection(int fd, const Connection::CallbackReadType& read_cb) {
  LOG_CHECK(fd >= 0, "");
  std::shared_ptr<Connection> conn_ptr(new Connection(fd, epoll_ptr_));
  conn_pool_[conn_ptr.get()] = conn_ptr;
  conn_ptr->SetReadOperation(read_cb);
  conn_ptr->SetCloseOperation(std::bind(&EventLoop::DeleteConnection, this, conn_ptr.get()));
  conn_ptr->Initialize();
}

/* When connection is closing, it will be invoked. 
 * Note: we can't pass the shared_ptr of connection to this callback, otherwise, 
 * it won't be destroyed since it exits in the connection itself. */
void EventLoop::DeleteConnection(Connection* conn_ptr) {
  ConnectionPoolType::iterator it = conn_pool_.find(conn_ptr);
  LOG_CHECK(it != conn_pool_.end(), "");
  std::shared_ptr<Connection> conn_backup = it->second;
  conn_pool_.erase(it);
  /* Use RunLater not RunNowOrLater here, since it could ensure that all 
   * the events of this connection occured before this point will be processed 
   * before the current connection's destruction. */
  epoll_ptr_->RunLater(std::bind(&EventLoop::DeleteConnectionInLoop, this, conn_backup));
  LOG_INFO("connection on fd=%d is closing", conn_ptr->Fd());
}

void EventLoop::DeleteConnectionInLoop(std::shared_ptr<Connection> conn_shared_ptr) {
  conn_shared_ptr->DestroyedInLoop(conn_shared_ptr); /* Delete channel from epoll. */
}
} // namespace glue_network
