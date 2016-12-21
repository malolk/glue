#include "../thread.h"

#include <unistd.h>

#include <iostream>
#include <functional>

using namespace std;

void task1() {
  cout << "task: Current thread id: " << glue_libbase::ThreadId() << endl;
}

void task2(int sec) {
  cout << "task1: Current thread id: " << glue_libbase::ThreadId() << endl;
  sleep(sec);
}


int main() {
  cout << "Process ID: " << glue_libbase::ThreadId() << endl;
  std::function<void()> func = std::bind(task2, 2), func1 = task1, func2 = task1; 
  std::function<void()> func3 = std::bind(task2, 3);

  glue_libbase::Thread thread1, thread2;
  thread1.Start();
  thread2.Start();
 
  thread1.Schedule(func); 
  thread1.Schedule(func1); 
  thread1.Schedule(func2); 
  thread1.Stop();
  thread2.Schedule(func3); 
  
//  thread1.Join();
  thread2.Join();

  return 0;
}

