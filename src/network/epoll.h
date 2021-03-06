#ifndef GLUE_NETWORK_EPOLL_H_
#define GLUE_NETWORK_EPOLL_H_

#include "network/socket.h"
#include "network/event_channel.h"
#include "network/timer.h"
#include "network/timer_queue.h"
#include "libbase/noncopyable.h"
#include "libbase/loggerutil.h"
#include "libbase/mutexlock.h"
#include "libbase/timeutil.h"
#include "libbase/thread.h"

#include <vector>
#include <unordered_set>
#include <memory>
#include <functional>
#include <atomic>

#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/eventfd.h>
#include <unistd.h>
#include <errno.h>

/* TODO: add timer_queue. */
namespace network {
class Epoll: private libbase::Noncopyable {
 public:
  typedef std::function<void()> CallbackType;
  Epoll()
    : epoll_fd_(-1), wakeup_fd_(-1), process_id_(-1), 
      wakeup_chann_(this), running_(false), is_handling_events_(false), 
      events_(default_event_num_), timer_queue_(this) { 
  }

  ~Epoll() { 	
	::close(wakeup_fd_);
    ::close(epoll_fd_);
  }

  void Initialize();
  void MustInLoopThread();
  void Run();
  void Stop();
  void RunNowOrLater(const CallbackType& req);
  void RunLater(const CallbackType& req);

  bool HasChannel(EventChannel* );
  void AddChannel(EventChannel* );
  void DelChannel(EventChannel* );
  void UpdateChannel(EventChannel* );

  void RunTimer(TimerQueue::TimerIdType* id, Timer& timer);
  void CancelTimer(TimerQueue::TimerIdType* id);
  int ThreadId() const {
    return process_id_;
  }
 private:
  void StopInLoop();
  void SetEvents(EventChannel*, int op);
  void HandleEvents(int events_num);
  void HandleEventsImpl(EventChannel*, uint32_t);
  void HandleRequests();
  void Wakeup();
  void CloseWakeupChannel();
  void ReadWakeupChannel();
  void WriteWakeupChannel();

  int epoll_fd_;		
  int wakeup_fd_;
  pid_t process_id_;
  EventChannel wakeup_chann_;	
  std::atomic<bool> running_;
  std::atomic<bool> is_handling_events_;
  std::vector<struct epoll_event> events_; 
  std::vector<CallbackType> pending_requests_; 
  std::unordered_set<EventChannel*> channels_;
  libbase::MutexLock mu_;
  TimerQueue timer_queue_;
  static const size_t default_event_num_;
};	
} // namespace network
#endif // GLUE_NETWORK_EPOLL_H_
