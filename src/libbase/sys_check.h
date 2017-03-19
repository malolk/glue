#ifndef LIBBASE_SYS_CHECK_H__
#define LIBBASE_SYS_CHECK_H__

#include <stdio.h>
#include <string.h>

#define SYS_CHECK(RET)    do {                                                                      \
                               if (RET) {                                                           \
                                 fprintf(stderr, "%s::%s:%d error: #%d %s\n", __FILE__, __func__,   \
                                        __LINE__, RET, strerror(RET));                              \
                               }                                                                    \
                             } while(0)                                                             \

#endif // LIBBASE_SYS_CHECK_H__

