#ifndef GLUE_NETWORK_EVENTCHANNEL_H_
#define GLUE_NETWORK_EVENTCHANNEL_H_

#include "../libbase/noncopyable.h"
#include "../libbase/logger.h"

#include <memory>
#include <string>
#include <functional>
#include <atomic>

namespace glue_network {
// just a interface class for epoll
class Epoll;
class EventChannel: private glue_libbase::Noncopyable {
 public:
  typedef std::function<void()> CallbackType;
  EventChannel(Epoll* ep, int fd)
    : io_fd_(fd), epoll_ptr_(ep), event_fields(0), 
      is_notify_write_(false), is_notify_read_(false) {
    LOG_CHECK(io_fd_ >= 0, "");
    LOG_CHECK(epoll_ptr_ != NULL, "");
  }

  ~EventChannel() {
  }

  void SetCallbacks(const CallbackType& read_cb, const CallbackType& write_cb, 
                    const CallbackType& close_cb_) {
    read_cb_ = read_cb;
    write_cb_ = write_cb;
    close_cb_ = close_cb;
  }

  void HandleRead();
  void HandleWrite();
  void HandleClose();

  void AddIntoEpollWithRead();
  void AddIntoEpollWithWrite();
  void AddIntoEpoll();
  void DeleteFromEpoll();
  void EnableRDWR();
  void DisableRDWR();
  void EnableRD();
  void DisableRD();
  void EnableWR();
  void DisableWR();

  bool IsWriteEnabled() { 
    return write_enabled_; 
  }

  bool IsReadEnabled() { 
    return read_enabled_; 
  }

  int Fd() const { 
    return io_fd_; 
  }

  Epoll* EpollPtr() const { 
    return epoll_ptr_; 
  }
 
  uint32_t EventFields() const {
    return event_fields;
  }
 private:
  void EnableRDWR(bool flag, uint32_t setBit);
  void AddIntoEpollWithReadInEpoll();
  void AddIntoEpollWithWriteInEpoll();

  int io_fd_;
  Epoll* epoll_ptr_;
  uint32_t event_fields;
  bool write_enabled_;
  bool read_enabled_;
  CallbackType read_cb_;
  CallbackType write_cb_;
  CallbackType close_cb_;
};
} // namespace glue_network
#endif // GLUE_NETWORK_EVENTCHANNEL_H_
