#include <stdarg.h>

#ifndef _logger_h
#define _logger_h

#define LOG_NONE  0
#define LOG_TRACE 1
#define LOG_DEBUG 2
#define LOG_INFO  3
#define LOG_WARN  4
#define LOG_ERROR 5
#define LOG_FATAL 6

#define LOG_LEVEL LOG_FATAL

#define elog(LEVEL) LEVEL >= LOG_LEVEL

void plog(int level, char *msg, ...);

#endif
