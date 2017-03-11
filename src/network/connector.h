#ifndef GLUE_NETWORK_CONNECTOR_H_
#define GLUE_NETWORK_CONNECTOR_H_

#include "network/socket.h"
#include "network/socket_address.h"
#include "libbase/noncopyable.h"
#include "libbase/loggerutil.h"

namespace network {
class Connector : private libbase::Noncopyable {
 public: 
  /* Get fully connected socket. */
  static int GetConnectedSocket(int max_runs, const SocketAddress& server_addr, int non_block = 1) {
    Connector conn(server_addr, non_block);
    return conn.Connect(max_runs);
  }
 private:
  /* Default non-blocking I/O. */
  explicit Connector(const SocketAddress& server_addr, bool non_block = 1) 
    : sockfd_(-1), non_block_(non_block), server_addr_(server_addr) {
    int family = server_addr_.Family();
    if (family == AF_INET) {
      sockfd_ = Socket::NewSocket();
    } else if (family == AF_INET6) {
      sockfd_ = Socket::NewSocket(AF_INET6, SOCK_STREAM, 0);
    }
    LOG_CHECK(sockfd_ >= 0, "");
    Socket::EnablePortReuse(sockfd_, 1);
  }

  ~Connector() {
  }

  /* Retry max_runs times when connect failed. */
  int Connect(int max_runs) {
    LOG_CHECK(max_runs >= 0, "");
    while (max_runs-- >= 0) {
      int ret = Socket::Connect(sockfd_, server_addr_);
      if (ret == 0) {
        Socket::EnableNonBlock(sockfd_, non_block_);
        return sockfd_;
      } else if (ret == Socket::kERROR) {
        /* Error occured. */
        break;
      } else if (ret == Socket::kRETRY) {
        continue;
      }
    }
    /* failed, so close the sockfd_. */
    Socket::Close(sockfd_);
    return -1;
  }

  int sockfd_;
  bool non_block_;
  const SocketAddress& server_addr_;
};
} // namespace network
#endif // GLUE_NETWORK_CONNECTOR_H_

