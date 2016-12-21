#ifndef GLUE_LIBBASE_CONDVAR_H_
#define GLUE_LIBBASE_CONDVAR_H_

#include "libbase/mutexlock.h"
#include "libbase/noncopyable.h"

#include <pthread.h>

namespace glue_libbase {
class CondVar: private Noncopyable {
 public:
  explicit CondVar(MutexLock& mutex_lock): mutex_lock_(mutex_lock) {
    int ret = pthread_cond_init(&cond_var_, NULL);
    LOG_CHECK(!ret, "pthread_cond_init failed");
  }

  ~CondVar() {
    int ret = pthread_cond_destroy(&cond_var_);
    LOG_CHECK(!ret, "pthread_cond_destroy failed");
  }

  void Wait() {
    /* mu_ is a data member of MutexLock */
    int ret = pthread_cond_wait(&cond_var_, &mutex_lock_.mu_);
    LOG_CHECK(!ret, "pthread_cond_wait failed");
  }

  void NotifyOne() {
    int ret = pthread_cond_signal(&cond_var_);
    LOG_CHECK(!ret, "pthread_cond_signal failed");
  }

  void NotifyAll() {
    int ret = pthread_cond_broadcast(&cond_var_);
    LOG_CHECK(!ret, "pthread_cond_broadcast failed");
  }

 private:
  pthread_cond_t cond_var_;	
  MutexLock& mutex_lock_;
};
}

#endif		//GLUE_LIBASE_COND_H
