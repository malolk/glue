#include "libbase/bounded_blocking_queue.h"
#include "libbase/thread.h"
#include "libbase/logger.h"
#include "libbase/waitmember.h"

#include <vector>
#include <string>
#include <functional>
#include <iostream>

#include <unistd.h>

typedef libbase::BoundedBlockingQueue<int> QueueType;
typedef std::function<void()> FuncType;
typedef libbase::WaitMember WType;

void producer(QueueType* q, int start, WType* barrier) { 
  std::string ret;
  for (int i = start; i < start * 2; ++i) {
    q->Insert(i);
	ret += std::to_string(i);
    ret += " ";
  }
  barrier->Done();
  LOG_INFO(ret.c_str());
}

void consumer(QueueType* q) {
  std::string ret;
  while (true) {
    int item = q->Get();
    if (item == 0) break;
	ret += std::to_string(item);
    ret += " ";
  }
  LOG_INFO(ret.c_str());
}

void TestCase(int cap) {
  LOG_INFO("\n=========TestCase with cap = %d =========\n", cap);
  QueueType q(cap);
  WType barrier(4);

  FuncType producer_func_1 = std::bind(producer, &q, 1, &barrier);
  FuncType producer_func_5 = std::bind(producer, &q, 5, &barrier);
  FuncType producer_func_10 = std::bind(producer, &q, 10, &barrier);
  FuncType consumer_func = std::bind(consumer, &q);

  libbase::Thread producer_1, producer_5, producer_10;
  libbase::Thread consumer_1, consumer_2;

  producer_1.Start();
  producer_5.Start();
  producer_10.Start();

  consumer_1.Start();
  consumer_2.Start();

  consumer_1.Schedule(consumer_func);
  consumer_2.Schedule(consumer_func);
  producer_1.Schedule(producer_func_1);
  producer_5.Schedule(producer_func_5);
  producer_10.Schedule(producer_func_10);
  
  barrier.Done();

  q.Insert(0);
  q.Insert(0);
  producer_1.Join();
  producer_5.Join();
  producer_10.Join();
  
  consumer_1.Join();
  consumer_2.Join();
  LOG_INFO("\n=================[PASS]===================\n", cap);
}

int main(int argc, char* argv[]) {
  TestCase(3);
  std::cout << "Case " << 3 << "PASS" << std::endl;
  TestCase(10); 
  std::cout << "Case " << 10 << "PASS" << std::endl;
  TestCase(20); 
  std::cout << "Case " << 20 << "PASS" << std::endl;
  return 0;
}

