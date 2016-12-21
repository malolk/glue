#include "httpd/http_server.h"
#include "httpd/http_request.h"
#include "httpd/http_response.h"

#include <functional>

namespace glue_httpd {
void HttpServer::CallbackOnRequest(ConnectionPtr conn, glue_network::ByteBuffer& buf) {
  HttpRequest http_req(conn);
  if (http_req.IsAlready(buf)) {
	http_req.DoRequest(buf);
	conn->Shutdown();
  }
}

void HttpServer::Start() {
  server_.Run();
}
} // namespace glue_httpd
