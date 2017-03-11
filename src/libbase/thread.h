#ifndef GLUE_LIBBASE_THREAD_H_
#define GLUE_LIBBASE_THREAD_H_

#include "libbase/noncopyable.h"
#include "libbase/condvar.h"
#include "libbase/mutexlock.h"
#include "libbase/blocking_queue.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <linux/unistd.h>

#include <functional>
#include <atomic>

namespace libbase {

/* Get pid of current thread. */
pid_t ThreadId();

class Thread: private Noncopyable {
 public:
  typedef std::function<void()> FuncType;
  explicit Thread(const std::string& name = "untitled_thread")
    : process_id_(0), thread_id_(0), name_(name), 
      joined_(false), running_(false), quit_flag_(false), 
      mu_(), condvar_(mu_), bqueue_() {
  }

  ~Thread();
  int Start();
  int Schedule(const FuncType& task);
  int Join();
  int Stop();

  bool IsJoined() { return joined_; }
  size_t TaskNum() { return bqueue_.size(); }
  pid_t GetThreadId() { return process_id_; }
  std::string Name() { return name_; }

 private:
  void Run();
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
} // namespace libbase
#endif  // GLUE_LIBBASE_THREAD_H_
