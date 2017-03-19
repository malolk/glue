#include "libbase/timeutil.h"

#include <string>

#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

namespace libbase {
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

std::string TimeUtil::NowTime() {
  char buf[128] = {'\0'}; /* It's enough to store the time string */
  ToStringOfMicros(buf, sizeof(buf), 1);
  return std::string(buf, strlen(buf));
}

std::string TimeUtil::NowTimeSeconds() {
  char buf[128] = {'\0'};
  ToStringOfMicros(buf, sizeof(buf), 0);
  return std::string(buf, strlen(buf)); 
}

struct timeval TimeUtil::NowTimeval() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv; 
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

int64_t TimeUtil::ElapsedSeconds(const struct timeval& start) {
  return NowSeconds() - start.tv_sec;
}

int64_t TimeUtil::ElapsedMicros(const struct timeval& start) {
  struct timeval diff = DiffTimeval(start, NowTimeval());
  return diff.tv_sec * 1000000 + diff.tv_usec;
}

void TimeUtil::AddMicros(struct timeval& t, int64_t micros) {
  int64_t m = t.tv_usec + micros;
  t.tv_sec += m / 1000000;
  t.tv_usec = m % 1000000;
}

void TimeUtil::AddSeconds(struct timeval& t, int64_t secs) { 
  t.tv_sec += secs;
}

struct timeval TimeUtil::DiffTimeval(const struct timeval& lhs, 
                                     const struct timeval& rhs) { 
  time_t diff_seconds = lhs.tv_sec - rhs.tv_sec;
  long diff_usecs = lhs.tv_usec - rhs.tv_usec;
  if (diff_usecs < 0) {
    diff_usecs += 1000000;
    diff_seconds -= 1;
  }
  struct timeval diff = {diff_seconds, diff_usecs};
  return diff;
}

int TimeUtil::CompareTimeval(const struct timeval& lhs, 
                              const struct timeval& rhs) {
  if (lhs.tv_sec < rhs.tv_sec) {
    return -1;
  } else if (lhs.tv_sec > rhs.tv_sec) {
    return 1;
  } else if (lhs.tv_usec < rhs.tv_usec) {
    return -1;
  } else if (lhs.tv_usec > rhs.tv_usec) {
    return 1;
  }
  return 0;
}

void TimeUtil::WaitInSeconds(int secs) {
  assert(secs >= 0);
  struct timespec tm;
  tm.tv_sec = secs;
  tm.tv_nsec = 0;
  int ret = ::nanosleep(&tm, NULL);
  if (ret != 0) {
    fprintf(stderr, "%s: %d nanosleep errors: %d %s\n", 
            __func__, __LINE__, ret, strerror(ret));
  }
}

} // namespace libbase
