#ifndef GLUE_LIBBASE_BLOCKINGQUEUE_H_
#define GLUE_LIBBASE_BLOCKINGQUEUE_H_

#include "libbase/mutexlock.h"
#include "libbase/condvar.h"
#include "libbase/noncopyable.h"

#include <deque>

namespace libbase {
template<typename T>
class BlockingQueue: private Noncopyable {
 public:
  BlockingQueue(): mu_(), condvar_(mu_){
  }
  
  void Insert(const T& elem) {
    MutexLockGuard m(mu_);
	queue_.push_back(elem);
	condvar_.NotifyOne();
  }

  T Get() {
    MutexLockGuard m(mu_);
    while (queue_.empty()) {
      condvar_.Wait();
	}
    T ret = queue_.front();
    queue_.pop_front();
    return ret;
  }

  typename std::deque<T>::size_type size() {
    MutexLockGuard m(mu_);
    return queue_.size();	
  }

 protected:  
  MutexLock mu_;
  CondVar condvar_;
  std::deque<T> queue_;
}; 	
}

#endif // GLUE_LIBBASE_BLOCKINGQUEUE_H_
