#include "libbase/logger.h"
#include "libbase/timeutil.h"
#include "libbase/thread.h"

#include <string>

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

namespace glue_libbase {

namespace {
const char config_path[] = "logger.conf";
/* Log formate like this: <[LEVEL]> <TIME> <FILE> <LINE> <FUNC> <THREADNUM> <MSG> */
const char* default_format = "[%-5s] %s %s:%-4d %s() @Thread[%5d] %s\n";
const char* level_str[] = {"", "TRACE", "INFO", "WARN", "ERROR", "FATAL"};
}
Logger* Logger::logger_ = new Logger();
Logger::Helper Logger::helper_;

/* TODO: Read the buf settings from the configure file. */
#define LOGGER_FLUSH_PERIOD  (3)   /* Flush the logger buffer to the disk at most 3 seconds. */
#define LOGGER_MIN_BUF_NUM   (3)   
#define LOGGER_MAX_BUF_NUM   (10)  /* Limit the memory space occupied by the logger. */
#define LOGGER_BUF_SIZE      (4 * 1024 * 1024) /* The size of each buffer. */

Logger::Logger() 
  : level_(kWARN), file_(NULL), mu_(), 
    condvar_(mu_), thread_("logger"), running_(false), started_(false), current_day_(0), 
    buf_size_(LOGGER_BUF_SIZE), max_buf_num_(LOGGER_MAX_BUF_NUM) {
  Initialize();
}

Logger::~Logger() {
  if (started_) {
    StopBackgroundThread();
  }
  if (file_) {
    fclose(file_);  
    file_ = NULL;
  }
}

void Logger::Initialize() {
  current_day_ = TimeUtil::NowDay();
  Config(config_path);
  assert(file_);
  assert(level_ >= kTRACE && level_ <= kFATAL);
  spared_bufs_.reserve(LOGGER_MIN_BUF_NUM);
  for (int i = 0; i < LOGGER_MIN_BUF_NUM; ++i) {
    spared_bufs_.emplace_back(new glue_libbase::ByteBuffer);
    spared_bufs_.back()->SpareSpace(LOGGER_BUF_SIZE);
  }
}

/* Background thread function doing file dump. */
void Logger::BackgroundDump() {
  std::vector<std::unique_ptr<glue_libbase::ByteBuffer>> ready_bufs;
  running_ = true;
  while (running_) {
    bool memory_warn = false;
    size_t ready_num = 0;
    {
      MutexLockGuard m(mu_);
      // Flush in at most LOGGER_FLUSH_PERIOD seconds. 
      if (full_bufs_.empty()) {
        condvar_.WaitInSeconds(LOGGER_FLUSH_PERIOD);  
      }
      if (!full_bufs_.empty()) {
        ready_bufs.swap(full_bufs_);
      }
      if (current_buf_) {
        ready_bufs.emplace_back(std::move(current_buf_)); 
      }
      ready_num = ready_bufs.size();
      if (ready_num + spared_bufs_.size() >= LOGGER_MAX_BUF_NUM) {
        memory_warn = true; // logging is too often, maybe some errors occured.
      }
    }
    int now_day = TimeUtil::NowDay();
    if (now_day != current_day_) {
      SetValue("logfile", base_name_);  
      current_day_ = now_day;
    }
    
    if (memory_warn && ready_num > 2) {
      ready_bufs.resize(2);
      ready_num = 2; // 2 buffers of logs left to check the reason of fast-logging.
    }
    for (size_t i = 0; i < ready_num; ++i) {
      ::fwrite(ready_bufs[i]->AddrOfRead(), 1, ready_bufs[i]->ReadableBytes(), file_);
      ready_bufs[i]->Reset();
    }
    
    // reclaim the ready_bufs. 
    {
      MutexLockGuard m(mu_);
      size_t sum_num = spared_bufs_.size() + ready_num;
      spared_bufs_.reserve(sum_num);
      for (size_t i = 0; i < ready_num; ++i) {
        spared_bufs_.emplace_back(std::move(ready_bufs[i]));
      }
    }
    ready_bufs.clear();
    ::fflush(file_);
  }
  // When exits, there might be any unflushed bufs. 
  // Be sure that the other threads have been joined. 
  for (unsigned int i = 0; i < full_bufs_.size(); ++i) {
    ::fwrite(full_bufs_[i]->AddrOfRead(), 1, full_bufs_[i]->ReadableBytes(), file_);
  }
  if (current_buf_) {
    ::fwrite(current_buf_->AddrOfRead(), 1, current_buf_->ReadableBytes(), file_);
  }
  ::fflush(file_);
}

void Logger::WriteBuffer(const char* data, int size) {
  assert(size >= 0);
  MutexLockGuard m(mu_);
  if (!started_) {
    StartBackgroundThread();
    started_ = true;
  }
  if (current_buf_) {
    if (current_buf_->WritableBytes() >= static_cast<unsigned int>(size)) {
      current_buf_->Append(data, size);
    } else {
      // No enough space in the current buffer 
      full_bufs_.emplace_back(std::move(current_buf_));
      if (!spared_bufs_.empty()) { 
        current_buf_.swap(spared_bufs_.back());
        spared_bufs_.pop_back();
      } else {
        // spared bufs is empty, allocate one. 
        current_buf_.reset(new glue_libbase::ByteBuffer);
        current_buf_->SpareSpace(LOGGER_BUF_SIZE);
      }
      current_buf_->Append(data, size);
      condvar_.NotifyOne();
    }
  } else {
    if (!spared_bufs_.empty()) { 
      current_buf_.swap(spared_bufs_.back());
      spared_bufs_.pop_back();
    } else {
      // spared bufs is empty, allocate one. 
      current_buf_.reset(new glue_libbase::ByteBuffer);
      current_buf_->SpareSpace(LOGGER_BUF_SIZE);
    }
    current_buf_->Append(data, size);
  }
}

/*
 * Now, only the log file path and level need to be set in config file.
 * More options would be added here.
 */
int Logger::SetValue(const std::string& key, const std::string& value) {
  int ret = 1;
  if ("logfile" == key) {
    if (base_name_.empty()) {
      base_name_ = value;
    }
    if (file_) {
      // we are opening a new file, so close the old file.
      fclose(file_);
    }
    file_ = fopen((TimeUtil::NowTime() + base_name_ + ".log").c_str(), "a+");
    if (!file_) ret = 0;
  } else if ("level" == key) {
    level_ = static_cast<LevelType>(atoi(value.c_str()));
    level_ = static_cast<LevelType>(level_/10 * 10);
    if (level_ < kTRACE) {
      level_ = kTRACE;
    } else if (level_ > kFATAL) {
      level_ = kFATAL;
    }
  } else {
    /* Unknown options. */
    ret = 0; 
  }
  return ret;
}

/*
 * Config file should be only one key-value pair on one line,
 * while the key-value formate is like this: key=value
 */
int Logger::Config(const char* path) {
  int ret = 1;
  FILE *fp = fopen(path, "r");
  if (fp) {
    char *line = NULL;
    size_t len = 0;
    ssize_t read_num;

    std::string key, value;
    while ((read_num = getline(&line, &len, fp)) != -1) {
      char *dlim = strchr(line, '='); 
      if (dlim == NULL) {
        ret = 0;
        break;
      }
      key = std::string(line, dlim - line);
      value = std::string(dlim + 1, read_num - key.size() - 2);
      if (SetValue(key, value) == 0) {
        ret = 0;
        break;
      }
    }

    free(line);
    fclose(fp);
  }
  /* Using default log_file */
  if (!file_) {
    SetValue("logfile", "");
  }
  return ret;
}

static int LogPrintf(char** buf, int& len, const char* fmt, va_list ap) {
  int size = 0;
  va_list ap_copy;
  for(int i = 0; i < 2; ++i) {
    va_copy(ap_copy, ap);
    size = vsnprintf(*buf, len, fmt, ap_copy);
    va_end(ap_copy);
    if (i == 0) {
      if (size >= len) {
        /* Need more space */
        *buf = new char[len*5];
        len *= 5;
      } else {
        break;
      }
    } else {
      assert(size < len);
    }
  }
  return size;
}

static int LogPrintfWrapper(char** buf, int& len, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  int size = LogPrintf(buf, len, fmt, ap);
  va_end(ap);
  return size;
}

static const int LOG_BASE_SIZE = 1024;
static const int LOG_MSG_SIZE = 512;
/* TODO: flush stream asynchronously */
void Logger::Log(int level, const char* file, int line, const char* func, 
   const char* msg_format, va_list ap, const char* state) { /* state: errno string */
  /* Write message part */
  char msg_buf[LOG_MSG_SIZE] = {'\0'};
  char* p = msg_buf;
  int len = sizeof(msg_buf);
  int size = 0;
  if (msg_format && ap) {
     size = LogPrintf(&p, len, msg_format, ap);
  }
  char* msg_ptr = p;
  if (state) {
    char* tail = msg_ptr + size - 1; 
    *(tail + 1) = ' '; /* Change the null-terminator to the space char. */
    strcat(tail + 1, state);
  }

  /* Write whole log */
  char base_buf[LOG_BASE_SIZE] = {'\0'};
  int thread_id = ThreadId();
  p = base_buf;
  len = sizeof(base_buf);
   
  /* header formate like this: <[LEVEL]> <TIME> <FILE> <LINE> <FUNC> <MSG> <THREADNUM> */
  size = LogPrintfWrapper(&p, len, default_format, 
                    level_str[level/10], TimeUtil::NowTime().c_str(), 
                    file, line, func, thread_id, msg_ptr);
  WriteBuffer(p, size);
  if (msg_ptr != msg_buf) {
    delete msg_ptr;
  }
  if (p != base_buf) {
    delete p;
  }
}

void Logger::LogTrace(const char* file, int line, const char* func) {
  if (kTRACE < level_) return;
  Log(kTRACE, file, line, func, NULL, NULL, NULL);
}

void Logger::LogInfo(const char* file, int line, const char* func, const char* fmt, ...) {
  if (kINFO < level_) return;
  va_list ap;
  va_start(ap, fmt);
  Log(kINFO, file, line, func, fmt, ap, NULL);
  va_end(ap);
}

void Logger::LogWarn(const char* file, int line, const char* func, const char* fmt, ...) {
  if (kWARN < level_) return;
  va_list ap;
  va_start(ap, fmt);
  Log(kWARN, file, line, func, fmt, ap, NULL);
  va_end(ap);
}

void Logger::LogError(const char* file, int line, const char* func, const char* fmt, ...) {
  if (kERROR < level_) return;
  const char* err_str = errno ? strerror(errno) : "";
  va_list ap;
  va_start(ap, fmt);
  Log(kERROR, file, line, func, fmt, ap, err_str);
  va_end(ap);
}

void Logger::LogFatal(const char* file, int line, const char* func, const char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  const char* err_str = errno ? strerror(errno) : "";
  Log(kFATAL, file, line, func, fmt, ap, err_str);
  va_end(ap);
  abort();
}

void Logger::LogCheck(const char* file, int line, const char* func, bool flag, const char* msg) {
  if (flag) return;
  LogFatal(file, line, func, "Check Failed: %s!", msg);
}
} // namespace glue_libbase
