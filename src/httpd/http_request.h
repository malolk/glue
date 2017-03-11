#ifndef GLUE_HTTP_REQUEST_H_
#define GLUE_HTTP_REQUEST_H_

#include "network/connection.h"
#include "libbase/buffer.h"
#include "libbase/loggerutil.h"

#include <vector>
#include <string>
#include <algorithm>
#include <utility>
#include <cctype>
#include <memory>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>

namespace httpd {
class HttpRequest {
 public:
  typedef std::shared_ptr<network::Connection> ConnectionPtr;
  typedef struct stat PathStat;
  typedef libbase::ByteBuffer Buf;
  explicit HttpRequest(const ConnectionPtr& conn): conn_(conn) { 
  }

  ~HttpRequest() {
  }

  bool IsAlready(const Buf& buf);
  void DoRequest(Buf& buf);

 private:
  void DoMethod();
  void DoCgi(const std::string& path);
  void GetMethod();
  void PostMethod();
  void NullMethod();
  bool GetFileInfo(const std::string& path, PathStat& path_stat);
  bool IsDirectory(const PathStat& path_stat);
  bool IsRegular(const PathStat& path_stat);
  bool IsCgi(const PathStat& path_stat);
	
  void ExtractMethod(const std::string& line);
  void ExtractHeader(Buf& buf);
  void ExtractBody(Buf& buf);

  void HandleError(const std::string& status, const std::string& msg) const;
  std::string GetQueryStr(const std::string& url);
  std::string GetPath(const std::string& url);
  bool CloseWrapper(int fd);

  std::pair<std::string, std::string> GetHeader(const std::string& line);
  std::string GetOneLine(const Buf& buf);

  ConnectionPtr conn_;	
  std::string method_;
  std::string url_;
  std::string version_;
  std::map<std::string, std::string> headers_;
  std::string body_;	
};
} // namespace httpd
#endif // GLUE_HTTP_REQUEST_H_
