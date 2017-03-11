#include "network/timer_queue.h"
#include "network/epoll.h"
#include "libbase/loggerutil.h"

#include <string.h>

namespace network {
const int TimerQueue::max_num_one_shot_ = 1000;
bool CompareTimer(const Timer& lhs, const Timer& rhs) {
  return (lhs.GetExpiration() < rhs.GetExpiration());
}

namespace {
void SetTimerFd(int fd, int64_t expiration) {
  LOG_CHECK(fd >= 0, "");
  struct itimerspec new_val;
  bzero(&new_val, sizeof(new_val));
  new_val.it_value.tv_sec = expiration / 1000000LL;
  new_val.it_value.tv_nsec = (expiration - new_val.it_value.tv_sec * 1000000LL) * 1000L;
  int ret = timerfd_settime(fd, 0, &new_val, NULL);
  LOG_CHECK(ret == 0, "");
}

void StartTimerFd(int fd, int64_t when) {
  /* Here, use the time difference relative to current time to setting timerfd. */
  int64_t diff = when - libbase::TimeUtil::NowMicros();
  if (diff < 0)  {
    diff = 0;
  }
  SetTimerFd(fd, diff);
}

void StopTimerFd(int fd) {
  SetTimerFd(fd, 0);
}

void CallbackWrite() {
}

void CallbackClose() {
}
}

/* 
* across thread: delTimer won't affect the readTimerQueueChannel
* same thread: delTimer maybe affect the readTimerQueueChannel, 
* e.g. TimerA could delete TimerB while timeout, then TimerB 
* should't be restart when reset timerfd
*/
void TimerQueue::DelTimer(TimerIdType* id) {
  LOG_CHECK(id != NULL, "");
  epoll_ptr_->RunNowOrLater(std::bind(&TimerQueue::DelTimerInLoop, this, std::ref(*id)));	
}

void TimerQueue::DelTimerInLoop(TimerIdType& id) {
  epoll_ptr_->MustInLoopThread();
  timer_pool_.Delete(id);
}
 
/* thread-safe */
void TimerQueue::AddTimer(TimerIdType* id, const Timer& timer) {
  epoll_ptr_->RunNowOrLater(std::bind(&TimerQueue::AddTimerInLoop, this, id, std::ref(timer)));
}

void TimerQueue::AddTimerInLoop(TimerIdType* id, const Timer& timer) {
  bool is_new_min = true;
  /* Check whether the comming timer is the new-minimum. */
  if (!timer_pool_.Empty()) {
    if (CompareTimer(timer_pool_.Top(), timer)) {
      is_new_min = false;  
    }	
  }
  if (id) {
    /* Transfer the TimerId to the user. */
    *id = timer_pool_.Insert(timer);
  } else {
    timer_pool_.Insert(timer);
  }
  if (is_new_min) {
    /* Resetting the timeout time. */
    StartTimerFd(timer_fd_, timer.GetExpiration());
  }
}

void TimerQueue::ReadTimerChannel() {
  uint64_t buf;
  ssize_t ret = ::read(timer_fd_, &buf, sizeof(uint64_t));
  LOG_CHECK(ret == static_cast<ssize_t>(sizeof(uint64_t)), "timerfd read error");
  
  int timeout_num = 0;
  int64_t now_time = libbase::TimeUtil::NowMicros();
  while (!timer_pool_.Empty() && timeout_num < max_num_one_shot_) {
    if (timer_pool_.Top().GetExpiration() <= now_time) {
      timer_pool_.Top().Timeout();
      if (timer_pool_.Top().IsRepeated()) {
        /* Reinsert the top element to the heap. */
        timer_pool_.Top().Update();
        timer_pool_.Sink(0);        
      } else {
        timer_pool_.Pop();
      }
      ++timeout_num;
    } else {
      break;
    }
  }
  ResetTimerFd();	
}

void TimerQueue::ResetTimerFd() {
  if (timer_pool_.Empty()) {
    /* No any active timers, so stop the timer channel. */
    StopTimerFd(timer_fd_);
  } else {
    /* Setting the timer as the time of top element in heap. */
	StartTimerFd(timer_fd_, timer_pool_.Top().GetExpiration()); 
  }
}

void TimerQueue::Initialize() {
  timer_fd_ = timerfd_create(CLOCK_MONOTONIC, TFD_NONBLOCK | TFD_CLOEXEC);
  LOG_CHECK(timer_fd_ >= 0, "");
  timer_chann_.Initialize(std::bind(&TimerQueue::ReadTimerChannel, this), CallbackWrite, CallbackClose, timer_fd_);
  timer_chann_.AddIntoLoopWithRead();
}
} // namespace network
