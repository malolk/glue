#ifndef GLUE_LIBBASE_THREADPOOL_H_
#define GLUE_LIBBASE_THREADPOOL_H_

#include "libbase/loggerutil.h"
#include "libbase/mutexlock.h"
#include "libbase/condvar.h"
#include "libbase/thread.h"
#include "libbase/noncopyable.h"
#include "libbase/bounded_blocking_queue.h"

#include <functional>
#include <deque>
#include <vector>
#include <atomic>

namespace libbase {
class ThreadPool: private Noncopyable {
 public:
  enum RunningState {
    kSTARTUP = 1,
	kRUNNING = 2,
    kLEAVING = 3,
	kSTOP = 4,
  };
  
  typedef std::function<void()> TaskType;
  explicit ThreadPool(int size) 
    : size_(size), threads_(size), state_(kSTARTUP), quit_(false), queue_(size) {
    LOG_CHECK(size > 0, "At least one thread in ThreadPool");
  } 
  ~ThreadPool() {
    LOG_TRACE();
	if (state_ != kSTOP) {
	  Stop();
    }
  }
  
  void Start(); 
  void Add(const TaskType& task);
  void Stop();

 private:
  void ThreadRoutine();
  int size_;
  std::vector<Thread*> threads_;
  RunningState state_;
  std::atomic<bool> quit_;
  BoundedBlockingQueue<TaskType> queue_;
};

}  // namespace libbase 
#endif //GLUE_LIBBASE_THREADPOOL_H_
