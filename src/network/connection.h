#ifndef NETWORK_CONNECTION_H
#define NETWORK_CONNECTION_H

#include "socket.h"
#include "epoll.h"
#include "buffer.h"
#include "event_channel.h"
#include "../libbase/noncopyable.h"
#include "../libbase/timeutil.h"
#include "../libbase/logger.h"

#include <functional>
#include <memory>
#include <atomic>

namespace glue_network {
class Connection: private libbase::Noncopyable,
				  public std::enable_shared_from_this<Connection> {
 public:
  typedef std::function<void(std::shared_ptr<Connection>&, BUF& )> CallbackReadType;
  typedef std::function<void(std::shared_ptr<Connection>&)> CallbackInitType;
  typedef std::function<void()> CallbackCloseType;

  enum {
    kCONNECTED = 10,
    kCLOSING = 20,
    kCLOSED = 30,
  };
  Connection(int sockfd, poller::Epoll* ep)
    : sockfd_(sockfd), epoll_ptr_(ep), channel_(ep, sockfd),
	  state_(CONNECTED) { 
    LOG_CHECK(sockfd_ >= 0 && ep != NULL, "");
  }
	
  ~Connection() {
	LOG_INFO("Connection on fd=%d destructed", sockfd_);
  }

  void SetReadOperation(const CallbackReadType& cb);
  void SetCloseOperation(const CallbackCloseType& cb);
  void SetInitOperation(const CallbackInitType& cb);
　void Send(ByteBuffer& data);
  void WriteCallback();
  void ReadCallback();
  void Close();
  void Shutdown();
  void ShutdownNow();
  void Initialize(); 
  Epoll* EpollPtr() const { 
    return epoll_ptr_; 
  } 

 private:
  void SendInLoopThread(ByteBuffer& data);
  void StopWrite();
  void StopRead();
  int sockfd_;
  Epoll* epoll_ptr_;
  EventChannel channel_;
  /* state_ should be Atomic variable */
  std::atomic<int> state_;
  ByteBuffer send_buf_;
  ByteBuffer recv_buf_;
  CallbackReadType read_cb_;
  CallbackCloseType close_cb_;
  CallbackInitType init_cb_;
};
} // namespace glue_network
#endif // GLUE_NETWORK_CONNECTION_H_
