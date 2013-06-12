/**
 * @file  logger.h
 * @brief header for logger module
 *
 * Defines different logging levels and global logging level and output; \n
 * The methods defined here are not intended for direct usage! Instead use the log_trace up to log_fatal which are
 * dynamically provided in the preprocessor preparation. Therefore they can also be eliminated at compile time, if the
 * logging level for the concerned file is set to a higher level!
 *
 * @author cornelius moucha
 **/

#ifndef _logger_h
#define _logger_h

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "base.h"
#include "time.h"
#include "memUtils.h"
#include "threadUtils.h"
#include "loggerconf.h"

#ifdef  __cplusplus
  extern "C" {
#endif

/// trace messages; most verbose logging level; outputs everything
#define LOG_TRACE   2
/// debugging messages
#define LOG_DEBUG   3
/// information messages
#define LOG_INFO    4
/// warning without compromising program
#define LOG_WARN    5
/// error compromising program, but program execution can continue
#define LOG_ERROR   6
/// error preventing continuous program execution (does NOT exist on Android -> mapped to LOG_ERROR)
#define LOG_FATAL   7
/// logging disabled
#define LOG_DISABLE 8

/// auxiliary function for quoting value of other preprocessor directives; used for automatic generation of logger configuration
#define QUOTEME_(x) #x
///auxiliary function for quoting value of other preprocessor directives; \n indirect quoting necessary for getting value instead of the name of directives
#define QUOTEME(x)  QUOTEME_(x)

#ifdef PEF_LOGANDROID
  #include <utils/Log.h>
  /// reference to the Android internal logging behavior to simulate with the usual logging output
  #define PEFLOG __android_log_print(level+1, modulename, "[Log] %s",buffer);

  // redefine LOG_FATAL to LOG_ERROR as Android's highest supported logging level
  #undef LOG_FATAL
  #define LOG_FATAL LOG_ERROR
#else
  /// define the logging output syntax scheme
  #define PEFLOG fprintf(fp,"[%s][%15.15s][%s] %s\n", timestr, modulename, loglevels[level-1], buffer);
  //#define PEFLOG fprintf(fp,"[%15.15s][%s] %s\n", modulename, loglevels[level-1], buffer);
#endif

/// internal logging buffer
static char buffer[2048];

/// static textual representation of logging levels; used for logging
static char* const loglevels[]={"notSet","TRACE","DEBUG","INFO ","WARN ","ERROR","FATAL"};

/// static reference to the runtime loglevel defined in the logger configuration;
/// only used if the runtime evaluation of the logging level is enabled
static unsigned int runtimeLogLevel=RUNTIME_LOGLEVEL;

/// Initialize pefLogger, the logger mutex depending on the current platform
void logInit();

/// Deallocate pefLogger, the logger mutex depending on the current platform
void logDestroy();

/// Platform dependent locking of logger mutex
void loggerMutexLock();
/// Platform dependent releasing of logger mutex
void loggerMutexUnlock();

/**
 * Read the variable arguments list and output the logging message
 * to the given filepointer (usual stdout) according to format string given as msg
 *
 * @todo  Insert parameter for disabling newline!
 *
 * @param fp         file pointer for logging message (usually stdout)
 * @param modulename name of the appropriate module (e.g. filename)
 * @param level      log level for logging message
 * @param msg        message to be logged with placeholders according format string specification for printf
 **/
void rlog(FILE *fp, char *modulename, unsigned int level, char *msg, ...);


/**
 * Read the variable arguments list and output the logging message without a final newline
 * to the given filepointer (usual stdout) according to format string given as msg
 *
 * @param fp         file pointer for logging message (usually stdout)
 * @param level      log level for logging message
 * @param msg        message to be logged with placeholders according format string specification for printf
 **/
void rlognl(FILE *fp, unsigned int level, char *msg, ...);

/**
 * Set the runtime logging level to the given value; only used if runtime logging evaluation is enabled
 *
 * @param  newLevel   new logging level
 * @return R_SUCCESS
 **/
unsigned int pefLogSetRTLevel(unsigned int newLevel);

#ifdef  __cplusplus
  }
#endif

#endif




