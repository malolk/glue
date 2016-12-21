#include "httpd/http_request.h"
#include "httpd/http_response.h"
#include "httpd/stringutil.h"

#include <signal.h>    // for kill()
#include <sys/wait.h>  // for wait()
#include <sys/stat.h>  // for stat()
#include <unistd.h> 

namespace glue_httpd {
namespace {
  const extern std::string base_path = "docs";
}	

bool HttpRequest::IsAlready(const Buf& buf) {
  const char* crlf = buf.Find("\r\n\r\n");
  if (crlf != nullptr) {
    const std::string start_line = GetOneLine(buf);
	ExtractMethod(start_line);
	if (method_ != std::string("POST") && method_ != std::string("PUT")) {
	  return true;
    }
	const char* content_len_ptr = buf.Find("Content-Length");
	std::string line(content_len_ptr, crlf - content_len_ptr);
	std::string content_len_str = GetHeader(line).second;

	LOG_CHECK(IsNumStr(content_len_str), "");
	unsigned long long length = std::stoull(content_len_str);

	const char* body__ptr = crlf + 4;
	if (static_cast<unsigned long long>(buf.AddrOfWrite() - body__ptr) == length) {
	  return true;
    }
  }
  return false;
}

void HttpRequest::DoRequest(Buf& buf) {
  std::string start_line = GetOneLine(buf);
  ExtractMethod(start_line);
  buf.MoveReadPos(start_line.size() + 2);
  ExtractHeader(buf);

  if (method_ == "POST" || method_ == "PUT") {
    ExtractBody(buf);
  }
  DoMethod();
}

void HttpRequest::DoMethod() {
  if (method_ == "GET") {
	GetMethod();	
  } else if (method_ == "POST") {
	PostMethod();
  } else {
	NullMethod();
  }
}

bool HttpRequest::IsDirectory(const PathStat& path_stat) {
  return S_ISDIR(path_stat.st_mode);
}

bool HttpRequest::IsRegular(const PathStat& path_stat) {
  return S_ISREG(path_stat.st_mode);
}

bool HttpRequest::IsCgi(const PathStat& path_stat) {
  if (path_stat.st_mode & S_IXUSR ||
	  path_stat.st_mode & S_IXGRP || 
      path_stat.st_mode & S_IXOTH) {
    return true;
  } else {
	return false;
  }
}

bool HttpRequest::GetFileInfo(const std::string& path, PathStat& path_stat) {
  bzero(&path_stat, sizeof(path_stat));
  int ret = stat(path.c_str(), &path_stat);
  if (ret < 0) {
	if (errno == ENOENT) {
	  HandleError("404", "File not found");
    } else {
	  HandleError("500", "Server internal error");
    }
	return false;
  }
  return true;
}

void HttpRequest::HandleError(const std::string& status, const std::string& msg) const {
  HttpResponse response(conn_);
  response.SendStatusPage(status, msg);
}

std::string HttpRequest::GetQueryStr(const std::string& url) {
  std::string::size_type pos = url.find("?");
  return (pos == std::string::npos ? std::string() : url.substr(pos+1));
}

std::string HttpRequest::GetPath(const std::string& url) {
  std::string::size_type pos = url.find("?");
  return (pos == std::string::npos ? url : url.substr(0, pos));
}

bool HttpRequest::CloseWrapper(int fd) {
  if (::close(fd) < 0) {
    HandleError("500", "Server internal error");
	return false;
  } else {
	return true;
  }
}

void HttpRequest::DoCgi(const std::string& path) {
  int cgi_in[2], cgi_out[2];
  pid_t ret_pid;
  if (pipe(cgi_in) < 0 || pipe(cgi_out) < 0 || (ret_pid = fork()) < 0) {
    HandleError("500", "Server internal error");
	return;
  }
	
  if (ret_pid == 0) {
    if (!CloseWrapper(cgi_in[1])) {
        return;
    }
	if (!CloseWrapper(cgi_out[0])) {
        return;
    }
			 
	if (::dup2(cgi_in[0], 0) < 0 || ::dup2(cgi_out[1], 1) < 0) {
	  HandleError("500", "Server internal error");
	  return;
	}

#pragma GCC diagnostic ignored "-Wold-style-cast"
    const char* dummy = "dummy";
	if (execl(path.c_str(), dummy, (char *) NULL) < 0) {
	  HandleError("500", "Server internal error");
	  return;	
	}
#pragma GCC diagnostic error "-Wold-style-cast"
  } else {
	if (!CloseWrapper(cgi_in[0])) return;
	if (!CloseWrapper(cgi_out[1])) return;

	Buf buf;
	if (method_ == "POST" || method_ == "PUT") {
	  buf.AppendString(body_);
	  buf.WriteFd(cgi_in[1]);
	} else {
	  std::string query_str;
	  query_str.append(GetQueryStr(url_));
	  buf.AppendString(query_str);
	  buf.WriteFd(cgi_in[1]);
	}
		
    if (buf.ReadableBytes() != 0 || CloseWrapper(cgi_in[1]) == false) {			
	  HandleError("500", "Server internal error");	
	  ::kill(ret_pid, SIGKILL);
	  ::waitpid(ret_pid, nullptr, WNOHANG);
	  return;
	}
			
	buf.ReadFd(cgi_out[0]);
	std::string header_line = GetOneLine(buf);
	buf.MoveReadPos(header_line.size() + 2);
	std::pair<std::string, std::string> header = GetHeader(header_line);
	LOG_CHECK(header.first == "Content-type", "");
	HttpResponse http_response(conn_);
	http_response.Send(buf, header.second);

	::waitpid(ret_pid, nullptr, 0);
  }
}

void HttpRequest::GetMethod() {
  std::string path(base_path);	
  LOG_CHECK(!url_.empty() && url_[0] == '/', "");
  path.append(GetPath(url_));

  if (path.back() == '/') {
    path.append("index.html");
  }
	
  HttpResponse response(conn_);
  PathStat path_stat;
  if (!GetFileInfo(path, path_stat)) {
    return;
  }
  if (IsDirectory(path_stat)) {
    path.append("/index.html");
	if (!GetFileInfo(path, path_stat)) {
	  return;
    }
  }
	
  if (IsCgi(path_stat)) {
    DoCgi(path);
  } else {
	response.SendFile(path, path_stat.st_size);
  }
}

void HttpRequest::PostMethod() {
  std::string path(base_path);	
  LOG_CHECK(!url_.empty() && url_[0] == '/', "");
  path.append(url_);

  HttpResponse response(conn_);
  PathStat path_stat;
  if (!GetFileInfo(path, path_stat)) {
    return;
  }
  if (!IsCgi(path_stat)) {
    response.SendStatusPage("400", "No such cgi program");
  }
  DoCgi(path);
}

void HttpRequest::NullMethod() {
  HttpResponse response(conn_);
  response.SendStatusPage("501", "Method unimplemented");
}

void HttpRequest::ExtractBody(Buf& buf) {
  body_ = buf.ToString();
  buf.Reset();
  unsigned long long length = std::stoull(headers_[std::string("Content-Length")]);
  LOG_CHECK(body_.size() == length, "");
}

void HttpRequest::ExtractHeader(Buf& buf) {
  const char* crlf = buf.Find("\r\n\r\n");
  while (buf.AddrOfRead() < crlf) {
    std::string header_line = GetOneLine(buf);
    buf.MoveReadPos(header_line.size() + 2);
  	std::pair<std::string, std::string> header = GetHeader(header_line);
  	headers_[header.first] = header.second;
  }
  buf.MoveReadPos(2);
}

std::pair<std::string, std::string> HttpRequest::GetHeader(const std::string& line) {
  std::string::size_type pos = line.find(":");
  LOG_CHECK(pos != std::string::npos, "");
  std::string key = RemoveSpace(line.substr(0, pos));
  std::string value = RemoveSpace(line.substr(pos + 1));
  return std::make_pair(key, value);
}

std::string HttpRequest::GetOneLine(const Buf& buf) {
  const char* line_end = buf.Find("\r\n");
  if (line_end != nullptr) {
  	std::string line(buf.AddrOfRead(), line_end - buf.AddrOfRead());
  	return line;
  }	
  return std::string();	
}

void HttpRequest::ExtractMethod(const std::string& line) {
  if (!line.empty()) {
  	std::vector<std::string> segments = SliceStrClearly(line, " ");
  	LOG_CHECK(segments.size() == 3, "");
  	method_ = segments[0];
  	url_ = segments[1];
  	version_ = segments[2];
  }
}
} // namespace glue_httpd

