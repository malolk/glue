#ifndef LIBBASE_BOUNDEDBLOCKINGQUEUE_H
#define LIBBASE_BOUNDEDBLOCKINGQUEUE_H

#include <libbase/MutexLock.h>
#include <libbase/Cond.h>
#include <libbase/Noncopyable.h>

#include <deque>
#include <iostream>

namespace libbase 
{
	// UNFINISHED: need to be noncopyable
template<typename T>
class BoundedBlockingQueue: private Noncopyable
{
public:
	explicit BoundedBlockingQueue(size_t cap = 10): 
		mu(), 
		condOfInsert(mu), 
		condOfGet(mu), 
		capacity(cap),
		enableGet(true), 
		enableInsert(true)
	{ }

	int insert(const T& elem)
	{
		MutexLockGuard m(mu);
		while(queueInst.size() == capacity)
		{
			condOfInsert.wait();
			if (!enableInsert)
				return -1;
		}
		queueInst.push_back(elem);
		condOfGet.notifyOne();

		return 0;
	}

	T get()
	{
		MutexLockGuard m(mu);
		while(queueInst.empty())
		{
			condOfGet.wait();
			if (!enableGet)
			{
				return T();
			}
		}
		T tmp = queueInst.front();
		queueInst.pop_front();
		condOfInsert.notifyOne();
		return tmp;
	}
	 
	typename std::deque<T>::size_type size()
	{
		MutexLockGuard m(mu);
		return queueInst.size();	
	}
	
	void closeInsertion()
	{
		MutexLockGuard m(mu);
		enableInsert = false;
		condOfInsert.notifyAll();	
	}

	void wakeupGet()
	{
		MutexLockGuard m(mu);
		enableGet = false;
		condOfGet.notifyAll();
	}
private:
	mutable MutexLock mu;
	Cond condOfInsert;    //for multiple inserters
	Cond condOfGet;		  //for multiple getters
	std::deque<T> queueInst;
	size_t capacity;
	bool enableGet;
	bool enableInsert;
}; 	
}
#endif //LIBBASE_BOUNDEDBLOCKINGQUEUE_H

