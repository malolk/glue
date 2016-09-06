#include <httpd/HttpRequest.h>
#include <httpd/HttpResponse.h>
#include <libbase/StringUtil.h>

#include <stdlib.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>

namespace network
{
namespace httpd
{
	const extern std::string basePath = "docs";
	// default timeout value 10s
	const int DEFLT_TIMEOUT = 10;  
}	
}

using namespace libbase;
using namespace network;
using namespace network::httpd;

bool HttpRequest::isAlready(const Buf& buf)
{
	LOGTRACE();
	//LOGINFO(buf.toString());
	const char* crlf = buf.findBytes("\r\n\r\n");
	if (crlf != nullptr)
	{
		const std::string startLine = getOneLine(buf);
		extractMethod(startLine);
		if (method != std::string("POST") && method != std::string("PUT"))
			return true;
		const char* contentLenPtr = buf.findBytes("Content-Length");
		std::string line(contentLenPtr, crlf - contentLenPtr);
		std::string contentLengthStr = getHeader(line).second;

		CHECK(isNumStr(contentLengthStr));
		unsigned long long length = std::stoull(contentLengthStr);

		const char* bodyPtr = crlf + 4;
		if (static_cast<unsigned long long>(buf.addrOfWrite() - bodyPtr) == length)
			return true;
	}
	return false;
}

void HttpRequest::doRequest(Buf& buf)
{
	LOGTRACE();
	std::string startLine = getOneLine(buf);
	extractMethod(startLine);
	buf.movePosOfRead(startLine.size() + 2);
	extractHeader(buf);

	if (method == "POST" || method == "PUT")
		extractBody(buf);
	doMethod();
}

/*implemented: Get, POST */
/*unimplemented: PUT, DELETE */
void HttpRequest::doMethod()
{
	LOGTRACE();
	if (method == "GET")
		methodGet();	
	else if (method == "POST")
		methodPost();
	else
		methodNull();
}

bool HttpRequest::isDirectory(const PathStat& pathStat)
{
	LOGTRACE();
	return S_ISDIR(pathStat.st_mode);
}

bool HttpRequest::isRegular(const PathStat& pathStat)
{
	LOGTRACE();
	return S_ISREG(pathStat.st_mode);
}

bool HttpRequest::isCgi(const PathStat& pathStat)
{
	LOGTRACE();
	if (pathStat.st_mode & S_IXUSR ||
		pathStat.st_mode & S_IXGRP || pathStat.st_mode & S_IXOTH)
		return true;
	return false;
}

bool HttpRequest::getFileInfo(const std::string& path, PathStat& pathStat)
{
	LOGTRACE();
	bzero(&pathStat, sizeof(pathStat));
	int ret = stat(path.c_str(), &pathStat);
	if (ret < 0)
	{
		if (errno == ENOENT)
			handleError("404", "File not found");
		else
			handleError("500", "Server internal error");
		return false;
	}
	return true;
}

void HttpRequest::handleError(const std::string& status, const std::string& msg) const 
{
	LOGTRACE();
	HttpResponse response(conn);
	LOGERROR(errno);
	response.sendStatusPage(status, msg);
}

std::string HttpRequest::getQueryStr(const std::string& urlIn)
{
	LOGTRACE();
	std::string::size_type pos = urlIn.find("?");
	return (pos == std::string::npos ? std::string() : urlIn.substr(pos+1));
}

std::string HttpRequest::getPath(const std::string& urlIn)
{
	LOGTRACE();
	std::string::size_type pos = urlIn.find("?");
	return (pos == std::string::npos ? urlIn : urlIn.substr(0, pos));
}

bool HttpRequest::closeWrapper(int fd)
{
	LOGTRACE();
	if (::close(fd) < 0)
	{
		handleError("500", "Server internal error");
		return false;
	}
	return true;
}

