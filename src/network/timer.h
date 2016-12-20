#ifndef GLUE_NETWORK_TIMER_H_
#define GLUE_NETWORK_TIMER_H_

#include "../libbase/noncopyable.h"
#include "../libbase/timeutil.h"
#include "../libbase/logger.h"

#include <functional>

namespace glue_network {
class Timer {
 public:
  typedef std::fuction<void()> CallbackTimeoutType;
  /* when interval == 0, the timer will just timeout once at most. Otherwise, the next timeout time 
   * will be the sum of current time and the interval. */
  Timer(const CallbackTimeoutType& timeout_cb, int64_t expiration, int64_t interval = 0)
    : expiration_(expiration), interval_(interval), timeout_cb_(timeout_cb) {
    LOG_CHECK(interval_ >= 0, "");
    if (!timout_cb_) {
      LOG_FATAL("Every timer should be equipped with a valide timeout callback.");
    }
	repeated_ = (interval_ == 0 ? false : true);
    if (repeated_) {
      expiration_ = glue_libbase::TimeUtil::NowMicros() + interval_;
    }
  }

  ~Timer() {
  }
  
  void Timeout() { 
    timeout_cb_(); 
  }
  
  void Update() {
    LOG_CHECK(repeated_, "");
    expiration_ = glue_libbase::TimeUtil::NowMicros() + interval_;
  }
	
  int64_t GetExpiration() const {
    return expiration;
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
} // namespace glue_network
#endif // GLUE_NETWORK_TIMER_H_
