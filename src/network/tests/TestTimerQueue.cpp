#include <network/EpollThread.h>
#include <network/Epoll.h>
#include <network/Timer.h>
#include <network/TimerQueue.h>

#include <iostream>
#include <memory>

using namespace network;
using namespace network::poller;
using namespace network::timer;
using namespace libbase;

bool stopFlag = false;

void timeout()
{
	static int num = 0;
	num++;
	std::cout << "called timeout " << num << "time(s)" << std::endl;
	if (num >= 3)
		stopFlag = true;
}

int main()
{
	EpollThread epollThread;
	Epoll* epollPtr = epollThread.startThread();

	TimeStamp now;
	now.addInterval(5);

	Timer* mytimer = epollPtr->addTimer(now, timeout, 1);
	while(!stopFlag);
	std::cout << "stop timer" << std::endl;

	epollPtr->delTimer(mytimer);
	
	epollPtr->stop();
	epollThread.join();
	LOGTRACE();
	std::cout << "leaving..." << std::endl;
	return 0;
}

