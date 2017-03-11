#ifndef GLUE_LIBBASE_LOGGER_H_
#define GLUE_LIBBASE_LOGGER_H_

#include "libbase/buffer.h"
#include "libbase/thread.h"
#include "libbase/mutexlock.h"
#include "libbase/condvar.h"

#include <stdio.h>
#include <stddef.h>

#include <string>
#include <vector>
#include <memory>
#include <atomic>

/* A thread-safe implementation of logger in Singleton pattern. */
namespace libbase {
class Logger {
 public: 
  enum LevelType {
    kTRACE   =   10,
    kINFO    =   20,
    kWARN    =   30,
    kERROR   =   40,
    kFATAL   =   50,
  };

  static Logger& GetLogger() {
    return *logger_;
  }
  void LogTrace(const char* file, int line, const char* func);
  void LogInfo(const char* file, int line, const char* func, const char* fmt, ...);
  void LogWarn(const char* file, int line, const char* func, const char* fmt, ...); 
  void LogError(const char* file, int line, const char* func, const char* fmt, ...);
  void LogFatal(const char* file, int line, const char* func, const char* fmt, ...);
  void LogCheck(const char* file, int line, const char* func, bool flag, const char* msg);

 private:
  Logger();
  ~Logger();

  // No copy allowed.
  Logger(const Logger&);
  const Logger& operator=(const Logger&);

  void Log(int level, const char* file, int line, const char* func, const char* msg_format, va_list ap, const char*);
  int SetValue(const std::string& key, const std::string& value);
  int Config(const char* path_of_config);
  void Initialize();
  // Free logger_ after exitting main routine
  class Helper {
   public:
    ~Helper() {
      if (logger_) {
        delete logger_;
        logger_ = NULL;
      }
    }
  };

  void WriteBuffer(const char* data, int size);
  void BackgroundDump();
  void StartBackgroundThread() {
    thread_.Start();
    thread_.Schedule(std::bind(&Logger::BackgroundDump, this));
  }
  void StopBackgroundThread() {
    // wakeup the thread. 
    condvar_.NotifyOne();
    running_ = false;
    thread_.Join();
  }

  LevelType level_;
  FILE* file_;
  MutexLock mu_;
  CondVar condvar_;
  Thread thread_;
  std::unique_ptr<ByteBuffer> current_buf_;
  std::vector<std::unique_ptr<ByteBuffer>> spared_bufs_;
  std::vector<std::unique_ptr<ByteBuffer>> full_bufs_;
  std::atomic<bool> running_;
  std::atomic<bool> started_;
  // switch to different logging file when the current_day_ changed.
  std::string base_name_;
  int current_day_;
  const int buf_size_;
  // limit the num of bufs that the logger would take up at one time.
  const int max_buf_num_; 
  static Logger* logger_;
  static Helper helper_;
};
}  // namespace libbase

#endif // GLUE_LIBBASE_LOGGER_H_
