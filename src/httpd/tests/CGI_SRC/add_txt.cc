#include <iostream>
#include <vector>
#include <string>

#include "network/buffer.h"
#include "httpd/stringutil.h"

#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>

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
  const char* pos = buf.Find("=");
  if (pos != nullptr) {
    buf.MoveReadPos(pos -  buf.AddrOfRead() + 1);
  }
    
  int file_fd = open("docs/data.txt", O_RDWR | O_CREAT | O_APPEND, S_IRUSR | S_IWUSR);
  if (file_fd < 0) {
    buf.Reset();
    buf.AppendString(FormOutput("Can't open the file"));
    buf.WriteFd(1);  
    ::close(1);
    return 0;
  }
  
  buf.WriteFd(file_fd);
  if (lseek(file_fd, 0, SEEK_SET) < 0)
  {
    buf.Reset();
    buf.AppendString(FormOutput("file seek error"));
    buf.WriteFd(1);  
    ::close(1);
    return 0;  
  }
  buf.ReadFd(file_fd);
  std::string body(FormOutput(buf.toString()));
  buf.Reset();
  buf.AppendString(body);
  buf.WriteFd(1);
  ::close(1);
  return 0;
}


