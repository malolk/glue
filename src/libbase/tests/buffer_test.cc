#include "libbase/buffer.h"
#include "libbase/loggerutil.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include <iostream>
#include <vector>
#include <string>

int main() {
  const char str1[] = "Idiom1: You need to pull up your socks. This means you need to make an effort to improve your work or behaviour because it is not good enough.";
  const char str2[] = "Idiom2: Ball is in your court. This means it is up to you to make the next decision or step.";
  const char str3[] = "Idiom3: It's raining cats and dogs. This means to rain very heavily.";
  const int size1 = sizeof(str1) - 1; // no null-terminator
  const int size2 = sizeof(str2) - 1;
  const int size3 = sizeof(str3) - 1;

  int size = 0;
  const std::string const_str = "1234567890";

  libbase::ByteBuffer buf;
  LOG_CHECK(static_cast<int>(buf.ReadableBytes()) == 0, "intiate readable size is non-zero");
  LOG_CHECK(buf.WritableBytes() > 0, "intiate writable size is not BUF_INIT_CAPACITY");
	
  size += size1;
  buf.Append(str1, size1);
  LOG_CHECK(static_cast<int>(buf.ReadableBytes()) == size, "Buffer Append error");
  LOG_INFO("Buf content: %s Append() [PASS] size = %d", buf.ToString().c_str(), buf.ToString().size());

  size += size2;
  std::vector<char> char_vec(str2, str2 + size2);
  buf.AppendArray(char_vec);
  LOG_CHECK(static_cast<int>(buf.ReadableBytes()) == size, "Buffer AppendArray error");
  LOG_INFO("Buf content: %s AppendArray() [PASS] size = %d", buf.ToString().c_str(), buf.ToString().size());

  size += static_cast<int>(const_str.size());
  buf.AppendString(const_str);
  LOG_CHECK(static_cast<int>(buf.ReadableBytes()) == size, "Buffer AppendString error");
  LOG_INFO("Buf content: %s AppendString() [PASS]", buf.ToString().c_str());

  libbase::ByteBuffer buf2;
  buf2.Append(str3, size3);
  LOG_INFO("Buf2 content: %s ", buf2.ToString().c_str());
  
  size += size3;
  buf.AppendBuffer(buf2);
  LOG_CHECK(static_cast<int>(buf.ReadableBytes()) == size, "");
  LOG_INFO("Buf content: %s AppendBuffer() [PASS]", buf.ToString().c_str());

  size -= size3;
  std::vector<char> char_vec_2 = buf.Read(size3);
  LOG_INFO("Read into vec content: %s ", std::string(&(*char_vec_2.begin()), char_vec_2.size()).c_str());
  LOG_CHECK(static_cast<int>(buf.ReadableBytes()) == size, "");
  LOG_INFO("Buf content: %s Read() [PASS]", buf.ToString().c_str());

  const char* last_pos = buf.FindLast("is");
  LOG_CHECK(last_pos != NULL, "FindLast error");
  LOG_INFO("Substring after found pos: %s", std::string(last_pos, buf.AddrOfWrite() - last_pos).c_str());
  LOG_INFO("Buf content: %s FindLast() [PASS]", buf.ToString().c_str());

  const char* first_pos = buf.Find("is");
  LOG_CHECK(first_pos != NULL, "Find error");
  LOG_INFO("Substring after found pos: %s", std::string(first_pos, buf.AddrOfWrite() - first_pos).c_str());
  LOG_INFO("Buf content: %s Find() [PASS]", buf.ToString().c_str());

  std::string before_str = buf.ToString();
  umask(0);
  int fd = ::open("buffer_test_txt", O_RDWR | O_CREAT, 0666); /* Permission is needed when creating a new file. */
  int write_cnt = buf.WriteFd(fd);
  LOG_CHECK(write_cnt == static_cast<int>(before_str.size()), "WriteFd error");
  LOG_CHECK(static_cast<int>(buf.ReadableBytes()) == 0, "WriteFd error");
  ::lseek(fd, 0, SEEK_SET); // Need this to seek the file offset to start
  int read_cnt = buf.ReadFd(fd);
  close(fd);
  LOG_CHECK(write_cnt == read_cnt, "ReadFd error");
  LOG_INFO("Buf content: %s WriteFd() ReadFd() [PASS]", buf.ToString().c_str());
  
  buf.Reset();
  LOG_CHECK(static_cast<int>(buf.ReadableBytes()) == 0, "");
  LOG_CHECK(buf.AddrOfWrite() == buf.AddrOfRead(), "");
  LOG_INFO("Buf content: %s Reset() [PASS]", buf.ToString().c_str());

  LOG_INFO("===============[PASS]===============");
}