void HttpRequest::doCgi(const std::string& path)
{
	LOGTRACE();
	int cgiIn[2], cgiOut[2];
	pid_t retPid;
	if (pipe(cgiIn) < 0 || pipe(cgiOut) < 0 || (retPid = fork()) < 0)
	{
		handleError("500", "Server internal error");
		return;
	}
	
	if (retPid == 0)
	{
		if (!closeWrapper(cgiIn[1])) return;
		if (!closeWrapper(cgiOut[0])) return;
			 
		if (::dup2(cgiIn[0], 0) < 0 || ::dup2(cgiOut[1], 1) < 0)
		{
			handleError("500", "Server internal error");
			return;
		}

	//	if (!closeWrapper(cgiIn[0])) return;
	//	if (!closeWrapper(cgiOut[1])) return;
#pragma GCC diagnostic ignored "-Wold-style-cast"
		const char* dummy = "dummy";
		if (execl(path.c_str(), dummy, (char *) NULL) < 0)
		{
			handleError("500", "Server internal error");
			return;	
		}
#pragma GCC diagnostic error "-Wold-style-cast"
	}
	else
	{
		if (!closeWrapper(cgiIn[0])) return;
		if (!closeWrapper(cgiOut[1])) return;

		Buf buf;
		if (method == "POST" || method == "PUT")
		{
			buf.appendBytes(body);
			buf.writeFd(cgiIn[1]);
		}
		else
		{
			std::string queryStr;
			queryStr.append(getQueryStr(url));
			buf.appendBytes(queryStr);
			buf.writeFd(cgiIn[1]);
		}
		if (buf.readableBytes() != 0 || closeWrapper(cgiIn[1]) == false)
		{			
			handleError("500", "Server internal error");	
			::kill(retPid, SIGKILL);
			::waitpid(retPid, nullptr, WNOHANG);
			return;
		}
			
		buf.readFd(cgiOut[0]);
		std::string headerLine = getOneLine(buf);
		buf.movePosOfRead(headerLine.size() + 2);
		std::pair<std::string, std::string> header = getHeader(headerLine);
		CHECK(header.first == "Content-type");
		HttpResponse httpResponse(conn);
		httpResponse.send(buf, header.second);

		::waitpid(retPid, nullptr, 0);
	}
}

void HttpRequest::methodGet()
{
	LOGTRACE();
	std::string path(basePath);	
	CHECK(!url.empty() && url[0] == '/');
	path.append(getPath(url));

	if (path.back() == '/')
		path.append("index.html");
	
	HttpResponse response(conn);
	PathStat pathStat;
	if (!getFileInfo(path, pathStat))
		return;
	if (isDirectory(pathStat))
	{
		path.append("/index.html");
		if (!getFileInfo(path, pathStat))
			return;
	}
	
	if (isCgi(pathStat))
		doCgi(path);
	else
		response.sendFile(path, pathStat.st_size);
}

void HttpRequest::methodPost()
{
	LOGTRACE();
	std::string path(basePath);	
	CHECK(!url.empty() && url[0] == '/');
	path.append(url);

	HttpResponse response(conn);
	PathStat pathStat;
	if (!getFileInfo(path, pathStat))
		return;
	if (!isCgi(pathStat))
		response.sendStatusPage("400", "No such cgi program");
	doCgi(path);
}

void HttpRequest::methodNull()
{
	LOGTRACE();
	HttpResponse response(conn);
	response.sendStatusPage("501", "Method unimplemented");
}

void HttpRequest::extractBody(Buf& buf)
{
	LOGTRACE();
	body = buf.toString();
	buf.reset();
	unsigned long long length = std::stoull(headers[std::string("Content-Length")]);
	CHECK(body.size() == length);
}

void HttpRequest::extractHeader(Buf& buf)
{
	LOGTRACE();
	const char* crlf = buf.findBytes("\r\n\r\n");
	while (buf.addrOfRead() < crlf)
	{
		std::string headerLine = getOneLine(buf);
		buf.movePosOfRead(headerLine.size() + 2);
		std::pair<std::string, std::string> header = getHeader(headerLine);
		headers[header.first] = header.second;
	}
	buf.movePosOfRead(2);
}

std::pair<std::string, std::string> HttpRequest::getHeader(const std::string& line)
{
	LOGTRACE();
	std::string::size_type pos = line.find(":");
	CHECK(pos != std::string::npos);
	std::string key = removeSpace(line.substr(0, pos));
	std::string value = removeSpace(line.substr(pos + 1));
	return std::make_pair(key, value);
}

std::string HttpRequest::getOneLine(const Buf& buf)
{
	LOGTRACE();
	const char* lineEnd = buf.findBytes("\r\n");
	if (lineEnd != nullptr)
	{
		std::string line(buf.addrOfRead(), lineEnd - buf.addrOfRead());
		return line;
	}
	
	return std::string();	
}

void HttpRequest::extractMethod(const std::string& line)
{
	LOGTRACE();
	if (!line.empty())
	{
		std::vector<std::string> segments = sliceStrClearly(line, " ");
		CHECK(segments.size() == 3);
		method = segments[0];
		url = segments[1];
		version = segments[2];
	}
}

