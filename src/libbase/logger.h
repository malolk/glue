#ifndef GLUE_LIBBASE_LOGGER_H_
#define GLUE_LIBBASE_LOGGER_H_

#include <stdio.h>
#include <stddef.h>

#include <string>

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

  LevelType level_;
  FILE* file_;
  static Logger* logger_;
  static Helper helper_;
};
}  // namespace glue_libbase

#endif // GLUE_LIBBASE_LOGGER_H_
