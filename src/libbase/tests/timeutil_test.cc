//#include <libbase/TimeStamp.h>
#include "libbase/timeutil.h"

#include <iostream>

int main() {
  using namespace libbase;
  int64_t elapsed = 0;
  {
    TimeCounter counter(&elapsed);
    std::cout << TimeUtil::NowTime() << std::endl;
    std::cout << TimeUtil::NowTimeSeconds() << std::endl;
    struct timeval tv = TimeUtil::NowTimeval();
    std::cout << "Now Time: seconds : " << tv.tv_sec << " useconds: " << tv.tv_usec << std::endl;
  }
  std::cout << "Elapsed seconds: " << elapsed << std::endl;
  return 0;
}



