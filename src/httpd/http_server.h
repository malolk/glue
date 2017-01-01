#ifndef GLUE_HTTP_SERVER_H_
#define GLUE_HTTP_SERVER_H_

#include "network/tcp_server.h"
#include "network/connection.h"
#include "network/socket_address.h"
#include "libbase/buffer.h"
#include "libbase/noncopyable.h"

#include <string>
#include <memory>

namespace glue_httpd {
class HttpServer: private glue_libbase::Noncopyable {
 public:
  typedef std::shared_ptr<glue_network::Connection> ConnectionPtr;
  HttpServer(const glue_network::SocketAddress& server_addr, int thread_num)
    : server_(server_addr, thread_num, CallbackOnRequest) {
  }

  void Start();
private:
  static void CallbackOnRequest(ConnectionPtr conn, glue_libbase::ByteBuffer& buf);
  glue_network::TcpServer server_;
};
} // namespace glue_httpd
#endif // GLUE_HTTP_SERVER_H_
