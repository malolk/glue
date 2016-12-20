#ifndef GLUE_NETWORK_EVENTLOOP_POOL_H_
#define GLUE_NETWORK_EVENTLOOP_POOL_H_

#include "epoll.h"
#include "eventloop.h"
#include "event_channel.h"
#include "../libbase/logger.h"

#include <vector>
#include <memory>

namespace glue_network {
class EventLoopPool: private glue_libbase::Noncopyable {
 public:
  explicit EventLoopPool(int pool_size) 
    : pool_size_(pool_size), dispatch_id_(0) {
    LOG_CHECK(pool_size_ >= 0, "");
  }

  ~EventLoopPool() {
  }
  
  void Start();
  void Shutdown();
  EventLoop* NextEventLoop();

 private:
  const int pool_size_;  
  int dispatch_id_;
  std::vector<std::shared_ptr<EventLoop>> pool_;
};
} // glue_network
#endif // GLUE_NETWORK_EVENTLOOP_POOL_H_
