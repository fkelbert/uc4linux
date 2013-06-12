#include "logger.h"
#include "logger_base.h"

#ifndef _log_shell_wrapper_c_h
#define _log_shell_wrapper_c_h

  #ifndef LOG_SHELL_WRAPPER_C
    #define LOG_SHELL_WRAPPER_C GLOBAL_LOG_LEVEL
  #endif

  #ifndef LOG_SHELL_WRAPPER_C_OUTPUT
    #define LOG_SHELL_WRAPPER_C_OUTPUT GLOBAL_LOG_OUTPUT
	#ifdef STDOUTFLAG 
		#define LOG_SHELL_WRAPPER_C_OUTPUT_STD 1
	#endif
  #endif
    extern FILE *LOG_SHELL_WRAPPER_C_OUTPUT;
    static char *curlogfilename=QUOTEME(LOG_SHELL_WRAPPER_C_OUTPUT);

    void log_shell_wrapper_c_init(FILE *fp, char *modname, unsigned int level, char *msg, ...);
    void (*log_shell_wrapper_c_outputfunction)(FILE *fp, char *modname, unsigned int level, char *msg, ... )=log_shell_wrapper_c_init;
    void log_shell_wrapper_c_init(FILE *fp, char *modname, unsigned int level, char *msg, ...)
    {
	  #ifndef LOG_SHELL_WRAPPER_C_OUTPUT_STD
		  if(LOG_SHELL_WRAPPER_C_OUTPUT==NULL)
		  {
			char *fplogfilename;
			fplogfilename=(char*)calloc(strlen(curlogfilename) + strlen(log_absPath)+1, sizeof(char));
			snprintf(fplogfilename, (strlen(log_absPath)+strlen(curlogfilename)+1), "%s%s",log_absPath, curlogfilename);
			LOG_SHELL_WRAPPER_C_OUTPUT=fopen(fplogfilename, "a+");
			if(LOG_SHELL_WRAPPER_C_OUTPUT==NULL) {fprintf(stderr,"error opening logging output file...\n"); exit(EXIT_FAILURE);}
		  }
	  #else
		LOG_SHELL_WRAPPER_C_OUTPUT = stdout;
	  #endif
      va_list fmtargs;
      va_start(fmtargs,msg);
      vsnprintf(buffer,sizeof(buffer)-1,msg,fmtargs);
      log_shell_wrapper_c_outputfunction=rlog; // redirect function pointer
      log_shell_wrapper_c_outputfunction(LOG_SHELL_WRAPPER_C_OUTPUT, (char*)"shell_wrapper", level, buffer);
    }

  #if LOG_SHELL_WRAPPER_C > LOG_TRACE
    #define log_trace(...)
  #else
    #define log_trace(...) log_shell_wrapper_c_outputfunction(LOG_SHELL_WRAPPER_C_OUTPUT, "shell_wrapper", LOG_TRACE, __VA_ARGS__)
  #endif

  #if LOG_SHELL_WRAPPER_C > LOG_DEBUG
    #define log_debug(...)
  #else
    #define log_debug(...) log_shell_wrapper_c_outputfunction(LOG_SHELL_WRAPPER_C_OUTPUT, "shell_wrapper", LOG_DEBUG, __VA_ARGS__)
  #endif

  #if LOG_SHELL_WRAPPER_C > LOG_INFO
    #define log_info(...)
  #else
    #define log_info(...) log_shell_wrapper_c_outputfunction(LOG_SHELL_WRAPPER_C_OUTPUT, "shell_wrapper", LOG_INFO, __VA_ARGS__)
  #endif

  #if LOG_SHELL_WRAPPER_C > LOG_WARN
    #define log_warn(...)
  #else
    #define log_warn(...) log_shell_wrapper_c_outputfunction(LOG_SHELL_WRAPPER_C_OUTPUT, "shell_wrapper", LOG_WARN, __VA_ARGS__)
  #endif

  #if LOG_SHELL_WRAPPER_C > LOG_ERROR
    #define log_error(...)
  #else
    #define log_error(...) log_shell_wrapper_c_outputfunction(LOG_SHELL_WRAPPER_C_OUTPUT, "shell_wrapper", LOG_ERROR, __VA_ARGS__)
  #endif

  #if LOG_SHELL_WRAPPER_C > LOG_FATAL
    #define log_fatal(...)
  #else
    #define log_fatal(...) log_shell_wrapper_c_outputfunction(LOG_SHELL_WRAPPER_C_OUTPUT, "shell_wrapper", LOG_FATAL, __VA_ARGS__)
  #endif
#endif
