#include <libbase/CurrentThread.h>

using namespace libbase;
using namespace libbase::CurrentThread;

namespace libbase
{
namespace CurrentThread
{
	__thread pid_t cachedTid = 0;
	pid_t gettid()
	{
		return static_cast<pid_t>(::syscall(SYS_gettid));
	}
}
pid_t tid()
{
	if (CurrentThread::cachedTid == 0)
		CurrentThread::cachedTid = CurrentThread::gettid();
	return CurrentThread::cachedTid;
}
}

// fork safe
void afterFork()
{
    CurrentThread::cachedTid = 0;
    tid();
}

class ThreadIdInitializer
{
public:
    ThreadIdInitializer()
    {   
        tid();
        pthread_atfork(NULL, NULL, &afterFork);
    }   
};

// initiate threadId from start point
ThreadIdInitializer initThreadId;

