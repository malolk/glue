#ifndef HTTP_REQUEST_H
#define HTTP_REQUEST_H

#include "../network/Connection.h"
#include "../network/Buffer.h"
#include "../libbase/Debug.h"

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

namespace network
{
namespace httpd
{

class HttpRequest
{
public:
	typedef std::shared_ptr<network::SocketConnection::Connection> ConnectionPtr;
	typedef struct stat PathStat;
	typedef network::ByteBuffer Buf;
	explicit HttpRequest(const ConnectionPtr& connIn): conn(connIn)
	{ }

	~HttpRequest() {}

	bool isAlready(const Buf& buf);
	void doRequest(Buf& buf);

private:
	void doMethod();
	void doCgi(const std::string& path);
	void methodGet();
	void methodPost();
	void methodNull();
	bool getFileInfo(const std::string& path, PathStat& pathStat);
	bool isDirectory(const PathStat& pathStat);
	bool isRegular(const PathStat& pathStat);
	bool isCgi(const PathStat& pathStat);
	
	void extractMethod(const std::string& line);
	void extractHeader(Buf& buf);
	void extractBody(Buf& buf);

	void handleError(const std::string& status, const std::string& msg) const;
	std::string getQueryStr(const std::string& urlIn);
	std::string getPath(const std::string& urlIn);
	bool closeWrapper(int fd);

	std::pair<std::string, std::string> getHeader(const std::string& line);
	std::string getOneLine(const Buf& buf);

	ConnectionPtr conn;	
	std::string method;
	std::string url;
	std::string version;
	std::map<std::string, std::string> headers;
	std::string body;	
};
	
}
}
#endif
