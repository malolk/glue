#ifndef GLUE_LIBBASE_CONDVAR_H_
#define GLUE_LIBBASE_CONDVAR_H_

#include "libbase/mutexlock.h"
#include "libbase/noncopyable.h"
#include "libbase/sys_check.h"

#include <string.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <errno.h>
#include <sys/time.h>

namespace libbase {
class CondVar: private Noncopyable {
 public:
  explicit CondVar(MutexLock& mutex_lock): mutex_lock_(mutex_lock) {
    int ret = pthread_cond_init(&cond_var_, NULL);
    SYS_CHECK(ret);
  }

  ~CondVar() {
    int ret = pthread_cond_destroy(&cond_var_);
    SYS_CHECK(ret);
  }

  void Wait() {
    /* mu_ is a data member of MutexLock */
    int ret = pthread_cond_wait(&cond_var_, &mutex_lock_.mu_);
    SYS_CHECK(ret);
  }
  
  void WaitInSeconds(int secs) {
    assert(secs >= 0); 
    struct timeval now;
    ::gettimeofday(&now, NULL);
    struct timespec wait_time;
    wait_time.tv_sec = secs + now.tv_sec;
    wait_time.tv_nsec = now.tv_usec * 1000UL;
    int ret = pthread_cond_timedwait(&cond_var_, &mutex_lock_.mu_, &wait_time);
    if (ret && ret != ETIMEDOUT) SYS_CHECK(ret);
  }

  void NotifyOne() {
    int ret = pthread_cond_signal(&cond_var_);
    SYS_CHECK(ret);
  }

  void NotifyAll() {
    int ret = pthread_cond_broadcast(&cond_var_);
    SYS_CHECK(ret);
  }

 private:
  MutexLock& mutex_lock_;
  pthread_cond_t cond_var_;	
};
}

#endif		//GLUE_LIBASE_COND_H
