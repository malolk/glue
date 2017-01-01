#ifndef GLUE_LIBBASE_TIMEUTIL_H_
#define GLUE_LIBBASE_TIMEUTIL_H_

#include <string>

#include <sys/time.h>
#include <time.h> 
#include <string.h>

namespace glue_libbase {
class TimeUtil {
 public:
  static std::string ToStringOfMicros(int64_t micro_sec);
  static std::string ToStringOfSeconds(int64_t micro_sec);
  static std::string NowTime();
  static struct timeval NowTimeval();
  static int64_t NowMicros();
  static int64_t NowSeconds();
  static int NowDay();
  static void StartTime(int64_t& start_micros);
  static int64_t ElapsedMicros(const int64_t& start_micros);
  static int64_t ElapsedSeconds(const int64_t& start_micros);
 private:
  static int ToStringOfMicros(char* buf, int len, int flag);
};
}
#endif // GLUE_LIBBASE_TIMEUTIL_H_
