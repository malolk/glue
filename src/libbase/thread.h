#ifndef GLUE_LIBBASE_THREAD_H_
#define GLUE_LIBBASE_THREAD_H_

/*
#include <libbase/noncopyable.h>
#include <libbase/condvar.h>
#include <libbase/mutexlock.h>
#include <libbase/blocking_queue.h>
#include <libbase/debug.h>
*/

#include "logger.h"
#include "noncopyable.h"
#include "condvar.h"
#include "mutexlock.h"
#include "blocking_queue.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

#include <functional>
#include <iostream>
#include <atomic>

namespace glue_libbase {

/* Get pid of current thread. */
pid_t tid();

class Thread: private Noncopyable {
 public:
  typedef std::function<void()> FuncType;
  explicit Thread(const std::string& name = "untitled_thread")
    : process_id_(0), thread_id_(0), name_(name), 
      joined_(false), running_(false), quit_flag_(false), 
      mu_(), condvar_(mu_), bqueue_() {
  }

  int Start() {
    if (running_) {
      LOG_WARN("Thread[%d] has already started", process_id_);
      return 1;
    }

    int ret = pthread_create(&thread_id_, NULL, &RunWrapper, this);
    LOG_CHECK(ret == 0, "pthread_create failed");
    /* Wait thread to start */
    MutexLockGuard m(mu_);
    while (!running_) {
      condvar_.Wait();
    }
    return (ret == 0);
  }

  /* TODO: No copies for large objects */
  int Schedule(const FuncType& task){
    int ret = 1;
    if (running_) {
      bqueue_.Insert(task);
    } else {
      LOG_WARN("Thread(%s) is not running", name_.c_str());
      ret = 0;
    }
    return ret;
  }

  int Join() {
    LOG_CHECK(!joined_, "Joined one thread mulitple times");
    LOG_CHECK(running_, "The thread is not started");

    /* A NULL functor could stop the thread */
    Schedule(FuncType(NULL));
    joined_ = true;
    int ret = pthread_join(thread_id_, NULL);
    LOG_CHECK(ret == 0, "pthread_join failed");
    running_ = false;
    return (ret == 0);
  }

  int Stop() {
    if (!running_ || quit_flag_) {
      return 1;
    } else {
      quit_flag_ = true;
      return Join();
    }
  }

  bool IsRunning() { return running_; }
  bool IsJoined() { return joined_; }
  int TaskNum() { return bqueue_.size(); }
  pid_t GetThreadId() { return thread_id_; }
   std::string name() { return name_; }

  ~Thread() {
    LOG_CHECK(joined_, "Thread instance should be joined before exit");
  }

 private:
  /* TODO: some optimizations should be taken to avoid copying functors */
  void Run() {
    process_id_ = tid();
    {
      MutexLockGuard m(mu_);
      running_ = true;
      condvar_.NotifyOne();
    }
    LOG_INFO("Thread[%d] started.", process_id_);
    while(true) {
      FuncType task = bqueue_.Get();
      if (!task) {
      /* A NULL task means an EXIT operation for current thread. */
        break;
      } else {
        task();
      }
      if (quit_flag_) {
        break;
      }
    }
    int unfinished_task_num = bqueue_.size();
    if (unfinished_task_num) {
      LOG_WARN("Thread[%d] exits with %d unfinished task(s)", process_id_, unfinished_task_num);
    }
    LOG_INFO("Thread[%d] finished.", process_id_);
    running_ = false;
  }

  static void* RunWrapper(void* arg) {
    (reinterpret_cast<Thread*>(arg)->Run)();
    return NULL;
  }

  pid_t process_id_;
  pthread_t thread_id_;
  std::string name_; 
  bool joined_;
  std::atomic<bool> running_;
  std::atomic<bool> quit_flag_;

  /* synchronizing startup */
  MutexLock mu_;
  CondVar condvar_;
  BlockingQueue<FuncType> bqueue_;
};
}

#endif  // GLUE_LIBBASE_THREAD_H_

