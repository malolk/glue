#include "epoll.h"
#include "event_channel.h"

#include <functional>
#include <algorithm>
#include <string>
#include <sstream>

#include <pthread.h>

namespace glue_network {
const size_t Epoll::default_event_num_ = 1024;

void Epoll::Initialize() {
  epoll_fd_ = ::epoll_create1(EPOLL_CLOEXEC);
  LOG_CHECK(epoll_fd_ >= 0, "");
  process_id_ = glue_libbase::ThreadId();
  wakeup_fd_ = ::eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK);
  LOG_CHECK(wakeup_fd_ >= 0, "");
  
  wakeup_chann_.Initialize(std::bind(&Epoll::ReadWakeupChannel, this), 
                          std::bind(&Epoll::WriteWakeupChannel, this),
                          std::bind(&Epoll::CloseWakeupChannel, this), wakeup_fd_);
  wakeup_chann_.AddIntoLoopWithRead();
}

void Epoll::MustInLoopThread() {
  LOG_CHECK(process_id_ == glue_libbase::ThreadId(), "Not the thread where epoll created. ");
}

void Epoll::Stop() {
  RunLater(std::bind(&Epoll::StopInLoop, this));	
}

void Epoll::StopInLoop() {
  MustInLoopThread();
  running_ = false;      
  Wakeup();
}

void Epoll::RunNowOrLater(const CallbackType& req) {   
  if (process_id_ == glue_libbase::ThreadId()) {
    req();
  } else {    
	RunLater(req);
  }
}           

void Epoll::RunLater(const CallbackType& req) {
  {
	glue_libbase::MutexLockGuard m(mu_);
	pending_requests_.push_back(req);    
  }
  if (process_id_ != glue_libbase::ThreadId() || 
      is_handling_events_) {
    Wakeup();
  }
}

void Epoll::AddChannel(EventChannel* ev) {
  LOG_CHECK(ev != NULL, "");
  MustInLoopThread();
#ifndef NDEBUG
  if (HasChannel(ev)) {
    LOG_WARN("Already added channel on fd=%d", ev->Fd());
    return;
  }
#endif
  SetEvents(ev, EPOLL_CTL_ADD);
  channels_.insert(ev);
}

void Epoll::UpdateChannel(EventChannel* ev) {
  LOG_CHECK(ev != NULL, "");
  MustInLoopThread();
#ifndef NDEBUG
  if (!HasChannel(ev)) {
    LOG_WARN("Channel on fd=%d not added in loop", ev->Fd());
    return;
  }
#endif
  SetEvents(ev, EPOLL_CTL_MOD);
}

void Epoll::DelChannel(EventChannel* ev) {
  LOG_CHECK(ev != NULL, "");
  MustInLoopThread();

  std::unordered_set<EventChannel*>::iterator it = channels_.find(ev);
  if (it == channels_.end()) {
	return;
  } else {
	ev->DisableRDWR();
	SetEvents(ev, EPOLL_CTL_DEL);
    channels_.erase(it);
  }
}

void Epoll::SetEvents(EventChannel* ev, int op) {
  struct epoll_event event;
  event.events = ev->EventFields();
  event.data.ptr = ev;
  int ret = epoll_ctl(epoll_fd_, op, ev->Fd(), &event);
  if (ret) { 
	LOG_ERROR("epoll_ctl failed on fd=%d", ev->Fd());	
  }
}

void Epoll::Run() {
  MustInLoopThread();
  LOG_CHECK(!running_, "");
  running_ = true;
  while (running_) {
	int ret = epoll_wait(epoll_fd_, &(*events_.begin()), static_cast<int>(events_.size()), -1);
	LOG_CHECK(ret > 0, "");
	if (static_cast<size_t>(ret) == events_.size()) {
	  events_.resize(2 * events_.size());
    }
	is_handling_events_ = true;
	HandleEvents(ret);
	is_handling_events_ = false;
	HandleRequests();
  }	
}

bool Epoll::HasChannel(EventChannel* ev) {
  std::unordered_set<EventChannel*>::iterator it = channels_.find(ev);
  return (it != channels_.end());
}

void Epoll::HandleEvents(int events_num) {
  MustInLoopThread();
  for (int index = 0; index < events_num; ++index) {
	EventChannel* chann_ptr = static_cast<EventChannel*>(events_[index].data.ptr);
	uint32_t ret_fields = events_[index].events;
	HandleEventsImpl(chann_ptr, ret_fields);
  }
}

void Epoll::HandleEventsImpl(EventChannel* chann_ptr, uint32_t ret_fields) {
#ifndef NDEBUG
  LOG_CHECK(HasChannel(chann_ptr), "");
#endif
  if (ret_fields & EPOLLIN || ret_fields & EPOLLRDHUP || ret_fields & EPOLLPRI) {
    chann_ptr->HandleRead();
  } else if (ret_fields & EPOLLOUT) {
	chann_ptr->HandleWrite();
  } else if (ret_fields & EPOLLERR) {
	LOG_WARN("epoll internal error");
	chann_ptr->HandleClose();
  } else if (!(ret_fields & EPOLLIN) && (ret_fields & EPOLLHUP)) {
	LOG_WARN("Hub on fd: %d", chann_ptr->Fd());
	chann_ptr->HandleClose();
  }
}

void Epoll::HandleRequests() {
  std::vector<CallbackType> req_buf;
  {
    glue_libbase::MutexLockGuard m(mu_);
	std::swap(req_buf, pending_requests_);
  }
  
  for (std::vector<CallbackType>::iterator it = req_buf.begin();
	   it != req_buf.end(); ++it) {
    (*it)();
  }
}

void Epoll::Wakeup() {
  uint64_t buf = 1;
  ssize_t ret = ::write(wakeup_fd_, &buf, sizeof(buf));
  if (ret < 0) {
	int err = errno;
	if (err == EINTR || err == EAGAIN) {
	  ret = ::write(wakeup_fd_, &buf, sizeof(buf));
	  LOG_CHECK(ret == sizeof(buf), "");
	} else {
	  LOG_ERROR("Wakeup error");		
	}
  }
}

void Epoll::ReadWakeupChannel() {
  uint64_t buf;
  ssize_t ret = ::read(wakeup_fd_, &buf, sizeof(buf));
  if (ret < 0) {
    int err = errno;
	if (err == EINTR) {
	  ret = ::read(wakeup_fd_, &buf, sizeof(buf));
	  if (ret < 0) {
	    LOG_WARN("event read error: EINTR");
      }
	} else if (err == EAGAIN) {
	  LOG_WARN("event read: EAGAIN");
    } else {
	  LOG_ERROR("Wakeup error");		
	}
  }
}

void Epoll::CloseWakeupChannel() {
  wakeup_chann_.DisableRDWR();
}

void Epoll::WriteWakeupChannel() {
  // Nothing.   
}

} // namespace glue_network
