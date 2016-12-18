#include "eventloop_pool.h"

namespace glue_network {
Epoll* EventLoopPool::NextEventLoop() {
  Epoll* ret = pool_[dispatch_id_++]->EpollPtr();
  if (dispatch_id_ == pool_size_) {
    dispatch_id_ = 0;
  }
  return ret;
}

void EventLoopPool::Start() {
  pool_.reserve(pool_size_);
  for (int index = 0; index < pool_size_; ++index) {
    pool_.push_back(std::shared_ptr<EventLoop>(new EventLoop()));
    pool_.back()->Start();
  }
}

void EventLoopPool::Shutdown() {
  for (int index = 0; index < pool_size_; ++index) {
    Epoll* epoll_ptr = pool_[index]->EpollPtr();
	epoll_ptr->Stop();
  }
	
  for (int index = 0; index < pool_size_; ++index) {
	pool_[index]->Join();
  }	
}
} // namespace glue_network
