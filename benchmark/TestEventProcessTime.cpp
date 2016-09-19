#include <network/EventChannel.h>
#include <network/Epoll.h>
//#include <libbase/MutexLock.h>
//#include <libbase/Cond.h>
#include <libbase/TimeStamp.h>
#include <libbase/Debug.h>

#include <vector>
#include <functional>
#include <iostream>

#include <sys/socket.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <unistd.h>

using namespace network;
using namespace network::poller;
using namespace libbase;

ssize_t recv_cnt;
int writes, fired;
int num_pipes, num_actives, num_writes;

std::vector<int> pipes;
std::vector<EventChannel*> chans;
Epoll* epollPtr = NULL;

void ReadCallback(int fd, int idx)
{
	char ch;
	recv_cnt += ::recv(fd, &ch, 1, 0);	
	if(writes > 0)
	{
		int widx = idx + 1;
		if(widx >= num_pipes)
			widx -= num_pipes;
		::send(pipes[widx*2 + 1], "m", 1, 0);
		--writes;	
		++fired;
	}
	if(fired == recv_cnt)
	{
		epollPtr->stop();
//		cond.notifyOne();	
	}
}

void RunOnce()
{
	TimeStamp before;
	for(int i = 0; i < num_pipes; i++)
	{
		chans[i]->setCallbackOnRead(std::bind(ReadCallback, chans[i]->iofd(), i));
		chans[i]->addIntoEpoll();
		chans[i]->enableReading();	
	}
	
	int space = num_pipes / num_actives;
	space *= 2;

	for(int i = 0; i < num_actives; i++)
	{
		::send(pipes[i * space + 1], "m", 1, 0);	
	}
	fired = num_actives;
	recv_cnt = 0;
	writes = num_writes;

	TimeStamp start;

	epollPtr->runEpoll();

	TimeStamp end;
	
	std::cout << start.diffInMicroSecond(before) << std::endl; 
	std::cout << end.diffInMicroSecond(start) << std::endl; 
}

int main(int argc, char* argv[])
{
	num_pipes = 100;
	num_actives = 1;
	num_writes = 100;
	
	int test_cnt = 25;

	int opt;
	while((opt = getopt(argc, argv, "n:a:w:t:")) != -1)
	{
		switch(opt)
		{
			case 'n' : num_pipes = atoi(optarg);
					   break;
			case 'a' : num_actives = atoi(optarg);
					   break;
			case 'w' : num_writes = atoi(optarg);
					   break;
			case 't' : test_cnt = atoi(optarg);
					   break;
			default: std::cerr << "Illegal argument: " << opt << std::endl;
					 return 1;
		}			
	}

	struct rlimit rl;
	rl.rlim_cur = rl.rlim_max = num_pipes * 2 + 50;
	if(::setrlimit(RLIMIT_NOFILE, &rl) == -1)
	{
		std::cerr << "setrlimit error: " << strerror(errno) << std::endl;
		return 1;	
	}

	pipes.resize(2 * num_pipes);
	for(int i = 0; i < num_pipes; ++i)
	{
		if(::socketpair(AF_UNIX, SOCK_STREAM, 0, &pipes[2 * i]) == -1)
		{
			std::cerr << "socketpair error: " << strerror(errno) << std::endl;
			return 1;	
			
		}	
	}

	Epoll epoll;
	epoll.epollInitialize();
	epollPtr = &epoll;
	
	for(int i = 0; i < num_pipes; i++)
	{
		chans.push_back(new EventChannel(epollPtr, pipes[i * 2]));	
	}

	std::cout << "=========start=========\n";
	for(int i = 0; i < test_cnt; ++i)
	{
		RunOnce();
		std::cout << "writes=" << writes 
		<< " fired=" << fired 
		<< " received=" << recv_cnt << std::endl;			
	}
	std::cout << "==========end==========\n";

	for(int i = 0; i < num_pipes; ++i)
	{	
		epollPtr->delChannel(chans[i]);	
		delete chans[i];
	}
	return 0;
}





