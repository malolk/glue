#include <iostream>
#include <vector>
#include <string>
#include <exception>
#include <typeinfo>

#include "network/buffer.h"
#include "httpd/stringutil.h"

#include <unistd.h>

std::string FormOutput(std::string body) {
  std::string output("Content-type: .html\r\n");
  output.append("<html>\n<head>\n<title> Welcome! </title></head>\n");
  output.append("<style>\n body \n{ width : 100em; margin: 0 auto; }\n</style>\n");
  output.append("<body>\n<h2> Welcome to MiniServer! </h2>\n");
  output.append("<p>");
  output.append(body);
  output.append("</p>\n</body>\n</html>\n");

  return output;
}

std::string GetArg(const std::string& str) {
  std::vector<std::string> arg_str = SliceStrClearly(str, "=");
  if (arg_str.size() != 2) {
    return std::string();
  }
  return arg_str[1];
}

int main() {
  glue_network::ByteBuffer buf;
  buf.ReadFd(0);
  std::vector<std::string> query_string = SliceStrClearly(buf.ToString(), "&");
  buf.Reset();
  if (query_string.size() != 2) {
    buf.AppendString(FormOutput("Error input!"));
  } else {
    std::string arg0 = GetArg(query_string[0]);
    std::string arg1 = GetArg(query_string[1]);
    if (arg0.empty() || arg1.empty() || 
      !IsNumStr(arg0) || !IsNumStr(arg1)) {
      buf.AppendString(FormOutput(std::string("Error input: ") + arg0 + " " + arg1));  
    } else {
      try {
        long long result_long =  std::stoi(arg0) * std::stoi(arg1);
        int32_t result_int = std::stoi(arg0) * std::stoi(arg1);
        if (result_long != result_int) {
          buf.AppendString(FormOutput(std::string("multiplication result overflow") + arg0 + " " + arg1));
        } else {  
          buf.AppendString(FormOutput(std::string("Result: ") + arg0 + " * " 
                           + arg1 + " = " + std::to_string(result_int)));
        }
      } catch(std::exception &e) {
        buf.AppendString(FormOutput(std::string("Error input: ") + arg0 + " " + arg1));  
        buf.AppendString(std::string("\nCause: ") + e.what() + std::string(" ") + typeid(e).name());
      }
    }
  }
  buf.WriteFd(1);  
  ::close(1);
  return 0;
}
