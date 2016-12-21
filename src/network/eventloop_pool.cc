#include "network/eventloop_pool.h"

namespace glue_network {
EventLoop* EventLoopPool::NextEventLoop() {
  LOG_CHECK(pool_size_ >= 1, "pool is empty");
  EventLoop* ret = pool_[dispatch_id_++].get();
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
    pool_[index]->Stop();
  }
	
  for (int index = 0; index < pool_size_; ++index) {
	pool_[index]->Join();
  }	
}
} // namespace glue_network
