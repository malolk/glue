#ifndef LIBBASE_THREADPOOL_H
#define LIBBASE_THREADPOOL_H

#include <libbase/MutexLock.h>
#include <libbase/Cond.h>
#include <libbase/Thread.h>
#include <libbase/WaitMember.h>
#include <libbase/Noncopyable.h>

#include <functional>
#include <deque>
#include <vector>
#include <iostream>
#include <memory>

namespace libbase
{

static const int DEFLTCNT = 4;
static const int DEFLTQUEUE = 10;
enum RunningState
{
	UNSTARTED = 0,
	RUNNING ,
	LEAVING,
	STOP
};

class ThreadPool: private Noncopyable
{
public:
	typedef std::function<void()> TaskType;
	//typedef std::function<void()> WokerType;
	//threadCount must greater than zero, queueSize at least be one
	explicit ThreadPool(size_t threadCount = DEFLTCNT, size_t sz = DEFLTQUEUE): 
	threadSize((threadCount > 0) ? threadCount : DEFLTCNT), 
	queueSize((sz > 0) ? sz : 1), 
	mu(), condOfAdd(mu), condOfGet(mu), 
	runningState(UNSTARTED)
	{}

	void start()
	{
		MutexLockGuard m(mu);
		runningState = RUNNING;
		threadStore.reserve(threadSize);
		for (std::vector<std::shared_ptr<Thread>>::iterator it = threadStore.begin();
		it != threadStore.end(); ++it)
		{
			threadStore.push_back(std::shared_ptr<Thread>(
			new Thread(std::bind(&libbase::ThreadPool::work, this))));
			(*it)->start();
		}
	} 

	void addOneTask(const TaskType& task)
	{
		MutexLockGuard m(mu);
		while(taskStore.size() == queueSize)
		{
			condOfAdd.wait();
		}
		taskStore.push_back(task);
		condOfGet.notifyOne();
	}

	~ThreadPool()
	{
		if (runningState != STOP)
			this->shutdown();
	}

	void shutdown();
	void shutdownNow();
private:
	void work();
	TaskType getOneTask();
	size_t threadSize;
	size_t queueSize;
	MutexLock mu;
	Cond condOfAdd;
	Cond condOfGet;
	std::deque<TaskType> taskStore;
	std::vector<std::shared_ptr<Thread>> threadStore;
	RunningState runningState;
};

ThreadPool::TaskType ThreadPool::getOneTask()
{
	MutexLockGuard m(mu);
	// maybe work takes too much time to be catched by notify
	if (runningState != RUNNING)
		return TaskType();
	while(taskStore.empty())
	{
		condOfGet.wait();
		if (runningState != RUNNING)
			return TaskType();
	}
	TaskType tmp = taskStore.front();
	taskStore.pop_front();
	condOfAdd.notifyOne();
	return tmp;
}

void ThreadPool::work()
{
	while(1)
	{
		TaskType task = getOneTask();
		if (task)
			task();
		else
			break;
	}
}

void ThreadPool::shutdown()
{
	WaitMember clearQueue(2);
	// assume all the valide tasks had been added into the back of queue before this dummy task
	addOneTask(std::bind(&libbase::WaitMember::done, &clearQueue));
	clearQueue.done();
	shutdownNow();
}

void ThreadPool::shutdownNow()
{
	{
		MutexLockGuard m(mu);
		runningState = LEAVING;
		condOfGet.notifyAll();
	}

	for (std::vector<std::shared_ptr<Thread>>::iterator it = threadStore.begin();
		it != threadStore.end(); ++it)
	{
		(*it)->join();
	}
	
	MutexLockGuard m(mu);
	runningState = STOP;
}

}

#endif //LIBBASE_THREADPOOL_H
