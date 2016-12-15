#ifndef GLUE_LIBBASE_MUTEXLOCK_H_
#define GLUE_LIBBASE_MUTEXLOCK_H_

/*
#include <libbase/noncopyable.h>
#include <libbase/debug.h>
*/

#include "noncopyable.h"
#include "logger.h"

#include <iostream>

#include <pthread.h>

namespace glue_libbase {
class MutexLock: private Noncopyable {
 public:
  MutexLock() {
    int ret = pthread_mutex_init(&mu_, NULL);
	LOG_CHECK(ret == 0, "pthread_mutex_init failed");
  }

  void Lock() {
	int ret = pthread_mutex_lock(&mu_);
	LOG_CHECK(ret == 0, "pthread_mutex_lock failed");
  }

  void Unlock() {
    int ret = pthread_mutex_unlock(&mu_);
    LOG_CHECK(ret == 0, "pthread_mutex_unlock failed");
  }

  ~MutexLock() {
    int ret = pthread_mutex_destroy(&mu_);	
    LOG_CHECK(ret == 0, "pthread_mutex_destroy failed");
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
