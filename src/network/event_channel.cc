#include "network/event_channel.h"
#include "network/epoll.h"
#include "libbase/loggerutil.h"

namespace glue_network {  
void EventChannel::HandleRead() {  
  epoll_ptr_->MustInLoopThread();
  if (read_cb_) {
	read_cb_();
  } else {
#ifndef NDBUG
    LOG_WARN("Read events occured, but read_cb_ not set");
#endif
  }
}
   
void EventChannel::HandleWrite() {  
  epoll_ptr_->MustInLoopThread();
  if (write_cb_) {
	write_cb_();
  } else {
#ifndef NDBUG
    LOG_WARN("Write events occured, but write_cb_ not set");
#endif
  }
}
  
void EventChannel::HandleClose() { 
  epoll_ptr_->MustInLoopThread();
  if (close_cb_) {
    close_cb_();
  } else {
#ifndef NDBUG
    LOG_WARN("Write events occured, but write_cb_ not set");
#endif
  }
}
   
void EventChannel::AddIntoLoop() {
  epoll_ptr_->RunNowOrLater(std::bind(&Epoll::AddChannel, epoll_ptr_, this));
}

void EventChannel::AddIntoLoopWithRead() {
  epoll_ptr_->RunNowOrLater(std::bind(&EventChannel::AddIntoLoopWithReadInLoopThread, this));
}

void EventChannel::AddIntoLoopWithReadInLoopThread() {
  epoll_ptr_->MustInLoopThread();
  epoll_ptr_->AddChannel(this);
  EnableRD();
}

void EventChannel::AddIntoLoopWithWrite() {
  epoll_ptr_->RunNowOrLater(std::bind(&EventChannel::AddIntoLoopWithWriteInLoopThread, this));
}

void EventChannel::AddIntoLoopWithWriteInLoopThread() {
  epoll_ptr_->MustInLoopThread();
  epoll_ptr_->AddChannel(this);
  EnableWR();
}

void EventChannel::DeleteFromLoop() {
  epoll_ptr_->MustInLoopThread();
#ifndef NDEBUG
  LOG_CHECK(epoll_ptr_->HasChannel(this), "");
#endif
  epoll_ptr_->DelChannel(this);
}

void EventChannel::EnableRD() { 
  EnableEvent(true, EPOLLIN | EPOLLRDHUP); 
  read_enabled_ = true; 
}

void EventChannel::DisableRD() { 
  EnableEvent(false, EPOLLIN | EPOLLRDHUP); 
  read_enabled_ = false; 
}

void EventChannel::EnableWR() { 
  EnableEvent(true, EPOLLOUT); 
  write_enabled_ = true; 
}

void EventChannel::DisableWR()  { 
  EnableEvent(false, EPOLLOUT); 
  write_enabled_ = false; 
}

void EventChannel::EnableRDWR() { 
  EnableEvent(true, EPOLLIN | EPOLLRDHUP | EPOLLOUT); 
  read_enabled_ = true; 
  write_enabled_ = true; 
}

void EventChannel::DisableRDWR() { 
  EnableEvent(false, EPOLLIN | EPOLLRDHUP | EPOLLOUT); 
  read_enabled_ = false; 
  write_enabled_ = false; 
}

void EventChannel::EnableEvent(bool flag, uint32_t set_bit) {
  epoll_ptr_->MustInLoopThread();
  event_fields_ = (flag ? (event_fields_ | set_bit) : (event_fields_ & (~set_bit)));
  epoll_ptr_->RunNowOrLater(std::bind(&Epoll::UpdateChannel, epoll_ptr_, this));
}

} // namespace glue_network
