#include "libbase/buffer.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <algorithm>

namespace glue_libbase {
const size_t ByteBuffer::default_capacity_  = 1024;

/* Find last substr that equal to given str, the return pos shoud sit before end */
const char* ByteBuffer::FindLast(const std::string& str, const char* end) const {
  assert(str.size() > 0 && end > AddrOfRead() && end <= AddrOfWrite());
  /* TODO: No temporary string please. */
  std::string::size_type pos = ToString().rfind(str, end - AddrOfRead());
  if (pos != std::string::npos) {
    return AddrOfRead() + pos;
  }
  return nullptr;
}

/* Find first substr that equal to given str, the return pos shoud sit after start */
const char* ByteBuffer::Find(const std::string& str, const char* start) const {
  assert(str.size() > 0 && start >= AddrOfRead() && start < AddrOfWrite());
  std::string::size_type pos = ToString().find(str, start - AddrOfRead());
  if (pos != std::string::npos) {
    return AddrOfRead() + pos;
  }
  return nullptr;
}

int ByteBuffer::ReadFd(int fd) {
  assert(fd >= 0);
  while (1) {
    ssize_t num = ::read(fd, AddrOfWrite(), WritableBytes());
	if (num == 0) {
      return static_cast<int>(ReadableBytes());
    } else if (num > 0) {
      MoveWritePos(num);
	  if (WritableBytes() == 0) {
		buf_.resize(buf_.size() * 2);
	  }
	} else {
      fprintf(stderr, "Read from fd %d failed", fd);
	  return -1;
	}
  }
}

int ByteBuffer::WriteFd(int fd) {
  assert(fd >= 0);
  const size_t write_cnt = ReadableBytes();
  while (ReadableBytes() > 0) {
    ssize_t num = ::write(fd, AddrOfRead(), ReadableBytes());
	if (num < 0) {
      fprintf(stderr, "Write to fd %d failed", fd);
	  return -1;
	}
	MoveReadPos(static_cast<size_t>(num));
  }
  Reset();
  return static_cast<int>(write_cnt);
}

std::vector<char> ByteBuffer::Read(size_t n) {
  const size_t remaining = ReadableBytes();
  size_t read_num = (n <= remaining ? n : remaining);
  std::vector<char> ret(buf_.begin() + read_pos_, buf_.begin() + read_pos_ + read_num);
  MoveReadPos(read_num);
  return ret;
}

void ByteBuffer::Append(const char* start, size_t size) {
  if (size > WritableBytes()) {
    SpareSpace(size);
  }
  assert((buf_.size() - write_pos_) >= size);
  std::copy(start, start + size, AddrOfWrite());
  MoveWritePos(size);
}

int ByteBuffer::SpareSpace(size_t size) {
  if (size <= (buf_.size() - write_pos_ + read_pos_)) {
    std::copy(buf_.begin() + read_pos_, 
              buf_.begin() + write_pos_, buf_.begin());
    write_pos_ = write_pos_ - read_pos_;
    read_pos_ = 0;
  } else {
    buf_.resize(write_pos_ + size);
    if (buf_.size() != (write_pos_ + size)) {
      fprintf(stderr, "SpareSpace failed with requested size=%d", static_cast<int>(size));
      return 0;
    }
  }
  return 1;
}

void ByteBuffer::MoveReadPos(size_t size) {
  assert(size <= ReadableBytes());
  read_pos_ += size;
  if (read_pos_ == write_pos_) {
    read_pos_ = write_pos_ = 0;
  }
}

} // namespace glue_libbase
