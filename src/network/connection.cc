#include "network/connection.h"

namespace network {
void Connection::Initialize() {
  channel_.Initialize(std::bind(&Connection::ReadCallback, this),
                      std::bind(&Connection::WriteCallback, this),
                      std::bind(&Connection::Close, this));
  channel_.AddIntoLoopWithRead();
  if (init_cb_) {
    std::shared_ptr<Connection> conn_ptr = shared_from_this();
    init_cb_(conn_ptr);
  }
}

void Connection::SetReadOperation(const CallbackReadType& cb) {
  if (!cb) {
    LOG_FATAL("Connection's read callback should be valide");
  } else {
    read_cb_ = cb;
  }
}

void Connection::SetInitOperation(const CallbackInitType& cb) {
  if (!cb) {
    LOG_FATAL("Connection's Init callback should be valide");
  } else {
    init_cb_ = cb;	
  }
}

void Connection::SetCloseOperation(const CallbackCloseType& cb) {
  if (!cb) {
    LOG_FATAL("Connection's close callback should be valide");
  } else {
    close_cb_ = cb;
  }
}

void Connection::Send(libbase::ByteBuffer& data) {
  /* I/O operation in loop thread is thread-safe. */
  epoll_ptr_->RunNowOrLater(std::bind(&Connection::SendInLoopThread,
                          this, data));
}

void Connection::SendInLoopThread(libbase::ByteBuffer data) {
  ssize_t sent_num = 0;
  const size_t send_size = data.ReadableBytes();
  if (send_buf_.ReadableBytes() == 0) {
	sent_num = Socket::Send(sockfd_, data);
	if (sent_num == Socket::kERROR) {
      if (state_ != kCLOSED) {
        /* Write error, so close it. */
        epoll_ptr_->RunLater(std::bind(&EventChannel::HandleClose, &channel_));
      }
	  return;
	} else if (static_cast<size_t>(sent_num) == send_size) {
	  /* Maybe connection already is in closing state and write is finished, 
       * then shutdown the connection. */
	  if (state_ == kCLOSING) {
	    Close();
      }
	}
  }
	
  /* Write is not finished. Enable the write-notification if necessary. */
  if (send_size > static_cast<size_t>(sent_num)) {
    send_buf_.Append(data.AddrOfRead(), send_size - sent_num);
    data.MoveReadPos(send_size - sent_num);
	if (!channel_.IsWriteEnabled()) {
	  channel_.EnableWR();
    }
  }
}

void Connection::WriteCallback() {
  epoll_ptr_->MustInLoopThread();

  ssize_t sent_num = Socket::Send(sockfd_, send_buf_);
  if (sent_num == Socket::kERROR) {
    if (state_ != kCLOSED) {
	  epoll_ptr_->RunLater(std::bind(&EventChannel::HandleClose, &channel_));
    }
	return;
  }
  if (send_buf_.ReadableBytes() == 0) {	
    /* Write is finished. Close it now if connection is closing. */
	if (state_ == kCLOSING) {
	  Close();
    }
  }
}

void Connection::ReadCallback() {
  epoll_ptr_->MustInLoopThread();

  ssize_t recv_num = Socket::Receive(sockfd_, recv_buf_);
  if (recv_num > 0) {
    std::shared_ptr<Connection> conn_ptr = shared_from_this();
	read_cb_(conn_ptr, recv_buf_);  
  } else if (recv_num == Socket::kNODATA) {
    /* No data. */
	return;
  } else if (recv_num == 0) {
    /* Peer closed. */
    if (state_ != kCLOSED) {
	  epoll_ptr_->RunLater(std::bind(&EventChannel::HandleClose, &channel_));
    }
  }	else {
    /* Error occured, log this and close it. */
    if (state_ != kCLOSED) {
	  epoll_ptr_->RunLater(std::bind(&EventChannel::HandleClose, &channel_));
    }
	LOG_ERROR("read error on connection of fd=%d", sockfd_);
  }
}

void Connection::Close() {
  epoll_ptr_->MustInLoopThread();
  if (state_ == kCLOSED) {
    return;
  } else {
    state_ = kCLOSED;
  }
  channel_.DisableRDWR();
  /* close_cb_ may erase current connection from connection-pool. 
   * But maybe there are events using this connection, 
   * so we should delete the connection after all the events of 
   * this connection have been processed. For the same reason, 
   * we can't delete the channel from the epoll. */
  close_cb_();
}

/* This callback would be invoked in loop after current connection removed from 
 * connection pool by the server. After this function being invoked, current
 * connection will be destructed. */
void Connection::DestroyedInLoop(std::shared_ptr<Connection> conn_ptr) {
  epoll_ptr_->MustInLoopThread();
  channel_.DeleteFromLoop();
  LOG_INFO("connection on fd=%d closed", sockfd_);
}

void Connection::ShutdownNow() {
  if (state_ != kCONNECTED) {
    return;
  }
  if (state_ != kCLOSED) {
    epoll_ptr_->RunNowOrLater(std::bind(&EventChannel::HandleClose, &channel_));
  }
}

/* Could be used across threads. We first close write and close entirely when 
 * peer close the connection. So current connection is in closing state. */
void Connection::Shutdown() {
  if (state_ != kCONNECTED) {
    return;
  }
  state_ = kCLOSING;
  epoll_ptr_->RunNowOrLater(std::bind(&Connection::StopWrite, this));
}

void Connection::StopWrite() {
  epoll_ptr_->MustInLoopThread();
  LOG_CHECK(state_ == kCLOSING, "");
  if (channel_.IsWriteEnabled()) {
    channel_.DisableWR();
  }
}
} // namespace network
