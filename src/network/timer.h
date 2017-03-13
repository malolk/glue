#ifndef GLUE_NETWORK_TIMER_H_
#define GLUE_NETWORK_TIMER_H_

#include "libbase/noncopyable.h"
#include "libbase/timeutil.h"
#include "libbase/loggerutil.h"

#include <functional>

namespace network {
class Timer {
 public:
  enum TimeScale {
    kSECOND = 1000000,
    kMILLIS = 1000,
    kMICROS = 1
  };

  typedef std::function<void()> CallbackTimeoutType;
  /* when interval == 0, the timer will just timeout once at most. Otherwise, the next timeout time 
   * will be the sum of current time and the interval. */
  Timer(const CallbackTimeoutType& timeout_cb, int64_t expiration, int64_t interval = 0, int64_t grid = kSECOND)
    : expiration_(0), interval_(0), timeout_cb_(timeout_cb) {
    LOG_CHECK(expiration >= 0 && interval >= 0, "");
    int64_t now_micros = libbase::TimeUtil::NowMicros();
    expiration_ = now_micros + expiration * grid;
    interval_ = interval * grid;
    if (!timeout_cb_) {
      LOG_FATAL("Every timer should be equipped with a valide timeout callback.");
    }
	repeated_ = (interval_ == 0 ? false : true);
    if (repeated_) {
      expiration_ = now_micros + interval_;
    }
  }

  ~Timer() {
  }
  
  void Timeout() { 
    timeout_cb_(); 
  }
  
  void Update() {
    LOG_CHECK(repeated_, "");
    expiration_ = libbase::TimeUtil::NowMicros() + interval_;
  }
	
  int64_t GetExpiration() const {
    return expiration_;
  }

  bool IsRepeated() const { 
    return repeated_; 
  }

 private:
  int64_t expiration_;
  int64_t interval_;
  bool repeated_;
  CallbackTimeoutType timeout_cb_;
};
} // namespace network
#endif // GLUE_NETWORK_TIMER_H_
