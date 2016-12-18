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
  explicit EventChannel(Epoll* ep) 
    : io_fd_(-1), epoll_ptr_(ep), event_fields_(0),
      write_enabled_(false), read_enabled_(false) {
    LOG_CHECK(epoll_ptr_ != NULL, "");
  }

  EventChannel(Epoll* ep, int fd)
    : io_fd_(fd), epoll_ptr_(ep), event_fields_(0), 
      write_enabled_(false), read_enabled_(false) {
    LOG_CHECK(io_fd_ >= 0, "");
    LOG_CHECK(epoll_ptr_ != NULL, "");
  }

  ~EventChannel() {
  }

  void Initialize(const CallbackType& read_cb, const CallbackType& write_cb,
                  const CallbackType& close_cb, int fd = -1) {
    read_cb_ = read_cb;
    write_cb_ = write_cb;
    close_cb_ = close_cb;
    if (fd < 0) {
      LOG_CHECK(io_fd_ >= 0, "io_fd_ is not set yet");
    } else {
      io_fd_ = fd; 
    }
  }
  void HandleRead();
  void HandleWrite();
  void HandleClose();

  void AddIntoLoopWithRead();
  void AddIntoLoopWithWrite();
  void AddIntoLoop();
  void DeleteFromLoop();
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
    return event_fields_;
  }
  
 private:
  void EnableEvent(bool flag, uint32_t set_bit);
  void AddIntoLoopWithReadInLoopThread();
  void AddIntoLoopWithWriteInLoopThread();

  int io_fd_;
  Epoll* epoll_ptr_;
  uint32_t event_fields_;
  bool write_enabled_;
  bool read_enabled_;
  CallbackType read_cb_;
  CallbackType write_cb_;
  CallbackType close_cb_;
};
} // namespace glue_network
#endif // GLUE_NETWORK_EVENTCHANNEL_H_
