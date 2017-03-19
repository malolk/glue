#ifndef GLUE_LIBBASE_MUTEXLOCK_H_
#define GLUE_LIBBASE_MUTEXLOCK_H_

#include "libbase/noncopyable.h"
#include "libbase/sys_check.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

namespace libbase {
class MutexLock: private Noncopyable {
 public:
  MutexLock() {
    int ret = pthread_mutex_init(&mu_, NULL);
    SYS_CHECK(ret);
  }

  void Lock() {
	int ret = pthread_mutex_lock(&mu_);
    SYS_CHECK(ret);
  }

  void Unlock() {
    int ret = pthread_mutex_unlock(&mu_);
    SYS_CHECK(ret);
  }

  ~MutexLock() {
    int ret = pthread_mutex_destroy(&mu_);	
    SYS_CHECK(ret);
  }

 private:
  /* cond varible need the pthread_mutex_t */
  friend class CondVar; 
  pthread_mutex_t mu_;		
};

class MutexLockGuard : private Noncopyable {
 public:
  explicit MutexLockGuard(MutexLock& m): mutex_lock_(m) {
    mutex_lock_.Lock();
  }
	
  ~MutexLockGuard() {
    mutex_lock_.Unlock();
  }

 private:
  MutexLock& mutex_lock_; 
};
}

#endif		// #GLUE_LIBBASE_MUTEXLOCK_H_
