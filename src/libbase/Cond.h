#ifndef LIBBASE_COND_H
#define LIBBASE_COND_H

#include <libbase/MutexLock.h>
#include <libbase/Noncopyable.h>
#include <libbase/Debug.h>

#include <pthread.h>

namespace libbase 
{

class Cond: private Noncopyable
{
public:
	explicit Cond(MutexLock& m): mu(m)
	{
		CHECKX((pthread_cond_init(&cond, NULL) == 0), "pthread_cond_init failed");
	}

	~Cond()
	{
		CHECKX((pthread_cond_destroy(&cond) == 0), "pthread_cond_destroy failed");
	}

	void wait()
	{
		CHECKX((pthread_cond_wait(&cond, mu.getInnerMutex()) == 0), "pthread_cond_wait failed");
	}

	void notifyOne()
	{
		CHECKX((pthread_cond_signal(&cond) == 0), "pthread_cond_signal failed");
	}

	void notifyAll()
	{
		CHECKX((pthread_cond_broadcast(&cond) == 0), "pthread_cond_broadcast failed");
	}

private:
	pthread_cond_t cond;	
	MutexLock& mu;
};

}
#endif		//LIBASE_COND_H
