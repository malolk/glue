/*
#include <libbase/thread.h>
*/

#include "thread.h"

namespace glue_libbase {
namespace glue_libbase_thread {
  __thread pid_t cached_tid = 0;

  pid_t GetTid() {
    return static_cast<pid_t>(::syscall(SYS_gettid));
  }
}

pid_t tid() {
  if (0 == glue_libbase_thread::cached_tid) {
    glue_libbase_thread::cached_tid = glue_libbase_thread::GetTid();
  }
  return glue_libbase_thread::cached_tid;
}

/* Fork safe */
void AfterFork() {
  glue_libbase_thread::cached_tid = 0;
  tid();
}

class ThreadIdInitializer {
 public:
  ThreadIdInitializer() {
    tid();
    pthread_atfork(NULL, NULL, &AfterFork);
  }
};

/* Initialize the current thread_id from the startup of the current thread */
ThreadIdInitializer init_thread_id;
} // namespace glue_libbase
