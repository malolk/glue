#include "network/tcp_server.h"

namespace glue_network {

void TcpServer::ReadListenSocket() {
  while (true) {
    int conn_fd = acceptor_.Accept();
    if (conn_fd >= 0) {
      dispatch_counter_++;
      Epoll* epoll_ptr = NULL;
      if (dispatch_counter_ == pool_size_) {
        epoll_ptr = EventLoop::epoll_ptr_;
        epoll_ptr->RunNowOrLater(std::bind(&TcpServer::NewConnection, this, conn_fd, read_cb_));
        dispatch_counter_ = 0;
      } else {
        EventLoop* eventloop_ptr = eventloop_pool_.NextEventLoop();
        epoll_ptr = eventloop_ptr->EpollPtr();
        epoll_ptr->RunNowOrLater(std::bind(&EventLoop::NewConnection, eventloop_ptr, conn_fd, read_cb_));
      }
      LOG_INFO("Accept connection on fd=%d, and transfer to epoll in thread=%d", conn_fd, epoll_ptr->ThreadId());
    } else {
      if (conn_fd == Socket::kRETRY) {
        /* Already accept all the connection. */
        break;
      } else {
        /* Error occured. Maybe there is no enough file descriptors. */
        LOG_ERROR("error occured when reading listen socket.");
        break;
      }
    }
  }
}

void TcpServer::Routine() {
  Epoll epoller;
  epoller.Initialize();
  eventloop_pool_.Start();
  {
    glue_libbase::MutexLockGuard m(mu_);
    EventLoop::epoll_ptr_ = &epoller;
    condvar_.NotifyOne();
  }
  EventChannel acceptor_chann(EventLoop::epoll_ptr_, acceptor_.Fd());
  /* TODO: Here, WriteListenSocket and CloseListenSocket do nothing but just to be in consistence with 
   * the EventChannel::Initialize. Fix it later. */
  acceptor_chann.Initialize(std::bind(&TcpServer::ReadListenSocket, this), WriteListenSocket, CloseListenSocket);
  acceptor_chann.AddIntoLoopWithRead();
  epoller.Run();
  /* Stop the threadpool. */
  eventloop_pool_.Shutdown();
  running_ = false;
}

void TcpServer::StartInThread() {
  EventLoop::thread_.Start();
  running_ = true;
  thread_.Schedule(std::bind(&TcpServer::Routine, this));
  {
    glue_libbase::MutexLockGuard m(mu_);
    while (EventLoop::epoll_ptr_ = NULL) {
      condvar_.Wait();
    }
  }
}

void TcpServer::Shutdown() {
  LOG_CHECK(running_, "");
  EventLoop::Stop();  /* It's thread-safe here. */
  EventLoop::Join();
}
} // namespace glue_network
