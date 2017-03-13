#ifndef GLUE_NETWORK_TIMERQUEUE_H_
#define GLUE_NETWORK_TIMERQUEUE_H_

#include "network/timer.h"
#include "network/event_channel.h"
#include "libbase/heap.h"
#include "libbase/noncopyable.h"
#include "libbase/timeutil.h"
#include "libbase/loggerutil.h"

#include <utility>
#include <memory>
#include <set>

#include <sys/timerfd.h>
#include <unistd.h>

namespace network {
bool CompareTimer(const network::Timer&, const network::Timer&);
class TimerQueue: private libbase::Noncopyable {
 public:
  typedef std::weak_ptr<libbase::Element<Timer>> TimerIdType;
  explicit TimerQueue(Epoll* epoll_ptr) 
    : epoll_ptr_(epoll_ptr), timer_fd_(-1), timer_chann_(epoll_ptr), timer_pool_(4, CompareTimer) { 
  }

  ~TimerQueue() {
    if (timer_fd_ >= 0) {
	  ::close(timer_fd_);
    }
  }

  void Initialize();
  /* User could use id to update or delete timer. 
   * id would be tied to the timer when id was not NULL. */
  void AddTimer(TimerIdType* id, const Timer& timer);
  void DelTimer(TimerIdType* id);
  static bool IsTimerExpired(TimerIdType* id) {
    LOG_CHECK(id != NULL, "");
    return id->expired();
  }

 private:
  void AddTimerInLoop(TimerIdType* id, const Timer& timer);
  void DelTimerInLoop(TimerIdType& timer_id);
  void ReadTimerChannel();
  void ResetTimerFd();

  Epoll* epoll_ptr_;
  int timer_fd_;
  EventChannel timer_chann_;
  libbase::Heap<Timer> timer_pool_;

  /* Prevent from handling too many timeouts in one time. */
  static const int max_num_one_shot_;
};
} // namespace network
#endif // GLUE_NETWORK_TIMERQUEUE_H_
