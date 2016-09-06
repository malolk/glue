#include <libbase/ThreadPool.h>
#include <libbase/MutexLock.h>

#include <iostream>
#include <functional>

#include <stdlib.h>
#include <stdio.h>

static const long LOOP = 1000000;
static const long ANS = (1 + LOOP)*LOOP/2;
static long global_sum = 0;
libbase::MutexLock mu;
void do_sum(long n)
{
	libbase::MutexLockGuard m(mu);
	global_sum += n;
}

void testCase(int threadSize, int queueSize)
{
	global_sum = 0;
	libbase::ThreadPool threadPool(threadSize, queueSize);
	threadPool.start();

	std::cout << "start adding tasks\n";	
	for (int index = 1; index <= LOOP; ++index)
	{
		int adder = index;
		threadPool.addOneTask(std::bind(do_sum, adder));
	}
	std::cout << "wait to finish\n";	

	threadPool.shutdown();
	if (global_sum == ANS)
		std::cout << "PASS TEST CASE " << "ThreadNum: " << threadSize << "QueueSize: " << queueSize << "\n";
	else 
		std::cout << "FAILED TEST CASE ANS: " << global_sum << "ThreadNum: " << threadSize << "QueueSize: " << queueSize << "\n";
}

int main(void)
{
	int threadSize = 4;
	testCase(threadSize, 0);    // coner case
	testCase(threadSize, 1);
	testCase(threadSize, 15);
	
	return 0;
}
