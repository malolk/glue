#ifndef LIBBASE_MUTEXLOCK_H
#define LIBBASE_MUTEXLOCK_H

#include <libbase/Noncopyable.h>
#include <libbase/Debug.h>

#include <iostream>

#include <pthread.h>

namespace libbase 
{

class MutexLock: private Noncopyable
{
public:
	MutexLock()
	{
		int ret = pthread_mutex_init(&mu, NULL);
		CHECKX(ret == 0, "pthread_mutex_init failed");
	}

	void mutexLock()
	{
		int ret = pthread_mutex_lock(&mu);
		CHECKX(ret == 0, "pthread_mutex_lock failed");
	}

	void mutexUnlock()
	{
		int ret = pthread_mutex_unlock(&mu);
		CHECKX(ret == 0, "pthread_mutex_unlock failed");
	}

	~MutexLock()
	{
		int ret = pthread_mutex_destroy(&mu);	
		CHECKX(ret == 0, "pthread_mutex_destroy failed");
	}

	pthread_mutex_t* getInnerMutex()
	{
		return &mu;
	}
private:
	pthread_mutex_t mu;		
};

class MutexLockGuard : private Noncopyable
{
public:
	explicit MutexLockGuard(MutexLock& m): mu(m)
	{
		mu.mutexLock();
	}
	~MutexLockGuard()
	{
		mu.mutexUnlock();
	}
private:
	MutexLock& mu; 
};

}
#endif		// #LIBBASE_MUTEXLOCK_H
