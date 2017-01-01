#ifndef GLUE_LIBBASE_CONDVAR_H_
#define GLUE_LIBBASE_CONDVAR_H_

#include "libbase/mutexlock.h"
#include "libbase/noncopyable.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <sys/time.h>

namespace glue_libbase {
class CondVar: private Noncopyable {
 public:
  explicit CondVar(MutexLock& mutex_lock): mutex_lock_(mutex_lock) {
    int ret = pthread_cond_init(&cond_var_, NULL);
    assert(ret == 0);
    (void)ret;
  }

  ~CondVar() {
    int ret = pthread_cond_destroy(&cond_var_);
    assert(ret == 0);
    (void)ret;
  }

  void Wait() {
    /* mu_ is a data member of MutexLock */
    int ret = pthread_cond_wait(&cond_var_, &mutex_lock_.mu_);
    assert(ret == 0);
    (void)ret;
  }
  
  void WaitInSeconds(int secs) {
    assert(secs >= 0); 
    struct timeval now;
    ::gettimeofday(&now, NULL);
    struct timespec wait_time;
    wait_time.tv_sec = secs + now.tv_sec;
    wait_time.tv_nsec = now.tv_usec * 1000UL;
    int ret = pthread_cond_timedwait(&cond_var_, &mutex_lock_.mu_, &wait_time);
    assert(ret == 0);
    (void)ret;
  }

  void NotifyOne() {
    int ret = pthread_cond_signal(&cond_var_);
    assert(ret == 0);
    (void)ret;
  }

  void NotifyAll() {
    int ret = pthread_cond_broadcast(&cond_var_);
    assert(ret == 0);
    (void)ret;
  }

 private:
  MutexLock& mutex_lock_;
  pthread_cond_t cond_var_;	
};
}

#endif		//GLUE_LIBASE_COND_H
