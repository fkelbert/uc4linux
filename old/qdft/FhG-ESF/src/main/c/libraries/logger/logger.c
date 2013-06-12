/**
 * @file logger.c
 * @brief implementation for logger module
 *
 * methods implementation for logger
 *
 * @author Ricardo Neisse, Cornelius Moucha
 **/

#include "logger.h"

/**
 * @brief logging method
 * reads the variable arguments list and outputs the logging message
 * to given filepointer (usual stdout) according to format string given as msg
 *
 * @todo  Insert parameter for disabling newline!
 *
 * @param fp      file pointer for logging message
 * @param level   log level for logging message
 * @param msg     message to be logged with placeholders according format string specification for printf
 **/

void rlog(FILE *fp, char *modulename, unsigned int level, char *msg, ...)
{
  va_list fmtargs;
  va_start(fmtargs,msg);
  vsnprintf(buffer,sizeof(buffer)-1,msg,fmtargs);

#ifdef __WIN32__
  SYSTEMTIME st, lt;
  GetLocalTime(&lt);
  char *timestr=malloc(20*sizeof(char));
  snprintf(timestr, 20, "%02d-%02d-%02d %02d:%02d:%02d", lt.wYear, lt.wMonth, lt.wDay, lt.wHour, lt.wMinute, lt.wSecond);
#else
  time_t t = time(NULL);
  struct tm *local = (struct tm*)localtime(&t);
  char *timestr=malloc(20*sizeof(char));
  strftime(timestr, 20, "%F %T", local);
#endif
  fprintf(fp,"[%s][%s][%s] %s\n", timestr, modulename, loglevels[level-1], buffer);
  free(timestr);
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

