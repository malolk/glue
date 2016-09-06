#include <libbase/BoundedBlockingQueue.h>
#include <libbase/Thread.h>

#include <iostream>
#include <vector>

#include <stdlib.h>
#include <stdio.h>

typedef libbase::Thread Thread;
typedef libbase::Thread::FuncType Func;
typedef libbase::BoundedBlockingQueue<int> BQueue;

void threadProducer(BQueue* q)
{
	std::vector<int> vec{3, 2, 8, 5, 23, 12, 234, 34 ,23, 434};
	for (int elem : vec)
	{
		q->insert(elem);
	}
}

void threadConsumer(BQueue* q)
{
	int cnt = 10;
	while(cnt--)
	{
		std::cout << " " << q->get();
	}
	std::cout << "\n";
}

int main(int argc, char* argv[])
{
	BQueue q(10);	
	Func func_producer = std::bind(threadProducer, &q);
	Thread th_pro(func_producer);
	Func func_consumer = std::bind(threadConsumer, &q);
	Thread th_con(func_consumer);
	
	th_pro.start();
	th_con.start();
	
	th_pro.join();
	th_con.join();

	return 0;
}

