#ifndef LIBBASE_CURRENTTHREAD_H
#define LIBBASE_CURRENTTHREAD_H

#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <linux/unistd.h>
#include <pthread.h>

namespace libbase
{
namespace CurrentThread
{
	extern __thread pid_t cachedTid;
	pid_t gettid();
}	

pid_t tid();

}
#endif  // LIBBASE_CURRENTTHREAD_H


