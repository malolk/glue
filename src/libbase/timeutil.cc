#include "libbase/timeutil.h"

#include <string>

#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <errno.h>

namespace glue_libbase {
int TimeUtil::ToStringOfMicros(char* buf, int len, int flag) {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t sec = tv.tv_sec;
  struct tm* time_store = localtime(&sec);
  int size = static_cast<int>(strftime(buf, len, "%Y-%m-%d %H:%M:%S", time_store));
  if (size == 0) return -1; /* not enough space to store the content */
  if (flag) { /* add microseconds */
    len -= size;
    buf += size;
    size = snprintf(buf, len, ".%06ld", tv.tv_usec);
    if(size > 0 && size <= len) {
      return 1;
    } if (size > len) {
      return 0; /* no enough space */
    } else {
      return -1;  
    }
  } else {
    return 1;
  }
}

std::string TimeUtil::ToStringOfMicros(int64_t micro_sec) {
  char buf[128] = {'\0'}; /* It's enough to store the time string */
  ToStringOfMicros(buf, sizeof(buf), 1);
  return std::string(buf, strlen(buf));
}

std::string TimeUtil::ToStringOfSeconds(int64_t micro_sec) {
  char buf[128] = {'\0'};
  ToStringOfMicros(buf, sizeof(buf), 0);
  return std::string(buf, strlen(buf)); 
}

std::string TimeUtil::NowTime() {
    return ToStringOfMicros(NowMicros());
}

struct timeval TimeUtil::NowTimeval() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv; 
}

int64_t TimeUtil::NowMicros() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return (tv.tv_sec * 1000000LL + tv.tv_usec);
}

int64_t TimeUtil::NowSeconds() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv.tv_sec;
}

// return the day num in the whole year.
int TimeUtil::NowDay() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  time_t sec = tv.tv_sec;
  struct tm* time_store = localtime(&sec);
  return time_store->tm_yday;
}

void TimeUtil::StartTime(int64_t& start_micros) {
  start_micros = NowMicros();
}

int64_t TimeUtil::ElapsedMicros(const int64_t& start_micros) {
  return (NowMicros() - start_micros);
}

int64_t TimeUtil::ElapsedSeconds(const int64_t& start_micros) {
  return (NowMicros() - start_micros) / 1000000LL;
}
} // namespace glue_libbase
