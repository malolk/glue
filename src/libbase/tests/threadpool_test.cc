#include "../threadpool.h"
#include "../logger.h"
#include "../mutexlock.h"
#include "../waitmember.h"

#include <unistd.h>

#include <iostream>
#include <functional>
#include <atomic>

static const long LOOP = 10;
static const long ANS = (1 + LOOP)*LOOP/2;

glue_libbase::MutexLock mu;
volatile long long global_sum;

void DoSum(long n) {
  glue_libbase::MutexLockGuard m(mu);
  global_sum += n;
}

void TestCase(int size) {
  global_sum = 0;
  glue_libbase::ThreadPool thread_pool(size);
  thread_pool.Start();

  for (int index = 1; index <= LOOP; ++index) {
    int adder = index;
	thread_pool.Add(std::bind(DoSum, adder));
  }

  thread_pool.Stop();
  if (global_sum == ANS) {
    LOG_INFO("TestCase: with thread size %d [PASS] sum = %d, ans = %d", size, global_sum, ANS);
  } else {
    LOG_INFO("TestCase: with thread size %d [FAILED] sum = %d, ans = %d", size, global_sum, ANS);
  }
}

int main(void) {
  TestCase(1);    
  TestCase(3);    
  TestCase(10);    

  return 0;
}
