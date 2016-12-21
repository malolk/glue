#include "HttpResponse.h"

using namespace network;
using namespace network::httpd;

const std::map<std::string, std::string>
code2Msg = {
	{"200", "OK"},	
	{"400", "Bad request"},
	{"404", "Not found"},
	{"500", "Internal server error"},
	{"501", "Not implemented"},
	{"505", "HTTP version not support"}
};

const std::map<std::string, std::string>
mimeType = {
	{".htm", 	"text/html"},
	{".html", 	"text/html"},
	{".doc", 	"application/msword"},
	{".ps", 	"application/postscript"},
	{".ppt", 	"application/powerpoint"},
	{".mpga", 	"audio/mpeg"},
	{".mp2", 	"audio/mpeg"},
	{".gif", 	"image/gif"},
	{".png",	"image/png"},
	{".jpg", 	"image/jpeg"},
	{".jpe", 	"image/jpeg"},
	{".jpeg", 	"image/jpeg"},
	{".txt", 	"text/plain"},
	{".mpeg", 	"video/mpeg"},
	{".mpe", 	"video/mpeg"},
	{".mpg", 	"video/mpeg"},
	{".avi", 	"video/x-msvideo"}
};

std::string HttpResponse::toHeader(const std::string& key, const std::string& value)
{
	return (key + ": " + value + "\r\n");
}

std::string HttpResponse::getReason(const std::string& status)
{
	if (code2Msg.find(status) != code2Msg.cend())
		return std::string(code2Msg.at(status));

	LOGWARN(std::string("status code: ") + status + " is undefined");
	return std::string();
}

std::string HttpResponse::getMimeType(const std::string& str)
{
	if (mimeType.find(str) != mimeType.cend())
		return std::string(mimeType.at(str));

	LOGWARN(std::string("File type ") + str + " is undefined");
	return std::string();
}

void HttpResponse::sendStatusPage(const std::string& status, const std::string& msg)
{
	LOGTRACE();
	std::string body;
	body.append("<html><title> Server Error</title>");
	//body.append("<body bgcolor= ""ffffff"">\n");
	body.append(std::string("\n") + status + " " + getReason(status));
	body.append(std::string("<p>") + msg + "</p>\n");
	body.append("</body></html>");

	std::string header;
	header.append(std::string("HTTP/1.1 ") + status + " " +  getReason(status) + "\r\n");
	header.append(toHeader("Server", "httpd"));
	header.append(toHeader("Content-Type", "text/html"));
	header.append(toHeader("Connection", "close"));
	header.append(toHeader("Content-Length", std::to_string(body.size())));
	header.append("\r\n");

	Buf buf((header + body).size());
	buf.appendBytes(header + body);
	conn->sendData(buf);
	LOGTRACE();
}

std::string HttpResponse::toHeaderPart(const std::string& fileType, size_t length, const std::string& status)
{
	std::string header;
	header.append(std::string("HTTP/1.1 ") + status + " " + getReason(status) + "\r\n");
	header.append(toHeader("Server", "httpd"));
	header.append(toHeader("Content-Type", fileType));
	header.append(toHeader("Connection", "close"));
	header.append(toHeader("Content-Length", std::to_string(length)));
	header.append("\r\n");

	return header;
}

void HttpResponse::sendFile(const std::string& path, size_t length)
{
	LOGTRACE();
	std::string header(toHeaderPart(getFileType(path), length, "200"));

	int fileFd = open(path.c_str(), O_RDONLY);
	CHECKX(fileFd >= 0, path + " open failed");
	if (fileFd < 0)
	{
		LOGERROR(errno);
		sendStatusPage("404", getFileName(path) + "Can't open");
		return;
	}	

	void* addr = mmap(NULL, length, PROT_READ, MAP_PRIVATE, fileFd, 0);
#pragma GCC diagnostic ignored "-Wold-style-cast"
	CHECKX(addr != (void *)-1, "mmap error");
	if (addr == (void *)-1)
#pragma GCC diagnostic error "-Wold-style-cast"
	{
		LOGERROR(errno);
		sendStatusPage("500", "Server error");
		return;
	}

	::close(fileFd);
	Buf buf(header.size() + length);
	buf.appendBytes(header);
	conn->sendData(buf);
	buf.reset();

	buf.appendBytes(static_cast<char *>(addr), length);
	conn->sendData(buf);

	int ret = munmap(addr, length);
	CHECKX(ret != -1, "munmap error");
}

std::string HttpResponse::getFileType(const std::string& path)
{
	std::string::size_type n = path.find(".");
	if (n == std::string::npos)
		return getMimeType(".txt");
	std::string type = path.substr(n);
	return getMimeType(type);
}

std::string HttpResponse::getFileName(const std::string& path)
{
	std::string::size_type n = path.rfind("/");
	if (n == std::string::npos)
		return path;
	CHECK(n + 1 < path.size());

	return path.substr(n + 1);
}

/* buf contains the body part of the response*/
/* fileType should be like the ".*" style*/
void HttpResponse::send(Buf& buf, const std::string& fileType)
{
	LOGTRACE();
	Buf bufHeader;
	std::string mimeTypeLocal = getFileType(fileType);
	std::string header(toHeaderPart(mimeTypeLocal, buf.readableBytes(), "200"));
	bufHeader.appendBytes(header);	
	conn->sendData(bufHeader);
	conn->sendData(buf);
}

