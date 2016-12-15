#ifndef GLUE_LIBBASE_HEAP_H_
#define GLUE_LIBBASE_HEAP_H_

#include "noncopyable.h"
#include "logger.h"

#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>

/* A n-ary heap implementation. And it's not thread safe */
namespace glue_libbase {
template <class T>
bool DefaultComparator(const T& lhs, const T& rhs);

template<class T>
class Heap: private Noncopyable {
 public:
  typedef std::function<bool(const T&, const T&)> CompType;	
  explicit Heap(bool is_top_min = true, int ary_num = 2, 
                const CompType& cmp = DefaultComparator<T>) 
    : is_top_min_(is_top_min), ary_num_(ary_num), size_(0), array_(INITCAP), cmp_(cmp) {
    LOG_CHECK(ary_num_ >= 2, "heap ary num should be at least 2");
  } 

  ~Heap() {
  }
  T TopAndPop();
  T Top();
  void Insert(const T& elem);
  bool Empty() { return (size_ == 0); }
  size_t Size() { return size_; }

private:
  bool Compare(const T&, const T&);
  void Swim(int );
  void Sink(int );

  bool is_top_min_;
  int ary_num_;
  int size_;
  std::vector<T> array_;
  CompType cmp_;
  static const int INITCAP = 10;
};

template <class T>
inline bool DefaultComparator(const T& lhs, const T& rhs) {
  return (lhs < rhs);
}

template <class T>
inline bool Heap<T>::Compare(const T& lhs, const T& rhs) {
  bool ret = cmp_(lhs, rhs);
  return (is_top_min_ ? ret : !ret);	
}

template <class T>
void Heap<T>::Swim(int loc) {
  int parent = (loc - 1)/ary_num_;
  int child = loc;
  T tmp = array_[child];
  while (parent >= 0 && child > 0 && Compare(tmp, array_[parent])) {
    array_[child] = array_[parent];
	child = parent;
	parent = (child - 1)/ary_num_;
  }
  array_[child] = tmp;
}

template <class T>
void Heap<T>::Sink(int loc) {
  int parent = loc;
  int child = loc * ary_num_ + 1;
  T tmp = array_[parent];
  while (child < size_) {
    int right_most = (child + ary_num_) < size_ ? (child + ary_num_) : size_;
	for (int num = child + 1; num < right_most; ++num)
	  if (!Compare(array_[child], array_[num])) {
	    child = num;
      }
	  if (Compare(tmp, array_[child])) {
          break;
      }
	  array_[parent] = array_[child];
	  parent = child;
	  child = parent * ary_num_ + 1;
	}
  array_[parent] = tmp;
}

template <class T>
void Heap<T>::Insert(const T& elem) {
  if (static_cast<unsigned int>(size_) == array_.size()) {
    array_.resize(2 * size_, T());
  }
  array_[size_++] = elem;
  Swim(size_ - 1);
}

template <class T>
T Heap<T>::TopAndPop() {
  LOG_CHECK(size_ > 0, "Heap is empty");
  T ret = array_[0];
  array_[0] = array_[--size_];
  Sink(0);
  return ret;
}

template <class T>
T Heap<T>::Top() {
  LOG_CHECK(size_ > 0, "Heap is empty");
  return array_[0];
}
} // namespace glue_libbase
#endif // GLUE_LIBBASE_HEAP_H_
