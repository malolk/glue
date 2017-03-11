#ifndef GLUE_NETWORK_EVENTLOOP_POOL_H_
#define GLUE_NETWORK_EVENTLOOP_POOL_H_

#include "network/epoll.h"
#include "network/eventloop.h"
#include "network/event_channel.h"
#include "libbase/loggerutil.h"

#include <vector>
#include <memory>

namespace network {
class EventLoopPool: private libbase::Noncopyable {
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
} // network
#endif // GLUE_NETWORK_EVENTLOOP_POOL_H_
