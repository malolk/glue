#include "network/epoll.h"
#include "network/eventloop.h"
#include "network/timer.h"

#include <unistd.h>

void Repeated() {
  LOG_INFO("I repeated myself.");
}

void NonRepeated() {
  LOG_INFO("One shot, I'm not repeated. ");
}

int main() {
  network::Timer timer_oneshot(NonRepeated, 5); /* After 5 seconds. */
  network::Timer timer_repeated(Repeated, 0, 2); /* Every 2 seconds. */
  network::EventLoop eventloop;
  eventloop.Start();
  network::Epoll* epoll_ptr = eventloop.EpollPtr();
  epoll_ptr->RunTimer(NULL, timer_oneshot);
  network::TimerQueue::TimerIdType id;
  epoll_ptr->RunTimer(&id, timer_repeated);
  sleep(6);
  epoll_ptr->CancelTimer(&id);
  sleep(2);
  eventloop.Stop();
  eventloop.Join();
  sleep(1);
}
