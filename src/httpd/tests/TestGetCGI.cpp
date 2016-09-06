#include <network/Buffer.h>
#include <libbase/StringUtil.h>

#include <iostream>
#include <vector>
#include <string>

#include <unistd.h>

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
	std::vector<std::string> queryString = sliceStrClearly(buf.toString(), "=");
	buf.reset();
	if (queryString.size() != 2)
	{
		buf.appendBytes(formOutput("Error input!"));
	}
	else
	{
		std::string numStr = queryString[1];
		if (!isNumStr(numStr))
			buf.appendBytes(formOutput(std::string("Error input: ") + numStr));
		else
		{
			int num = std::stoi(numStr);
			num = -1 - num;
			buf.appendBytes(formOutput(std::string("Result: -1 - num = ") + std::to_string(num)));
		}
	}
	buf.writeFd(1);	
	close(1);
	return 0;
}


