#include "logger.h"
#include "timeutil.h"
#include "thread.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include <fstream>
#include <string>
#include <iostream>

namespace glue_libbase {

static const char config_path[] = "logger.conf";
/* Log formate like this: <[LEVEL]> <TIME> <FILE> <LINE> <FUNC> <TREADNUM> <MSG> */
static const char* default_format = "[%-5s] %s %s:%-4d %s() @Thread[%5d] %s\n";
static const char* level_str[] = {"", "TRACE", "INFO", "WARN", "ERROR", "FATAL"};

static const char* default_log_path = "out.log";
static const Logger::LevelType default_level = Logger::kWARN;
Logger* Logger::logger_ = new Logger();
Logger::Helper Logger::helper_;

Logger::Logger() {
  int ret = Config(config_path);
  assert(ret);
  assert(file_);
  assert(level_ >= kTRACE && level_ <= kFATAL); 
}

Logger::~Logger() {
  if (file_) {
    fclose(file_);  
    file_ = NULL;
  }
}

/*
 * Now, only the log file path and level need to set value in config file.
 * More options would be added here.
 */
int Logger::SetValue(const std::string& key, const std::string& value) {
  int ret = 1;
  if ("logfile" == key) {
    file_ = fopen(value.c_str(), "w+");
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
    std::cout << key << std::endl;
    ret = 0; 
  }
  return ret;
}

/*
 * Config file should be only one key-value pair on one line,
 * while the key-value formate is like this: key=value
 */
int Logger::Config(const char* path) {
  std::ifstream ifs(path, std::ios::in); 
  int ret = 1;
  level_ = default_level;
  file_ = NULL;
  if (ifs.good()) {
    std::string key, value;
    while(!ifs.eof()) {
      if (!std::getline(ifs, key, '=')) break;
      if (!std::getline(ifs, value, '\n')) break;
      if (SetValue(key, value) == 0) {
        std::cout << "setvalue error" << std::endl;
        ret = 0;
        break;
      }
    }
  }
  
  if (ifs.is_open())  ifs.close();
  if (ret) {
    /* Using default log_file */
    if (!file_) SetValue("logfile", default_log_path);
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
  int thread_id = tid();
  p = base_buf;
  len = sizeof(base_buf);
   
  /* header formate like this: <[LEVEL]> <TIME> <FILE> <LINE> <FUNC> <MSG> <THREADNUM> */
  size = LogPrintfWrapper(&p, len, default_format, 
                    level_str[level/10], TimeUtil::NowTime().c_str(), 
                    file, line, func, thread_id, msg_ptr);
  fwrite(p, 1, size, file_);
  fflush(file_);
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
