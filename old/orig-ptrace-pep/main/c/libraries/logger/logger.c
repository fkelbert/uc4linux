/**
 * @file  logger.c
 * @brief Implementation of PEF logger module
 * @see logger.h
 *
 * @author cornelius moucha
 **/

#include "logger.h"

/// Global logger mutex
pefMutex_ptr logmtx=NULL;

void rlog(FILE *fp, char *modulename, unsigned int level, char *msg, ...)
{
  va_list fmtargs;
  va_start(fmtargs,msg);

  #if RUNTIME_LOGLEVEL_EVAL == 1
    if(level<runtimeLogLevel) return;
  #endif

  loggerMutexLock();
  vsnprintf(buffer,sizeof(buffer)-1,msg,fmtargs);

#ifdef _WIN32
  SYSTEMTIME lt;
  GetLocalTime(&lt);
  char *timestr=malloc(20*sizeof(char));
  snprintf(timestr, 20, "%02d-%02d-%02d %02d:%02d:%02d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
#else
  time_t t = time(NULL);
  struct tm *local = (struct tm*)localtime(&t);
  char *timestr=malloc(20*sizeof(char));
  strftime(timestr, 20, "%F %T", local);
#endif
  PEFLOG
  free(timestr);

  loggerMutexUnlock();
  va_end(fmtargs);
  fflush(fp);
}

void rlognl(FILE *fp, unsigned int level, char *msg, ...)
{
  va_list fmtargs;
  va_start(fmtargs,msg);
  vsnprintf(buffer,sizeof(buffer)-1,msg,fmtargs);
  fprintf(fp,"%s", buffer);
  va_end(fmtargs);
  fflush(fp);
}

unsigned int pefLogSetRTLevel(unsigned int newLevel)
{
  if(newLevel<LOG_TRACE || newLevel>LOG_DISABLE) return R_ERROR;
  loggerMutexLock();
  runtimeLogLevel=newLevel;
  loggerMutexUnlock();
  return R_SUCCESS;
}

