#include "network/buffer.h"
#include "httpd/stringutil.h"

#include <vector>
#include <string>

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

int main() {
  glue_network::ByteBuffer buf;
  buf.ReadFd(0);
  std::vector<std::string> query_string = glue_httpd::SliceStrClearly(buf.ToString(), "=");
  buf.Reset();
  if (query_string.size() != 2) {
    buf.AppendString(FormOutput("Error input!"));
  } else {
    std::string num_str = query_string[1];
    if (!glue_httpd::IsNumStr(num_str)) {
      buf.AppendString(FormOutput(std::string("Error input: ") + num_str));
    } else {
      int num = std::stoi(num_str);
      num = -1 - num;
      buf.AppendString(FormOutput(std::string("Result: -1 - num = ") + std::to_string(num)));
    }
  }
  buf.WriteFd(1);  
  ::close(1);
  return 0;
}


