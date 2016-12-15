//#include <libbase/TimeStamp.h>
#include "../timeutil.h"

#include <iostream>

int main() {
  using namespace glue_libbase;
  int64_t start_micros = 0;
  TimeUtil::StartTime(start_micros);
  std::cout << "start time in micros: " << start_micros << std::endl;
  int64_t now_seconds = TimeUtil::NowSeconds();
  std::cout << "now time in seconds: " << now_seconds << std::endl;
  std::cout << TimeUtil::ToStringOfMicros(start_micros) << std::endl;
  std::cout << TimeUtil::ToStringOfSeconds(start_micros) << std::endl;
  struct timeval tv = TimeUtil::NowTimeval();
  std::cout << "Now Time: seconds : " << tv.tv_sec << " useconds: " << tv.tv_usec << std::endl;
  std::cout << "Elapsed micros: " << TimeUtil::ElapsedMicros(start_micros) << std::endl;
  std::cout << "Elapsed seconds: " << TimeUtil::ElapsedSeconds(start_micros) << std::endl;

  return 0;
}



