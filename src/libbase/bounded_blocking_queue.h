#ifndef GLUE_LIBBASE_BOUNDEDBLOCKINGQUEUE_H_
#define GLUE_LIBBASE_BOUNDEDBLOCKINGQUEUE_H_

#include "libbase/noncopyable.h"
#include "libbase/mutexlock.h"
#include "libbase/condvar.h"
#include "libbase/loggerutil.h"

#include <deque>

namespace glue_libbase {
template<typename T>
class BoundedBlockingQueue: private Noncopyable {
 public:
  explicit BoundedBlockingQueue(size_t capacity) 
    : capacity_(capacity), mu_(), get_cv_(mu_), insert_cv_(mu_) {
    LOG_CHECK(capacity_ > 0, "capacity should be at least one");
  }

  void Insert(const T& elem) {
    MutexLockGuard m(mu_);
    while (queue_.size() >= capacity_) {
      insert_cv_.Wait();
    }
    queue_.push_back(elem);
    get_cv_.NotifyOne();
  }

  T Get() {
    MutexLockGuard m(mu_);
    while (queue_.empty()) {
      get_cv_.Wait();
    }
    T ret = queue_.front();
    queue_.pop_front();
    insert_cv_.NotifyOne();
    return ret;
  }

  typename std::deque<T>::size_type size() {
    MutexLockGuard m(mu_);
    return queue_.size();
  }

 private:
  const size_t capacity_;
  MutexLock mu_;
  CondVar get_cv_;
  CondVar insert_cv_;
  std::deque<T> queue_;
}; 	
} // namespace glue_libbase
#endif // GLUE_LIBBASE_BOUNDEDBLOCKINGQUEUE_H_

