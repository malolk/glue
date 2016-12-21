#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <typeinfo>

#include "../network/Buffer.h"
#include "../libbase/StringUtil.h"

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

std::string getArg(const std::string& str)
{
	std::vector<std::string> argStr = sliceStrClearly(str, "=");
	if (argStr.size() != 2)
		return std::string();
	return argStr[1];
}

int main()
{
	Buf buf;
	buf.readFd(0);
	std::vector<std::string> queryString = sliceStrClearly(buf.toString(), "&");
	buf.reset();
	if (queryString.size() != 2)
	{
		buf.appendBytes(formOutput("Error input!"));
	}
	else
	{
		std::string arg0 = getArg(queryString[0]);
		std::string arg1 = getArg(queryString[1]);
		if (arg0.empty() || arg1.empty() || 
			!isNumStr(arg0) || !isNumStr(arg1))
			buf.appendBytes(formOutput(std::string("Error input: ") + arg0 + " " + arg1));	
		else
		{
			try
			{
				long long resultLong =  std::stoi(arg0) * std::stoi(arg1);
				int32_t resultInt = std::stoi(arg0) * std::stoi(arg1);
				if (resultLong != resultInt)
					buf.appendBytes(formOutput(std::string("multiplication result overflow") + arg0 + " " + arg1));
				else	
					buf.appendBytes(formOutput(std::string("Result: ") + arg0 + " * " 
				+ arg1 + " = " + std::to_string(resultInt)));
			}
			catch(std::exception &e)
			{
				buf.appendBytes(formOutput(std::string("Error input: ") + arg0 + " " + arg1));	
				buf.appendBytes(std::string("\nCause: ") + e.what() + std::string(" ") + typeid(e).name());
			}
		}
	}
	buf.writeFd(1);	
	close(1);
	return 0;
}


