#include <stdio.h>
#include <stdarg.h>

#include "logger.h"
   
void plog(int level, char *msg, ...) {
  /*
   * TODO: option to direct the log to a file
   * and option to set different log levels for
   * different modules.
   *
   * - past_monitor->level = INFO
   * - control_monitor->level = TRACE
   *
   */

  if (level < LOG_LEVEL)
	  return;
  va_list fmtargs;
  char buffer[1024]; 
  va_start(fmtargs,msg);
  vsnprintf(buffer,sizeof(buffer)-1,msg,fmtargs);
  if (level==LOG_TRACE) printf("[TRACE] %s\n",buffer);
  if (level==LOG_DEBUG) printf("[DEBUG] %s\n",buffer);
  if (level==LOG_INFO)  printf("[INFO]  %s\n",buffer);
  if (level==LOG_WARN)  printf("[WARN]  %s\n",buffer);
  if (level==LOG_ERROR) printf("[ERROR] %s\n",buffer);
  if (level==LOG_FATAL) printf("[FATAL] %s\n",buffer);
  va_end(fmtargs);

  fflush(NULL);
}




