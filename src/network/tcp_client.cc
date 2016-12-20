#include "tcp_client.h"

namespace glue_network {
void TcpClient::Start() {
  sockfd_ = Connector::GetConnectedSocket(max_runs_, server_addr_, 1);
  LOG_CHECK(sockfd_ >= 0, "connect failed");
  Epoll epoller;
  epoller.Initialize();
  epoll_ptr_ = &epoller;
  conn_ = std::shared_ptr<Connection>(new Connection(sockfd_, &epoller));
  conn_->SetReadOperation(read_cb_);
  conn_->SetInitOperation(init_cb_);
  conn_->SetCloseOperation(std::bind(&TcpClient::Close, this));
  conn_->Initialize();
  epoller.Run();
}

void TcpClient::Initialize(const Connection::CallbackInitType& init_cb, 
                           const Connection::CallbackReadType& read_cb) {
  if (!init_cb || !read_cb) {
    LOG_FATAL("init callback and read callback should be valide.");
  } else {
    init_cb_ = init_cb;
    read_cb_ = read_cb;
  }
}
// for connection closed by client actively
void TcpClient::Stop() {
  epoll_ptr_->Stop();	
}

void TcpClient::Close() {
  std::shared_ptr<Connection> conn_backup = conn_;
  conn_.reset();
  epoll_ptr_->RunLater(std::bind(&TcpClient::DeleteInLoop, this, conn_backup));  
  Stop();
}

void TcpClient::DeleteInLoop(std::shared_ptr<Connection> conn_shared_ptr) {
  conn_shared_ptr->DestroyedInLoop(conn_shared_ptr);
}
} // namespace glue_network
