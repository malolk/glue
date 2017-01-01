#include "../loggerutil.h"

int main() {
  LOG_TRACE();

  LOG_INFO("Test LOG_INFO");
  LOG_INFO("Test LOG_INFO %d %s", 2, "times");

  LOG_WARN("Test LOG_WARN");
  LOG_WARN("Test LOG_WARN %d %s", 2, "times");

  
  LOG_ERROR("Test LOG_ERROR");
  LOG_ERROR("Test LOG_ERROR %d %s", 2, "times");

//  LOG_CHECK(0 == 1, "Test LOG_CHECK");
//  LOG_FATAL("Test LOG_FATAL");
}
