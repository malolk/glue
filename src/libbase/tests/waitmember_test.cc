#include "libbase/waitmember.h"
#include "libbase/thread.h"
#include "libbase/logger.h"

#include <vector>

#include <stdlib.h>
#include <stdio.h>

typedef libbase::Thread Thread;
typedef libbase::Thread::FuncType Func;
typedef libbase::WaitMember Bar;

void ThreadFunc(int id, Bar* barrier) {
	barrier->Done();
	LOG_INFO("Member ID#%d Finished", id);
}

void TestCase(int num) {
  LOG_INFO("TEST WAITMEMBER: Member number %d", num);
  std::vector<Thread*> members; 
  int group_size = num;
  Bar barrier(group_size);
  for (int index = 0; index < group_size; ++index) {	
	members.push_back(new Thread());
    members.back()->Start();
    Func func = std::bind(ThreadFunc, index, &barrier);
    members.back()->Schedule(func);
  }

  for (auto &f : members) {
	f->Join();
    delete f;
  }
  LOG_INFO("TEST WAITMEMBER: Member number %d PASS", num);
}

int main() {
  TestCase(1);
  TestCase(5);
  TestCase(10);
  return 0;
}
