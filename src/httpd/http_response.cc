#include "httpd/http_response.h"

namespace glue_httpd {
namespace {
const std::map<std::string, std::string>
code_msg = {
  {"200", "OK"},  
  {"400", "Bad request"},
  {"404", "Not found"},
  {"500", "Internal server error"},
  {"501", "Not implemented"},
  {"505", "HTTP version not support"}
};

const std::map<std::string, std::string>
mime_type = {
  {".htm", 	"text/html"},
  {".html", "text/html"},
  {".doc", 	"application/msword"},
  {".ps", 	"application/postscript"},
  {".ppt", 	"application/powerpoint"},
  {".mpga", "audio/mpeg"},
  {".mp2", 	"audio/mpeg"},
  {".gif", 	"image/gif"},
  {".png",	"image/png"},
  {".jpg", 	"image/jpeg"},
  {".jpe", 	"image/jpeg"},
  {".jpeg", "image/jpeg"},
  {".txt", 	"text/plain"},
  {".mpeg", "video/mpeg"},
  {".mpe", 	"video/mpeg"},
  {".mpg", 	"video/mpeg"},
  {".avi", 	"video/x-msvideo"}
};
}

std::string HttpResponse::ToHeader(const std::string& key, const std::string& value) {
  return (key + ": " + value + "\r\n");
}

std::string HttpResponse::GetReason(const std::string& status) {
  if (code_msg.find(status) != code_msg.cend()) {
  	return std::string(code_msg.at(status));
  }

  LOG_WARN("status code: %s is undefined", status.c_str());
  return std::string();
}

std::string HttpResponse::GetMimeType(const std::string& str) {
  if (mime_type.find(str) != mime_type.cend()) {
  	return std::string(mime_type.at(str));
  }

  LOG_WARN("File type %s is undefined", str.c_str());
  return std::string();
}

void HttpResponse::SendStatusPage(const std::string& status, const std::string& msg) {
  std::string body;
  body.append("<html><title> Server Error</title>");
  body.append(std::string("\n") + status + " " + GetReason(status));
  body.append(std::string("<p>") + msg + "</p>\n");
  body.append("</body></html>");

  std::string header;
  header.append(std::string("HTTP/1.1 ") + status + " " +  GetReason(status) + "\r\n");
  header.append(ToHeader("Server", "httpd"));
  header.append(ToHeader("Content-Type", "text/html"));
  header.append(ToHeader("Connection", "close"));
  header.append(ToHeader("Content-Length", std::to_string(body.size())));
  header.append("\r\n");

  Buf buf((header + body).size());
  buf.AppendString(header + body);
  conn_->Send(buf);
}

std::string HttpResponse::ToHeaderPart(const std::string& file_type, size_t length, const std::string& status) {
  std::string header;
  header.append(std::string("HTTP/1.1 ") + status + " " + GetReason(status) + "\r\n");
  header.append(ToHeader("Server", "httpd"));
  header.append(ToHeader("Content-Type", file_type));
  header.append(ToHeader("Connection", "close"));
  header.append(ToHeader("Content-Length", std::to_string(length)));
  header.append("\r\n");

  return header;
}

void HttpResponse::SendFile(const std::string& path, size_t length) {
  std::string header(ToHeaderPart(GetFileType(path), length, "200"));
  int file_fd = open(path.c_str(), O_RDONLY);
  if (file_fd < 0) {
  	LOG_ERROR("open file error");
  	SendStatusPage("404", GetFileName(path) + "Can't open");
  	return;
  }	

  void* addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, file_fd, 0);
#pragma GCC diagnostic ignored "-Wold-style-cast"
  if (addr == (void *)-1) {
#pragma GCC diagnostic error "-Wold-style-cast"
    LOG_ERROR("mmap error");
  	SendStatusPage("500", "Server error");
  	return;
  }

  ::close(file_fd);
  Buf buf(header.size() + length);
  buf.AppendString(header);
  conn_->Send(buf);
  buf.Reset();

  buf.Append(static_cast<char *>(addr), length);
  conn_->Send(buf);

  int ret = munmap(addr, length);
  if (ret != -1) {
    LOG_ERROR("munmap error");
  }
}

std::string HttpResponse::GetFileType(const std::string& path) {
  std::string::size_type n = path.find(".");
  if (n == std::string::npos) {
    return GetMimeType(".txt");
  }
  std::string type = path.substr(n);
  return GetMimeType(type);
}

std::string HttpResponse::GetFileName(const std::string& path) {
  std::string::size_type n = path.rfind("/");
  if (n == std::string::npos) {
    return path;
  }
  LOG_CHECK(n + 1 < path.size(), "");

  return path.substr(n + 1);
}

/* buf contains the body part of the response*/
/* file_type should be like the ".*" style*/
void HttpResponse::Send(Buf& buf, const std::string& file_type) {
  Buf header_buf;
  std::string mime_type_local = GetFileType(file_type);
  std::string header(ToHeaderPart(mime_type_local, buf.ReadableBytes(), "200"));
  header_buf.AppendString(header);  
  conn_->Send(header_buf);
  conn_->Send(buf);
}
} // namespace glue_httpd
