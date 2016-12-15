#include "buffer.h"

#include <unistd.h>
#include <errno.h>
#include <string.h>

#include <algorithm>

namespace glue_network {
const size_t ByteBuffer::default_capacity_  = 1024;

/* Find last substr that equal to given str, the return pos shoud sit before end */
const char* ByteBuffer::FindLast(const std::string& str, const char* end) const {
  LOG_CHECK(str.size() > 0 && end > AddrOfRead() && end <= AddrOfWrite(), 
            "Invalid arguments");
  /* TODO: No temporary string please. */
  std::string::size_type pos = ToString().rfind(str, end - AddrOfRead());
  if (pos != std::string::npos) {
    return AddrOfRead() + pos;
  }
  return nullptr;
}

/* Find first substr that equal to given str, the return pos shoud sit after start */
const char* ByteBuffer::Find(const std::string& str, const char* start) const {
  LOG_CHECK(str.size() > 0 && start >= AddrOfRead() && start < AddrOfWrite(), 
            "Invalide arguments");
  std::string::size_type pos = ToString().find(str, start - AddrOfRead());
  if (pos != std::string::npos) {
    return AddrOfRead() + pos;
  }
  return nullptr;
}

int ByteBuffer::ReadFd(int fd) {
  LOG_CHECK(fd >= 0, "fd should be non-negative");
  while (1) {
    ssize_t num = ::read(fd, AddrOfWrite(), WritableBytes());
	if (num == 0) {
      return static_cast<int>(ReadableBytes());
    } else if (num > 0) {
      MoveWritePos(num);
	  if (WritableBytes() == 0) {
		buf_.resize(capacity_ * 2);
        capacity_ *= 2;
	  }
	} else {
      LOG_ERROR("Read from fd %d failed", fd);
	  return -1;
	}
  }
}

int ByteBuffer::WriteFd(int fd) {
  LOG_CHECK(fd >= 0, "fd should be non-negative");
  const size_t write_cnt = ReadableBytes();
  while (ReadableBytes() > 0) {
    ssize_t num = ::write(fd, AddrOfRead(), ReadableBytes());
	if (num < 0) {
	  LOG_ERROR("Write to fd %d failed", fd);
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
  LOG_CHECK((read_num <= remaining), "No enough byte to be read");
  std::vector<char> ret(buf_.begin() + read_pos_, buf_.begin() + read_num);
  MoveReadPos(read_num);
  return ret;
}

void ByteBuffer::Append(const char* start, size_t size) {
  if (size > WritableBytes()) {
    SpareSpace(size);
  }
  LOG_CHECK((buf_.size() - write_pos_) >= size, "");
  std::copy(start, start + size, AddrOfWrite());
  write_pos_ += size;
}

void ByteBuffer::SpareSpace(size_t size) {
  if (size <= (capacity_ - write_pos_ + read_pos_)) {
    std::copy(buf_.begin() + read_pos_, 
              buf_.begin() + write_pos_, buf_.begin());
    write_pos_ = write_pos_ - read_pos_;
    read_pos_ = 0;
  } else {
    buf_.resize(write_pos_ + size);
    capacity_ = write_pos_ + size;
  }
  LOG_CHECK(buf_.size() == capacity_, "");
}

void ByteBuffer::MoveReadPos(size_t size) {
  if (size > ReadableBytes()) {
    LOG_CHECK(size <= ReadableBytes(), "");
	size = ReadableBytes();
  }
  read_pos_ += size;
  if (read_pos_ == write_pos_) {
    read_pos_ = write_pos_ = 0;
  }
}

} // namespace glue_netowrk
