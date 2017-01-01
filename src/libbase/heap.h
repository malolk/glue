#ifndef GLUE_LIBBASE_HEAP_H_
#define GLUE_LIBBASE_HEAP_H_

#include "libbase/noncopyable.h"
#include "libbase/loggerutil.h"

#include <vector>
#include <functional>
#include <algorithm>
#include <iterator>
#include <utility>
#include <memory>

/* A n-ary heap implementation. And it's not thread safe */
namespace glue_libbase {
template <class T>
bool DefaultComparator(const T& lhs, const T& rhs);

template <class T>
class Heap;

template <class T> 
class Element {
  Element(int index, const T& data) : index_(index), data_(data) {
    LOG_CHECK(index_ >= 0, "");
  }
 private:
  friend class Heap<T>;
  int index_;
  T data_;
};

template<class T>
class Heap: private Noncopyable {
 public:
  typedef std::function<bool(const T&, const T&)> CompType;	
  explicit Heap(int ary_num = 2, const CompType& cmp = DefaultComparator<T>) 
    : ary_num_(ary_num), size_(0), array_(INITCAP), cmp_(cmp) {
    LOG_CHECK(ary_num_ >= 2, "heap ary num should be at least 2");
  } 

  ~Heap() {
  }
  T TopAndPop();
  void Pop();
  T& Top();
  /* Use weak_ptr to detect whether the element is still exit in the array_. */
  std::weak_ptr<Element<T>> Insert(const T& data);
  std::weak_ptr<Element<T>> GetTopId() const;
  int Delete(std::weak_ptr<Element<T>>& id);
  int Update(std::weak_ptr<Element<T>>& id, const T& data);
  int Get(const std::weak_ptr<Element<T>>& id, T& result) const {
    std::shared_ptr<Element<T>> observer(id.lock());
    if (observer) {
      result = observer->data_;
      return 1; /* The element is still in the heap. */
    } else {
      return 0;
    }
  }

  bool Empty() const { 
    return (size_ == 0); 
  }

  size_t Size() const { 
    return size_; 
  }

  void Swim(int );
  void Sink(int );

 private:
  bool Compare(const std::shared_ptr<Element<T>>&, const std::shared_ptr<Element<T>>&);

  int ary_num_;
  int size_;
  std::vector<std::shared_ptr<Element<T>>> array_;
  CompType cmp_;
  static const int INITCAP = 10;
};

template <class T>
inline bool DefaultComparator(const T& lhs, const T& rhs) {
  return (lhs < rhs);
}

template <class T>
inline bool Heap<T>::Compare(const std::shared_ptr<Element<T>>& lhs, const std::shared_ptr<Element<T>>& rhs) {
  return ((!lhs) ? true : cmp_(lhs->data_, rhs->data_));
}

template <class T>
void Heap<T>::Swim(int index) {
  LOG_CHECK(index >= 0 && index < size_, "");
  if (index == 0) {
    return;
  }
  int parent_index = (index - 1)/ary_num_;
  int child_index = index;
  std::shared_ptr<Element<T>> tmp = array_[child_index];
  while (parent_index >= 0 && child_index > 0 && Compare(tmp, array_[parent_index])) {
    array_[child_index] = array_[parent_index];
    array_[child_index]->index_ = child_index;
	child_index = parent_index;
	parent_index = (child_index - 1)/ary_num_;
  }
  array_[child_index] = tmp;
  if (tmp) { /* Maybe the tmp has been deleted. See Delete() for detail. */
    tmp->index_ = child_index;
  }
}

template <class T>
void Heap<T>::Sink(int index) {
  LOG_CHECK(index >= 0 && index < size_, "");
  int parent_index = index;
  int child_index = index * ary_num_ + 1;
  std::shared_ptr<Element<T>> tmp = array_[parent_index];
  while (child_index < size_) {
    int right_most = (child_index + ary_num_) < size_ ? (child_index + ary_num_) : size_;
	for (int num = child_index + 1; num < right_most; ++num)
	  if (!Compare(array_[child_index], array_[num])) {
	    child_index = num;
      }
	  if (Compare(tmp, array_[child_index])) {
          break;
      }
	  array_[parent_index] = array_[child_index];
      array_[parent_index]->index_ = parent_index;
	  parent_index = child_index;
	  child_index = parent_index * ary_num_ + 1;
	}
  array_[parent_index] = tmp;
  tmp->index_ = parent_index;
}

template <class T>
std::weak_ptr<Element<T>> Heap<T>::GetTopId() const {
  LOG_CHECK(size_ > 0, "Heap is empty");
  return array_[0];    
}

template <class T>
std::weak_ptr<Element<T>> Heap<T>::Insert(const T& elem) {
  if (static_cast<unsigned int>(size_) == array_.size()) {
    array_.resize(2 * size_);
  }
  array_[size_] = std::shared_ptr<Element<T>>(new Element<T>(size_, elem));
  std::shared_ptr<Element<T>> ret = array_[size_];
  ++size_;
  Swim(size_ - 1);
  return ret;
}

template <class T>
void Heap<T>::Pop() {
  LOG_CHECK(size_ > 0, "Heap is empty");
  array_[0] = array_[size_ - 1];
  array_[size_ - 1].reset();
  if (--size_ > 0 ) {
    Sink(0);
  }
}

template <class T>
T Heap<T>::TopAndPop() {
  LOG_CHECK(size_ > 0, "Heap is empty");
  T ret = array_[0]->data_;
  array_[0] = array_[size_ - 1];
  array_[size_ - 1].reset();
  if (--size_ > 0 ) {
    Sink(0);
  }
  return ret;
}

template <class T>
T& Heap<T>::Top() {
  LOG_CHECK(size_ > 0, "Heap is empty");
  return array_[0]->data_;
}

template <class T>
int Heap<T>::Delete(std::weak_ptr<Element<T>>& id) {
  std::shared_ptr<Element<T>> observer(id.lock());
  if (observer) {
    int index = observer->index_;
    /* Note: no object-binded shared_ptr could used as a Minum(for Min-Heap) element. */
    array_[index].reset();
    Swim(index);
    array_[0] = array_[size_ - 1];
    if (array_[size_ - 1]) { 
      array_[size_ - 1].reset();
    }
    if (--size_ > 0) {
      Sink(0);     
    }
    return 1;
  } else {
    return 0;
  }
}

template <class T>
int Heap<T>::Update(std::weak_ptr<Element<T>>& id, const T& data) {
  std::shared_ptr<Element<T>> observer(id.lock());
  if (observer) {
    int index = observer->index_;
    int parent_index = (index - 1)/ary_num_;
    array_[index]->data_ = data;
    if (Compare(array_[index], array_[parent_index])) {
      Swim(index);
    } else {
      Sink(index);
    }
    return 1;
  } else {
    return 0; /* The element has been deleted. */
  }
}
} // namespace glue_libbase
#endif // GLUE_LIBBASE_HEAP_H_
