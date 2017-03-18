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
  Timer(): expiration_(libbase::TimeUtil::NowTimeval()), 
           interval_(0), timeout_cb_() {
  }

  Timer(const CallbackTimeoutType& timeout_cb, int64_t range, 
        int64_t interval = 0, int64_t grid = kSECOND): timeout_cb_(timeout_cb) {
    LOG_CHECK(range >= 0 && interval >= 0, "");
    struct timeval now = libbase::TimeUtil::NowTimeval();
    expiration_ = now;
    libbase::TimeUtil::AddMicros(expiration_, range * grid);
    interval_ = interval * grid;
    if (!timeout_cb_) {
      LOG_FATAL("Every timer should be equipped with a valide timeout callback.");
    }
	repeated_ = (interval_ == 0 ? false : true);
    if (repeated_) {
      expiration_ = now;
      libbase::TimeUtil::AddMicros(expiration_, interval_);
    }
  }

  ~Timer() {
  }
  
  void Timeout() { 
    timeout_cb_(); 
  }
  
  void Update() {
    LOG_CHECK(repeated_, "");
    expiration_ = libbase::TimeUtil::NowTimeval();
    libbase::TimeUtil::AddMicros(expiration_, interval_);
  }
	
  struct timeval GetExpiration() const {
    return expiration_;
  }

  bool IsRepeated() const { 
    return repeated_; 
  }

 private:
  struct timeval expiration_; 
  int64_t interval_;   // microseconds
  bool repeated_;
  CallbackTimeoutType timeout_cb_;
};
} // namespace network
#endif // GLUE_NETWORK_TIMER_H_
