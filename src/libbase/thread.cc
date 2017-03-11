#include "libbase/thread.h"

#include <assert.h>
#include <stdio.h>

namespace libbase {
namespace libbase_thread {
  __thread pid_t cached_tid = 0;

  pid_t GetTid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
  }
} // namespace libbase_thread

pid_t ThreadId() {
  if (0 == libbase_thread::cached_tid) {
    libbase_thread::cached_tid = libbase_thread::GetTid();
  }
  return libbase_thread::cached_tid;
}

namespace {
// Fork safe.
void AfterFork() {
  libbase_thread::cached_tid = 0;
  ThreadId();
}

class ThreadIdInitializer {
 public:
  ThreadIdInitializer() {
    ThreadId();
    pthread_atfork(NULL, NULL, &AfterFork);
  }
};

/* Initialize the current thread_id from the startup of the current thread */
ThreadIdInitializer init_thread_id;
} 

Thread::~Thread() {
  if (!joined_ && running_) {
    fprintf(stderr, "Thread instance should be joined before exit");
  }
}

int Thread::Start() {
  if (running_) {
    fprintf(stderr, "Thread[%d] has already started", process_id_);
    return 1;
  }

  int ret = pthread_create(&thread_id_, NULL, &RunWrapper, this);
  assert(ret == 0);
  // Wait thread to start
  {
    MutexLockGuard m(mu_);
    while (!running_) {
      condvar_.Wait();
    }
  }
  return (ret == 0);
}

/* TODO: No copies for large objects */
int Thread::Schedule(const FuncType& task){
  int ret = 1;
  if (running_) {
    bqueue_.Insert(task);
  } else {
    abort();
  }
  return ret;
}

int Thread::Join() {
  assert(!joined_);
  assert(running_);

  /* A NULL functor could stop the thread */
  Schedule(FuncType(NULL));
  joined_ = true;
  int ret = pthread_join(thread_id_, NULL);
  assert(ret == 0);
  running_ = false;
  return (ret == 0);
}

int Thread::Stop() {
  if (!running_ || quit_flag_) {
    return 1;
  } else {
    quit_flag_ = true;
    return Join();
  }
}

/* TODO: some optimizations should be taken to avoid copying functors */
void Thread::Run() {
  process_id_ = ThreadId();
  {
    MutexLockGuard m(mu_);
    running_ = true;
    condvar_.NotifyOne();
  }
  while (true) {
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
  size_t unfinished_task_num = bqueue_.size();
  if (unfinished_task_num) {
    fprintf(stderr, "Thread[%d] exits with %d unfinished task(s)", process_id_, static_cast<int>(unfinished_task_num));
  }
  running_ = false;
}
} // namespace libbase

