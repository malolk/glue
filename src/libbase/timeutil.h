#ifndef GLUE_LIBBASE_TIMEUTIL_H_
#define GLUE_LIBBASE_TIMEUTIL_H_

#include <string>

#include <sys/time.h>
#include <time.h> 
#include <string.h>

namespace libbase {
class TimeUtil {
 public:
  static std::string NowTimeSeconds();
  static std::string NowTime();
  static struct timeval NowTimeval();
  static int64_t NowSeconds();
  static int NowDay();
  static int64_t ElapsedSeconds(const struct timeval& start);
  static int64_t ElapsedMicros(const struct timeval& start);
  static void AddMicros(struct timeval& t, int64_t micros);
  static void AddSeconds(struct timeval& t, int64_t secs);
  static int CompareTimeval(const struct timeval& lhs, 
                            const struct timeval& rhs);
  static struct timeval DiffTimeval(const struct timeval& lhs,
                                    const struct timeval& rhs);
 private:
  static int ToStringOfMicros(char* buf, int len, int flag);
};

class TimeCounter {
 public:
  TimeCounter(int64_t* elapsed, int type = 0)
    : elapsed_(elapsed), type_(type), start(TimeUtil::NowTimeval()) { 
  }

  ~TimeCounter() {
    if (elapsed_) {
      if (!type_) {
        *elapsed_ = TimeUtil::ElapsedMicros(start);
      } else {
        *elapsed_ = TimeUtil::ElapsedSeconds(start);
      }
    }
  }

 private:
  int64_t* elapsed_;
  int type_; // 0: micros, 1: seconds
  struct timeval start;
};

}
#endif // GLUE_LIBBASE_TIMEUTIL_H_
