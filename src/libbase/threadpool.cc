#include "libbase/threadpool.h"

namespace libbase {
void ThreadPool::Start() {
  if (state_ > kSTARTUP) {
    LOG_WARN("ThreadPool is already started with %d threads", size_);
  } else {
    TaskType routine = std::bind(&ThreadPool::ThreadRoutine, this);
    for (int i = 0; i < size_; ++i) {
      threads_[i] = new Thread();
      threads_[i]->Start();
      threads_[i]->Schedule(routine);
    }
  }
  state_ = kRUNNING;
}

void ThreadPool::ThreadRoutine() {
  LOG_TRACE();
  while (true) {
    const TaskType& task = queue_.Get();
    /* Use quit_ to stop now */
    if (!task) {
      /* OK! It's time to leave. */
      break;
    }
    task();
  LOG_TRACE();
  }
  LOG_TRACE();
}

void ThreadPool::Add(const TaskType& task) {
  LOG_CHECK(state_ == kRUNNING, "ThreadPool is not running");
  if (!task) {
    LOG_FATAL("Task is not a runnable");
  }
  queue_.Insert(task);
}

void ThreadPool::Stop() {
  if (state_ == kSTOP) {
    LOG_WARN("Already stop!");
    return;
  } 
  LOG_CHECK(state_ == kRUNNING, "ThreadPool is not running");
  state_ = kLEAVING;
  LOG_TRACE();
  quit_ = true;
  for (int i = 0; i < size_; ++i) {
    /* Add NULL functor to wake the thread */
    queue_.Insert(NULL);
  }
  for (int i = 0; i < size_; ++i) {
    threads_[i]->Join();
    delete threads_[i];
  }
  state_ = kSTOP;
}
}  // namespace libbase 
