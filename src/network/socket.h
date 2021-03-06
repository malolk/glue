#ifndef GLUE_NETWORK_SOCKET_H_
#define GLUE_NETWORK_SOCKET_H_

#include "network/socket_address.h"
#include "libbase/buffer.h"
#include "libbase/loggerutil.h"

#include <string>

#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

namespace network {
class Socket {
 public:
  enum {
    kERROR = -1,
    kNODATA = -2,
    kRETRY = -3,
    kTIMEOUT = -4
  };

  static void StopWrite(int sockfd); 
  static void StopRead(int sockfd);

  static ssize_t Send(int sockfd, libbase::ByteBuffer& buf);
  static ssize_t Receive(int sockfd, libbase::ByteBuffer& buf);

  static void GetPeerName(int sockfd, SocketAddress &sa);
  static void GetSockName(int sockfd, SocketAddress &sa);

  static int Accept(int sockfd, SocketAddress *sa);
  static int Connect(int sockfd, const SocketAddress& addr);

  static void Close(int sockfd);
  static int NewSocket(int domain = AF_INET, int type = SOCK_STREAM, int protocol = IPPROTO_IP);
  static void BindAndListen(int sockfd, const SocketAddress& addr, int backlog);

  static void EnableAddrReuse(int sockfd, int flag);
  static void EnablePortReuse(int sockfd, int flag);
  static void EnableKeepAlive(int sockfd, int flag);
  static void EnableTcpNoDelay(int sockfd, int flag);
  static void EnableNonBlock(int sockfd, int flag);
};
} // namespace network
#endif // GLUE_NETWORK_SOCKET_H_
