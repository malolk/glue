#ifndef GLUE_NETWORK_ACCEPTOR_H_
#define GLUE_NETWORK_ACCEPTOR_H_

#include "libbase/logger.h"
#include "libbase/noncopyable.h"
#include "network/buffer.h"
#include "network/socket.h"
#include "network/socket_address.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>

namespace glue_network {
class Acceptor : private glue_libbase::Noncopyable {
 public:
  /* default setting is non-blocking I/O. */
  explicit Acceptor(const SocketAddress& server_addr, int non_block = 1) 
    : server_addr_(server_addr) {
    int family = server_addr_.Family();
    if (family == AF_INET) {
      listen_fd_ = Socket::NewSocket();
    } else if (family == AF_INET6) {
      listen_fd_= Socket::NewSocket(AF_INET6, SOCK_STREAM, 0);
    }
    LOG_CHECK(listen_fd_ >= 0, "");
    Socket::EnablePortReuse(listen_fd_, 1);
    Socket::EnableAddrReuse(listen_fd_, 1);
    Socket::EnableNonBlock(listen_fd_, non_block);

    /* Could use more flexible settings fot backlog? */
    int somaxconn_fd = ::open("/proc/sys/net/core/somaxconn", O_RDONLY);
    ByteBuffer file_buf;
    file_buf.ReadFd(somaxconn_fd);
    int64_t somaxconn = atoll(file_buf.ToString().c_str());
    LOG_CHECK(somaxconn > 0, "");
    Socket::BindAndListen(listen_fd_, server_addr_, 100);
  }

  int Accept() {
    /* default is non-blocking I/O. See Socket::Accept() for details. */
    int cli_fd = Socket::Accept(listen_fd_, NULL);
    if (cli_fd >= 0) {
      return cli_fd;
    } else {
      if (cli_fd == Socket::kRETRY) {
        return cli_fd;
      } else {
        LOG_ERROR("Acceptor::Accept() failed");
        /* TODO: Maybe there is no enough file descriptors, then free some backup fds. */
        return Socket::kERROR;
      }
    }
  }

  int Fd() const {
    return listen_fd_;
  }

  ~Acceptor() {
    Socket::Close(listen_fd_);
  }
 private:
  int listen_fd_;
  SocketAddress server_addr_;
};
} // namespace glue_network
#endif // GLUE_NETWORK_ACCEPTOR_H_
