#ifndef LIBBASE_BLOCKINGQUEUE_H
#define LIBBASE_BLOCKINGQUEUE_H

#include <libbase/MutexLock.h>
#include <libbase/Cond.h>
#include <libbase/Noncopyable.h>

#include <dequeue>

namespace libbase
{
	// UNFINISHED: need to be noncopyable
template<typename T>
class BlockingQueue: private Noncopyable
{
public:
	BlockingQueue(): mu(), cond(mu)
	{ }

	void insert(const T& elem)
	{
		MutexLockGuard m(mu);
		queue.push_back(elem);
		cond.notifyOne();
	}
	T get()
	{
		MutexLockGuard m(mu);
		while(queue.empty() == true)
		{
			cond.wait();
		}
		return queue.pop_front();
	} 
	typename std::dequeue::size_type size()
	{
		MutexLockGuard m(mu);
		return queue.size();	
	}

private:
	mutable MutexLock mu;
	Cond cond;
	std::dequeue<T> queue;
}; 	
}
#endif //LIBBASE_BLOCKINGQUEUE_H
