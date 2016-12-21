#ifndef GLUE_HTTP_RESPONSE_H_
#define GLUE_HTTP_RESPONSE_H_

#include "network/connection.h"
#include "network/buffer.h"
#include "libbase/logger.h"

#include <vector>
#include <string>
#include <memory>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

namespace glue_httpd {
class HttpResponse {
 public:
  typedef std::shared_ptr<glue_network::Connection> ConnectionPtr;
  typedef glue_network::ByteBuffer Buf;
  explicit HttpResponse(const ConnectionPtr& conn): conn_(conn) {
  }
  
  ~HttpResponse() {}

  void SendStatusPage(const std::string& status, const std::string& msg);
  void SendFile(const std::string& path, size_t length);
  void Send(Buf& buf, const std::string& file_type);
 private:
  std::string GetFileType(const std::string& path);
  std::string GetFileName(const std::string& path);
  std::string GetReason(const std::string& status);
  std::string GetMimeType(const std::string& str);
  std::string ToHeader(const std::string& key, const std::string& value);
  std::string ToHeaderPart(const std::string& file_type, size_t length, const std::string& status);
  ConnectionPtr conn_;
};
} // namespace glue_httpd
#endif // GLUE_HTTP_RESPONSE_H_
