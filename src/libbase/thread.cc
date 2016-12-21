#include "libbase/thread.h"

namespace glue_libbase {
namespace glue_libbase_thread {
  __thread pid_t cached_tid = 0;

  pid_t GetTid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
  }
} // namespace glue_libbase_thread

pid_t ThreadId() {
  if (0 == glue_libbase_thread::cached_tid) {
    glue_libbase_thread::cached_tid = glue_libbase_thread::GetTid();
  }
  return glue_libbase_thread::cached_tid;
}

namespace {
/* Fork safe */
void AfterFork() {
  glue_libbase_thread::cached_tid = 0;
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

int Thread::Start() {
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
int Thread::Schedule(const FuncType& task){
  int ret = 1;
  if (running_) {
    bqueue_.Insert(task);
  } else {
    LOG_WARN("Thread(%s) is not running", name_.c_str());
    ret = 0;
  }
  return ret;
}

int Thread::Join() {
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
  LOG_INFO("Thread[%d] started.", process_id_);
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
    LOG_WARN("Thread[%d] exits with %d unfinished task(s)", process_id_, unfinished_task_num);
  }
  LOG_INFO("Thread[%d] finished.", process_id_);
  running_ = false;
}
} // namespace glue_libbase

