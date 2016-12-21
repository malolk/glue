#include "../eventloop.h"
#include "../timer.h"

#include <unistd.h>

void Repeated() {
  LOG_INFO("I repeated myself.");
}

void NonRepeated() {
  LOG_INFO("One shot, I'm not repeated. ");
}

int main() {
  glue_network::Timer timer_oneshot(NonRepeated, glue_libbase::TimeUtil::NowMicros() + 5000000LL); /* After 5 seconds. */
  glue_network::Timer timer_repeated(Repeated, 0, 2000000LL); /* Every 2 seconds. */
  glue_network::EventLoop event_loop;
  event_loop.Start();
  event_loop.RunTimer(NULL, timer_oneshot);
  glue_network::TimerQueue::TimerIdType id;
  event_loop.RunTimer(&id, timer_repeated);
  sleep(6);
  event_loop.CancelTimer(&id);
  sleep(2);
  event_loop.Stop();
  event_loop.Join();
  sleep(1);
}
