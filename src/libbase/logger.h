#ifndef GLUE_LIBBASE_LOGGER_H_
#define GLUE_LIBBASE_LOGGER_H_

#include "network/buffer.h"
#include "libbase/thread.h"
#include "libbase/mutex.h"
#include "libbase/condvar.h"

#include <stdio.h>
#include <stddef.h>

#include <string>
#include <vector>
#include <memory>
#include <atomic>

/* Nasty macro definitions for LOG_* */
#define LOG_TRACE()       do { glue_libbase::Logger::GetLogger().LogTrace(__FILE__, __LINE__, __func__   \
                              ); } while(0)
#define LOG_INFO(M, ...)  do { glue_libbase::Logger::GetLogger().LogInfo(__FILE__, __LINE__, __func__,   \
                              M, ##__VA_ARGS__); } while(0)
#define LOG_WARN(M, ...)  do { glue_libbase::Logger::GetLogger().LogWarn(__FILE__, __LINE__, __func__,   \
                              M, ##__VA_ARGS__); } while(0)
#define LOG_ERROR(M, ...) do { glue_libbase::Logger::GetLogger().LogError(__FILE__, __LINE__, __func__,  \
                              M, ##__VA_ARGS__); } while(0)
#define LOG_FATAL(M, ...) do { glue_libbase::Logger::GetLogger().LogFatal(__FILE__, __LINE__, __func__,  \
                              M, ##__VA_ARGS__); } while(0)
#define LOG_CHECK(B, M)   do { glue_libbase::Logger::GetLogger().LogCheck(__FILE__, __LINE__, __func__,  \
                              (B), "("#B")" M);} while(0)

/* A thread-safe implementation of logger in Singleton pattern. */
namespace glue_libbase {
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

  /* No copy allowed. */
  Logger(const Logger&);
  const Logger& operator=(const Logger&);

  void Log(int level, const char* file, int line, const char* func, const char* msg_format, va_list ap, const char*);
  int SetValue(const std::string& key, const std::string& value);
  int Config(const char* path_of_config);
  /* Free logger_ after exitting main routine */
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
    /* wakeup the thread. */
    condvar_.NotifyOne();
    running_ = false;
    thread_.Join();
  }

  typedef glue_network::ByteBuffer BufferType;
  LevelType level_;
  FILE* file_;
  MutexLock mu_;
  CondVar condvar_;
  Thread thread_;
  std::unique_ptr<glue_network::Buffer> current_buf_;
  std::vector<std::unique_ptr<BufferType>> spared_bufs_;
  std::vector<std::unique_ptr<BufferType>> full_bufs_;
  std::atomic<bool> running_;
  const int buf_size_;
  /* limit the num of bufs that the logger would take up at one time. */
  const int max_buf_num_; 
  static Logger* logger_;
  static Helper helper_;
};
}  // namespace glue_libbase

#endif // GLUE_LIBBASE_LOGGER_H_
