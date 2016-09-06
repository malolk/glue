#ifndef LIBBASE_BOUNDEDPRIORITYQUEUE_H
#define LIBBASE_BOUNDEDPRIORITYQUEUE_H

#include <libbase/MutexLock.h>
#include <libbase/Cond.h>
#include <libbase/Noncopyable.h>

#include <queue>
#include <functional>

namespace libbase
{
	//UNFINISHED: need to be noncopyable
template<typename T>
class BoundedPriorityQueue: private Noncopyable
{
public:
	typedef std::function<bool(const T&, const T&)> comp; 
	explicit BoundedPriorityQueue(comp comparator, size_t cap = 10):
		mu(), 
		cond(mu), 
		heap(comparator),
		capacity(cap)
	{ }
	
	void insert(const T& elem)
	{
		MutexLockGuard m(mu);
		while(heap.size() == capacity)
		{
			cond.wait();
		}
		heap.push(elem);
		cond.notifyOne();
	}

	T get()
	{
		MutexLockGuard m(mu);
		while(heap.size() == 0)
		{
			cond.wait();
		}
		T tmp = heap.top();
		heap.pop();
		cond.notifyOne();
		return std::move(tmp);
	}

	typename std::priority_queue<T>::size_type
	size()
	{
		MutexLockGuard m(mu);
		return heap.size();
	}

private:
	mutable MutexLock mu;
	Cond cond;
	std::priority_queue<T, std::vector<T>, comp> heap;
	size_t capacity; 		
};
	
}
#endif //LIBBASE_BOUNDEDPRIORITYQUEUE_H
