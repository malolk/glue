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

Timer* timer1 = NULL;
Epoll* epollPtr = NULL;
bool stopFlag = false;

void timeout1()
{
	std::cout << "called timeout1 " << std::endl;
}

void timeout2()
{
	std::cout << "called timeout2, then stop timer1" << std::endl;
	epollPtr->delTimer(timer1);
}

int main()
{
	EpollThread epollThread;
	epollPtr = epollThread.startThread();

	TimeStamp now;
	now.addInterval(1);
	
	timer1 = epollPtr->addTimer(now, timeout1, 2);
	
	now.addInterval(5);
	epollPtr->addTimer(now, timeout2, 0);

	while(!stopFlag);	
	epollPtr->stop();
	epollThread.join();
	LOGTRACE();
	std::cout << "leaving..." << std::endl;
	return 0;
}

