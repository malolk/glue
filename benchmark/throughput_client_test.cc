#include "network/tcp_client.h"
#include "network/socket_address.h"
#include "network/connection.h"
#include "network/epoll.h"
#include "network/timer.h"
#include "network/eventloop_pool.h"
#include "libbase/buffer.h"
#include "libbase/noncopyable.h"
#include "libbase/logger.h"
#include "libbase/timeutil.h"

#include <vector>
#include <string>
#include <iostream>
#include <functional>
#include <memory>

#include <unistd.h>
#include <stdlib.h>

using namespace glue_network;
using namespace glue_libbase;

ByteBuffer ball;

class Client : private Noncopyable {
 public:
  explicit Client(const SocketAddress& server_addr)
    : cli_(server_addr, 10), send_num_(0), running_(false) {
  }

  ~Client() {
  }

  void Initialize() {
    cli_.Initialize(std::bind(&Client::InitCallback, this, std::placeholders::_1), 
                    std::bind(&Client::ReadCallback, this, std::placeholders::_1, std::placeholders::_2)); 
  }

  void Start(Epoll* ep) {
    cli_.Start(ep);
  }

  void Start(EventLoop* el) {
    cli_.Start(el);
  }

  void Stop() {
    running_ = false;
  }

  size_t GetSentNum() const {
    return send_num_;
  }
 private:
  void ReadCallback(std::shared_ptr<glue_network::Connection> conn, ByteBuffer& buf);
  void InitCallback(std::shared_ptr<glue_network::Connection> conn);
  TcpClient cli_;
  size_t send_num_;
  std::atomic<bool> running_;
};

void Client::ReadCallback(std::shared_ptr<glue_network::Connection> conn, ByteBuffer& buf) {
  if (!running_) {
    conn->Close();
  } else {
    send_num_ += buf.ReadableBytes();
    conn->Send(buf);  
  }
}

void Client::InitCallback(std::shared_ptr<glue_network::Connection> conn) {
  running_ = true;
  ByteBuffer ball_backup;
  ball_backup.AppendBuffer(ball);
  conn->Send(ball_backup);    
}

// just like TcpServer
class ClientCluster : private Noncopyable {
 public:
  ClientCluster(const SocketAddress& server_addr, int client_num,  int time_range, int thread_num)
    : server_addr_(server_addr), client_num_(client_num), thread_num_(thread_num),  
      time_range_(time_range), epoll_ptr_(NULL), eventloop_pool_(thread_num - 1) { 
  }

  ~ClientCluster() {}

  void Start() {
    Epoll epoller;
    epoller.Initialize();
    epoll_ptr_ = &epoller;
      
    if (thread_num_ > 1) {
      eventloop_pool_.Start();
    }
   
    glue_network::Timer timer(std::bind(&ClientCluster::Timeout, this), glue_libbase::TimeUtil::NowMicros() + time_range_ * 1000000LL);
    epoller.RunTimer(NULL, timer);
    
    for (int index = 0; index < client_num_; ++index) {
      cli_cluster_.push_back(std::unique_ptr<Client>(new Client(server_addr_)));
      cli_cluster_[index]->Initialize();
      if (thread_num_ > 1 && (index % (client_num_))) {
        cli_cluster_[index]->Start(eventloop_pool_.NextEventLoop());
      } else {
        cli_cluster_[index]->Start(epoll_ptr_);
      }
    }      
    epoller.Run();
    eventloop_pool_.Shutdown();
    CalcThroughput();    
  }
  
 private:
  void Timeout() {
    for (int index = 0; index < client_num_; ++index) {
      cli_cluster_[index]->Stop();        
    }
    epoll_ptr_->Stop();
  }

  void CalcThroughput() {
    long double sum = 0;
    for (int index = 0; index < client_num_; ++index) { 
      sum += cli_cluster_[index]->GetSentNum();
    }
    std::cout << "Running Time=" << time_range_
              << " " << "Received Bytes="<< sum 
              << " Throughput="<< (static_cast<double>(sum)/static_cast<double>(1024 * 1024 * time_range_)) 
              << " MiB/s" << std::endl;
  }

  glue_network::SocketAddress server_addr_;
  const int client_num_;
  const int thread_num_;
  const int time_range_;
  glue_network::Epoll* epoll_ptr_;
  std::vector<std::unique_ptr<Client>> cli_cluster_;
  glue_network::EventLoopPool eventloop_pool_;
};

int main(int argc, char* argv[]) {
  int kBytes = 16;
  int client = 20000;
  int time = 30;
  int thread = 1;
  std::string ip_str = "127.0.0.1";
  uint16_t port = 8080;
  int opt;
  while ((opt = getopt(argc, argv, "s:p:c:k:t:T:")) != -1) {
    switch (opt) {
      case 's': {
        ip_str = std::string(optarg);
        break;  
      }
#pragma GCC diagnostic ignored "-Wold-style-cast"            
      case 'p': {
        port = (uint16_t)atoi(optarg);
        break;  
      }
#pragma GCC diagnostic error "-Wold-style-cast"            
      case 'c': { 
        client = atoi(optarg);
        break;  
      }
      case 'k': {
        kBytes = atoi(optarg);
        break;  
      }
      case 't': {
        time = atoi(optarg);
        break;  
      }
      case 'T': {
        thread = atoi(optarg);
        break;
      }
      default:  {
        std::cerr << "usage:" 
                     "<ip> <port> <clients> <chunk size>" 
                     "<time> <thread>" 
                  << std::endl;
        return 1;
      }
    }        
  }  
  
  const std::string unit = "0123456789";
  for (int cnt = 0; cnt < kBytes*100; ++cnt) {
    ball.AppendString(unit);  
  }
  
  glue_network::SocketAddress server_addr(ip_str, port);  
  ClientCluster cluster(server_addr, client, time, thread);
  cluster.Start();
  
  return 0;  
}
