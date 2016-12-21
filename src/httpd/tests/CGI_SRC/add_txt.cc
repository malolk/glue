#include <iostream>
#include <vector>
#include <string>

#include "../network/Buffer.h"
#include "../libbase/StringUtil.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

using namespace libbase;
typedef network::ByteBuffer Buf;

std::string formOutput(std::string body)
{
	std::string output("Content-type: .html\r\n");
	output.append("<html>\n<head>\n<title> Welcome! </title></head>\n");
	output.append("<style>\n body \n{ width : 100em; margin: 0 auto; }\n</style>\n");
	output.append("<body>\n<h2> Welcome to MiniServer! </h2>\n");
	output.append("<p>");
	output.append(body);
	output.append("</p>\n</body>\n</html>\n");

	return output;
}

int main()
{
	Buf buf;
	buf.readFd(0);
	const char* pos = buf.findBytes("=");
	if (pos != nullptr)
	{
		buf.movePosOfRead(pos -  buf.addrOfRead() + 1);
	}
		
	int fileFd = open("docs/data.txt", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
	if (fileFd < 0)
	{
		buf.reset();
		buf.appendBytes(formOutput("Can't open the file"));
		buf.writeFd(1);	
		close(1);
		return 0;
	}
	
	buf.writeFd(fileFd);
	if (lseek(fileFd, 0, SEEK_SET) < 0)
	{
		buf.reset();
		buf.appendBytes(formOutput("file seek error"));
		buf.writeFd(1);	
		close(1);
		return 0;	
	}
	buf.readFd(fileFd);
	std::string body(formOutput(buf.toString()));
	buf.reset();
	buf.appendBytes(body);
	buf.writeFd(1);
	close(1);
	return 0;
}


