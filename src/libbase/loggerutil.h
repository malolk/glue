#ifndef GLUE_LIBBASE_LOGGERUTIL_H_
#define GLUE_LIBBASE_LOGGERUTIL_H_

#include "libbase/logger.h"

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

#endif // GLUE_LIBBASE_LOGGERUTIL_H_
