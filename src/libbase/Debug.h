#ifndef LIBBASE_DEBUG__H
#define LIBBASE_DEBUG_H

#include <libbase/CurrentThread.h>
#include <libbase/TimeStamp.h>

#include <iostream>
#include <string>

#include <string.h>
#include <errno.h>
#include <stdlib.h>

#ifndef DEBUG
	#define LOGTRACE() while(0)
	#define LOGPRINT(X, LEVEL) while(0) 
#else
	#define LOGTRACE() do { std::cerr << ( \
	libbase::TimeStamp().bePretty() +      \
	std::string(" TRACE: ") +              \
	__FILE__ + " LINE:" +                  \
	std::to_string(__LINE__) + " : " +     \
	 __func__ + "() T" +                   \
	std::to_string(libbase::tid())) << std::endl; } while(0)

	#define LOGPRINT(X, LEVEL) do { std::cerr << ( \
	libbase::TimeStamp().bePretty() + " " +        \
	std::string(LEVEL) + ": " +                    \
	__FILE__ + " LINE:" +                          \
	std::to_string(__LINE__) + " : " +             \
	__func__ + "() : " +                           \
	 X +                                           \
	 " T" + std::to_string(libbase::tid())) << std::endl; } while(0)
#endif

#define LOGINFO(X)  LOGPRINT(X, std::string("INFO")) 
#define LOGWARN(X)  LOGPRINT(X, std::string("WARN")) 
#define LOGFATAL(X) do { LOGPRINT(X, std::string("FATAL")); _Exit(1); } while(0) 
#define LOGERROR(E) do { LOGPRINT(std::string(E ? strerror(E) : ""), std::string("SYSERROR")); } while(0)
#define LOGERROREXIT(E) do { LOGPRINT(std::string(E ? strerror(E) : ""), std::string("SYSERROR")); _Exit(1); } while(0)

#define CHECK(C) if(!(C))  do { LOGINFO(std::string("CHECK failed")); } while(0)
#define CHECKEXIT(C) if(!(C)) do { LOGFATAL(std::string("CHECK failed, exit"));  } while(0)
#define CHECKXEXIT(C, X) if(!(C)) do { LOGFATAL(std::string("CHECK failed, exit") + std::string(X)); } while(0)
#define CHECKX(C, X) if(!(C)) do { LOGINFO(std::string("CHECK failed ") + std::string(X)); } while(0)
#define CHECKSYS(C, E) if(!(C)) do { LOGERROR(E); } while(0)

#endif //LIBBASE_DEBUG_H
