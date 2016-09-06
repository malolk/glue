#include <libbase/WaitMember.h>
#include <libbase/Thread.h>

#include <iostream>
#include <vector>

#include <stdlib.h>
#include <stdio.h>

typedef libbase::Thread Thread;
typedef libbase::Thread::FuncType Func;
typedef libbase::WaitMember Bar;

void threadFunc(int id, Bar* barrier)
{
	barrier->done();
	printf("thread #%d\n", id);

}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
	std::cerr << "usage: ./a.out NUM" << std::endl;
		return 0;
	}
	
	std::vector<Thread*> members; 
	int grpSize = atoi(argv[1]);
	Bar barrier(grpSize);
	for (int index = 0; index < grpSize; ++index) 
	{	
		Func func = std::bind(threadFunc, index, &barrier);
		members.push_back(new Thread(std::move(func)));
	}

	for (auto &f : members)
	{
		f->start();
	}
	for (auto &f : members)
	{
//		printf("yeah\n");
		f->join();
	}
	return 0;
}

