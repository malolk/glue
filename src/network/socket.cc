#include "socket.h"
#include "../libbase/logger.h"

#include <sys/uio.h>
#include <errno.h>
#include <signal.h>
#include <string.h>

namespace glue_network {
namespace { 
class IgnoreSigpipe {
 public:
  IgnoreSigpipe() {
	struct sigaction act;
	memset(&act, '\0', sizeof(act));
	act.sa_handler = SIG_IGN;
	act.sa_flags = 0;
	int ret = ::sigaction(SIGPIPE, &act, NULL);
    if (ret != 0) {
      LOG_FATAL("sigaction for SIGPIPE failed");
    }
  }
};
/* Ignore SIGPIPE signal from startup. */
static const IgnoreSigpipe sigpipe_masker;
} // non-named namespace 

void Socket::StopWrite(int sockfd) {
  LOG_CHECK(sockfd >= 0, "");
  int ret = ::shutdown(sockfd, SHUT_WR);
  if (ret != 0) {
    LOG_FATAL("::shutdown(SHUT_WR) failed on sockfd=%d ", sockfd);
  }
}

void Socket::StopRead(int sockfd) {
  LOG_CHECK(sockfd >= 0, "");
  int ret = ::shutdown(sockfd, SHUT_RD);
  if (ret != 0) {
    LOG_FATAL("::shutdown(SHUT_RD) failed on sockfd=%d ", sockfd);
  }
}


static void Bind(int sockfd, const SocketAddress& sa) {
  LOG_CHECK(sockfd >= 0, "");
  int ret = ::bind(sockfd, sa.ToAddrTypeVoid(), sa.Length());
  if (ret != 0) {
    LOG_FATAL("::bind for sockfd=%d ip=%s port=%d failed", sockfd, sa.ToString().c_str(), sa.Port());
  }
}

void Socket::BindAndListen(int sockfd, const SocketAddress& sa, int backlog) {
  Bind(sockfd, sa);
  int ret = ::listen(sockfd, backlog);
  if (ret != 0) {
    LOG_FATAL("listen for sockfd=%d, ip=%s, port=%d failed", sockfd, sa.ToString().c_str(), sa.Port());
  }
}

int Socket::Accept(int sockfd, SocketAddress *sa) {
  socklen_t addr_len = sizeof(SocketAddress::AddrType);
  int ret = 0;
  if (sa) { 
    ret = ::accept4(sockfd, sa->ToAddrTypeVoid(), &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
  } else {
    ret = ::accept4(sockfd, NULL, NULL, SOCK_NONBLOCK | SOCK_CLOEXEC);
  }
  if (ret < 0) {
	int err = errno;
	switch (err) {
	  case EAGAIN:
      case EINTR: {
        return kRETRY; /* It's ok for non-blocking I/O. Try again later. */
      }
	  case EMFILE:
	  case ENFILE: {
        LOG_WARN("accept4() no enough fd");
        /* fall through */
	  }
	  default: {
		LOG_ERROR("accept4() failed");
		return kERROR; /* Error, maybe try it three times later. */
      }
    }
  } else {
	return ret;
  }
}

void Socket::GetPeerName(int sockfd, SocketAddress& sa) {
  socklen_t addr_len = sizeof(SocketAddress::AddrType);
  int ret = ::getpeername(sockfd, sa.ToAddrTypeVoid(), &addr_len);
  if (ret != 0) {
    LOG_FATAL("getpeername for sockfd=%d failed", sockfd);
  }
} 

void Socket::GetSockName(int sockfd, SocketAddress& sa) {
  socklen_t addr_len = sizeof(SocketAddress::AddrType);
  int ret = ::getsockname(sockfd, sa.ToAddrTypeVoid(), &addr_len);
  if (ret != 0) {
    LOG_FATAL("getsockname for sockfd=%d failed", sockfd);
  }
} 

static const int BACKUP_BUF_SIZE = 65535;
ssize_t Socket::Receive(int sockfd, ByteBuffer& buf) {
  char backup[BACKUP_BUF_SIZE];     
  const size_t buf_size = buf.WritableBytes();
  struct iovec buf_array[2];
  buf_array[0].iov_base = buf.AddrOfWrite();     
  buf_array[0].iov_len = buf_size;
  buf_array[1].iov_base = backup;
  buf_array[1].iov_len = BACKUP_BUF_SIZE;

  ssize_t recv_num = 0;
  int try_times = 0, ret_flag = 1;
  do {
    recv_num = ::readv(sockfd, buf_array, 2);
    if (recv_num >= 0) {
      break;
    } else {
      int err = errno;
      if (err == EAGAIN || EWOULDBLOCK) {
        LOG_WARN("No data in this socket, sockfd = %d", sockfd);
        return kNODATA;
      } else if (err == EINTR) {
        LOG_WARN("Reception is interrupted, sockfd = %d", sockfd);
        if (++try_times <=3) {
          continue;
        } else {
          return kNODATA;
        }
      } else {
        LOG_ERROR("recv failed on sockfd=%d", sockfd);
        return kERROR; /* Bad connection, close it please! */
      }
    }
  } while (true);
  
  if(static_cast<size_t>(recv_num) > buf_size) {
    /* buf is full and extra data is stored in backup. */
	buf.MoveWritePos(buf_size);
	buf.Append(backup, static_cast<size_t>(recv_num - buf_size));
  } else {
	buf.MoveWritePos(static_cast<size_t>(recv_num));
  }
  return recv_num;
}

ssize_t Socket::Send(int sockfd, ByteBuffer& buf) {
  const size_t send_size = buf.ReadableBytes();
  ssize_t sent_num = ::write(sockfd, buf.AddrOfRead(), send_size);
  if (sent_num < 0)	{
	int err = errno;
	if (err == EAGAIN || err == EWOULDBLOCK) {
	  LOG_WARN("No space to write, sockfd=%d ", sockfd);
	  return 0;
	} else if (err == EPIPE) {
	  LOG_ERROR("Error on write to sockfd=%d ", sockfd);
	  return kERROR;
	}
  }	else {
	if (sent_num > 0) {
	  buf.MoveReadPos(static_cast<size_t>(sent_num));
    }
  }
  return sent_num;
}

int Socket::Connect(int sockfd, const SocketAddress& server_addr) {
  LOG_CHECK(sockfd >= 0, "");
  int ret = ::connect(sockfd, server_addr.ToAddrTypeVoid(), server_addr.Length());
  if (ret < 0) {
    LOG_ERROR("connect on ip=%s, port=%d, sockfd=%d ", 
              server_addr.ToString().c_str(), server_addr.Port(), sockfd);
	if (errno == EAGAIN) {
	  return kRETRY;
    } else {
      return kERROR;
    }
  }
  return 0;
}

void Socket::Close(int sockfd) {
  LOG_CHECK(sockfd > 0, "");
  int ret = ::close(sockfd);
  if (ret < 0) {
    if (errno == EINTR) {
      ret = ::close(sockfd);
      LOG_CHECK(ret == 0, "");
    } else {
      LOG_FATAL("close sockfd=%d failed", sockfd);
    }
  }
}

int Socket::NewSocket(int domain, int type, int protocol) {
  int sockfd = ::socket(domain, type, protocol);
  if (sockfd < 0) {
    LOG_ERROR("socket() failed on domain=%d, type=%d, protocol=%d ", 
              domain, type, protocol);
    return kERROR;
  }
  return sockfd;
}

void Socket::EnableAddrReuse(int sockfd, int flag) {
  int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, static_cast<void*>(&flag), sizeof(flag));
  LOG_CHECK(ret == 0, "");
}

void Socket::EnablePortReuse(int sockfd, int flag) {
  int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_REUSEPORT, static_cast<void*>(&flag), sizeof(flag));
  LOG_CHECK(ret == 0, "");
}

void Socket::EnableKeepAlive(int sockfd, int flag) {
  int ret = ::setsockopt(sockfd, SOL_SOCKET, SO_KEEPALIVE, static_cast<void*>(&flag), sizeof(flag));
  LOG_CHECK(ret == 0, "");
}

void Socket::EnableTcpNoDelay(int sockfd, int flag) {
  int ret = ::setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, static_cast<void*>(&flag), sizeof(flag));
  LOG_CHECK(ret == 0, "");
}

void Socket::EnableNonBlock(int sockfd, int flag) {
  int val = ::fcntl(sockfd, F_GETFL, 0);
  LOG_CHECK(val > 0, "");
  val = (flag ? (val | O_NONBLOCK) : (val & (~O_NONBLOCK)));
  int ret = ::fcntl(sockfd, F_SETFL, val);
  LOG_CHECK(ret >= 0, "");
}

} // namespace glue_network 



