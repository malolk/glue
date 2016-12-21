#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include "../network/Connection.h"
#include "../network/Buffer.h"
#include "../libbase/Debug.h"

#include <vector>
#include <string>
#include <memory>
#include <map>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <unistd.h>
#include <fcntl.h>

namespace network
{
namespace httpd
{
class HttpResponse
{
public:
	typedef std::shared_ptr<network::SocketConnection::Connection> ConnectionPtr;
	typedef network::ByteBuffer Buf;
	explicit HttpResponse(const ConnectionPtr& connIn): conn(connIn)
	{}
	~HttpResponse() {}

	void sendStatusPage(const std::string& status, const std::string& msg);
	void sendFile(const std::string& path, size_t length);
	void send(Buf& buf, const std::string& fileType);
private:
	std::string getFileType(const std::string& path);
	std::string getFileName(const std::string& path);
	std::string getReason(const std::string& status);
	std::string getMimeType(const std::string& str);
	std::string toHeader(const std::string& key, const std::string& value);
	std::string toHeaderPart(const std::string& fileType, size_t length, const std::string& status);
	ConnectionPtr conn;
};
}
}
#endif
