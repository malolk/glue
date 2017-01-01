#ifndef GLUE_LIBBASE_MUTEXLOCK_H_
#define GLUE_LIBBASE_MUTEXLOCK_H_

#include "libbase/noncopyable.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

namespace glue_libbase {
class MutexLock: private Noncopyable {
 public:
  MutexLock() {
    int ret = pthread_mutex_init(&mu_, NULL);
    if (ret != 0) {
      fprintf(stderr, "pthread_mutex_init failed");
      abort();
    }
  }

  void Lock() {
	int ret = pthread_mutex_lock(&mu_);
    if (ret != 0) {
      fprintf(stderr, "pthread_mutex_lock failed");
      abort();
    }
  }

  void Unlock() {
    int ret = pthread_mutex_unlock(&mu_);
    if (ret != 0) {
      fprintf(stderr, "pthread_mutex_unlock failed");
      abort();
    }
  }

  ~MutexLock() {
    int ret = pthread_mutex_destroy(&mu_);	
    if (ret != 0) {
      fprintf(stderr, "pthread_mutex_destroy failed");
      abort();
    }
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
