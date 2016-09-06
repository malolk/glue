#ifndef LIBBASE_THREAD_H
#define LIBBASE_THREAD_H

#include <libbase/Noncopyable.h>
#include <libbase/Cond.h>
#include <libbase/MutexLock.h>
#include <libbase/Debug.h>

#include <functional>
#include <pthread.h>

namespace libbase
{


void* runFunc(void *f);

class Thread: private Noncopyable
{
public:
	typedef std::function<void()> FuncType;
	Thread(const FuncType& funcIn): func(funcIn), threadId(0), joined(false), started(false)
	{}

	void start()
	{
		int ret = pthread_create(&threadId, NULL, &runFunc, static_cast<void *>(&func));
		CHECKX(ret == 0, "pthread_create failed");
		started = true;
	}

	int join()
	{
		joined = true;
		int ret = pthread_join(threadId, NULL);
		CHECKX(ret == 0, "pthread_join failed");
		return ret;
	}
	bool isStarted() { return started; }
	~Thread()
	{
		if (!joined)
		{
			int ret = pthread_detach(threadId);
			CHECKX(ret == 0, "pthread_detach failed");
		}
	}
private:
	FuncType func;
	pthread_t threadId;
	bool	joined;
	bool	started;
};
	
void* runFunc(void *f)
{
	// add some features here in future
	(*static_cast<Thread::FuncType*>(f))();

	return static_cast<void*>(0);
}

}

#endif  //LIBBASE_THREAD_H

