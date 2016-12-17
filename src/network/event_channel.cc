#include "event_channel.h"
#include "epoll.h"
#include "../libbase/logger.h"

namespace glue_network {  
void EventChannel::HandleRead() {  
  epoll_ptr_->MustInEpollThread();
  if (read_cb_) {
	read_cb_();
  }
}
   
void EventChannel::HandleWrite() {  
  epoll_ptr_->MustInEpollThread();
  if (write_cb_) {
	write_cb_();
  }
}
  
void EventChannel::HandleWrite() {  
  epoll_ptr_->MustInEpollThread();
  if (close_cb_) {
    close_cb_();
  }
}
   
void EventChannel::AddIntoEpoll() {
  epoll_ptr_->RunNowOrLater(std::bind(&Epoll::AddChannel, epoll_ptr_, this));
}

void EventChannel::AddIntoEpollWithRead() {
  epoll_ptr_->RunNowOrLater(std::bind(&EventChannel::AddIntoEpollWithReadInEpoll, this));
}

void EventChannel::AddIntoEpollWithReadInEpoll() {
  epoll_ptr_->MustInEpollThread();
  epoll_ptr_->AddChannel(this);
  EnableRD();
}

void EventChannel::AddIntoEpollWithWrite() {
  epoll_ptr_->RunNowOrLater(std::bind(&EventChannel::AddIntoEpollWithWriteInEpoll, this));
}

void EventChannel::AddIntoEpollWithWriteInEpoll() {
  epoll_ptr_->MustInEpollThread();
  epoll_ptr_->AddChannel(this);
  EnableWR();
}

void EventChannel::DeleteFromEpoll() {
  epoll_ptr_->MustInEpollThread();
#ifndef NDEBUG
  LOG_CHECK(epoll_ptr_->HasChannel(this), "");
#endif
  epoll_ptr->DelChannel(this);
}

void EventChannel::EnableRD() { 
  EnableRDWR(true, EPOLLIN | EPOLLRDHUP); 
  read_enabled_ = true; 
}

void EventChannel::DisableRD() { 
  EnableRDWR(false, EPOLLIN | EPOLLRDHUP); 
  read_enabled_ = false; 
}

void EventChannel::EnableWR() { 
  EnableRDWR(true, EPOLLOUT); 
  write_enabled_ = true; 
}

void EventChannel::DisableWR()  { 
  EnableRDWR(false, EPOLLOUT); 
  write_enabled_ = false; 
}

void EventChannel::EnableRDWR() { 
  EnableRDWR_(true, EPOLLIN | EPOLLRDHUP | EPOLLOUT); 
  read_enabled_ = true; 
  write_enabled_ = true; 
}

void EventChannel::DisableRDWR() { 
  EnableRDWR_(false, EPOLLIN | EPOLLRDHUP | EPOLLOUT); 
  read_enabled_ = false; 
  write_enabled_ = false; 
}

void EventChannel::EnableRDWR(bool flag, uint32_t set_bit) {
  epoll_ptr_->MustInEpollThread();
  event_fields = (flag ? (event_fields | set_bit) : (event_fields & (~set_bit)));
  epoll_ptr->RunNowOrLater(std::bind(&Epoll::UpdateChannel, epoll_ptr, this));
}

} // namespace glue_network
