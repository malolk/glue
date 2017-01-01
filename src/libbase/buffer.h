#ifndef GLUE_NETWORK_BYTEBUFFER_H_
#define GLUE_NETWORK_BYTEBUFFER_H_

#include "libbase/thread.h"

#include <vector>
#include <string>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

namespace glue_libbase {
/* Copable, default copy constructor, assignment operator and destructor is ok */
class ByteBuffer {
 public:
  explicit ByteBuffer(size_t capacity = default_capacity_)
    : buf_id_(glue_libbase::ThreadId()), read_pos_(0), write_pos_(0) {
    assert(capacity > 0);
    buf_.resize(capacity, 0);
  }
  ~ByteBuffer() {
  }

  const char* Find(const std::string& str) const {
    return Find(str, AddrOfRead());
  }

  const char* FindLast(const std::string& str) const {
    return FindLast(str, AddrOfRead() + write_pos_ - read_pos_);
  }

  void AppendArray(const std::vector<char>& vec) {
    Append(&(*vec.cbegin()), vec.size());
  }

  void AppendString(const std::string& str) {
    Append(str.c_str(), str.size());
  }

  void AppendBuffer(const ByteBuffer& buf_in) {
    Append(buf_in.AddrOfRead(), buf_in.ReadableBytes());
  }
	
  size_t ReadableBytes() const {
    assert(write_pos_ >= read_pos_);
    return (write_pos_ - read_pos_);
  }

  size_t WritableBytes() const {
    assert(write_pos_ <= buf_.size());
    return (buf_.size() - write_pos_);
  }

  std::string ToString() const {
    return std::string(AddrOfRead(), ReadableBytes());
  }

  void Reset() {
    read_pos_ = 0;
    write_pos_ = 0;
  }

  size_t Capacity() const { 
    return buf_.size(); 
  }

  char* AddrOfWrite() { 
    return &*(buf_.begin() + write_pos_); 
  }

  const char* AddrOfWrite() const { 
    return &*(buf_.begin() + write_pos_); 
  }

  const char* AddrOfRead() const { 
    return &*(buf_.begin() + read_pos_); 
  }

  void MoveWritePos(size_t size) {
    assert(size <= WritableBytes());
    write_pos_ += size;
  }

  int ReadFd(int fd);
  int WriteFd(int fd);
  void MoveReadPos(size_t size);
  void Append(const char *, size_t );
  const char* FindLast(const std::string&, const char*) const;
  const char* Find(const std::string&, const char*) const;
  std::vector<char> Read(size_t );
  int SpareSpace(size_t size);

 private:
  size_t ReadPos() const { return read_pos_; }
  size_t WritePos() const { return write_pos_; }

  std::vector<char> buf_;
  const pid_t buf_id_;
  size_t read_pos_;
  size_t write_pos_;
  static const size_t default_capacity_;
};
} // namespace glue_libbase
#endif // GLUE_NETWORK_BYTEBUFFER_H_
