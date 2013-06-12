/**
 * @file loggerconf.h
 * @brief logger configuration header
 *
 * Place for defining logging levels for specific source files, which will override global one, but only for the mentioned
 * source \n
 * Syntax (all uppercase): \n
 * LOG_<b>FILENAME</b>_<b>EXTENSION</b> LOG_LEVEL \n
 * (e.g. LOG_EVENT_C LOG_TRACE => event.c will have tracing enabled)
 * \n \n
 * Additionally an output file can be defined for specific source files, which will override global value STDOUT \n
 * Syntax (all uppercase): \n
 * LOG_<b>FILENAME</b>_<b>EXTENSION</b>_OUTPUT output_filename \n
 * (e.g. LOG_EVENT_C_OUTPUT output_eventc => loggings for event.c will be written to file output_eventc)
 *
 * @author Cornelius Moucha
 **/

#ifndef _loggerconf_h
#define _loggerconf_h

/**
 * LOG_ABSPATH_PREFIX: absolute path for logfiles specified with LOG_FILENAME_OUTPUT
 **/
//static char * const log_absPath = "/var/log/esf/";
// not working in windows; logfile will be written in executing directory
static char *const log_absPath = "";

/// global logging level; can be overridden for specific source files in loggerconf.h
#define GLOBAL_LOG_LEVEL LOG_NONE
/* #define GLOBAL_LOG_LEVEL LOG_INFO */

/// global output for logging; can be overridden for specific source files in loggerconf.h
#define GLOBAL_LOG_OUTPUT stdout
//#define GLOBAL_LOG_OUTPUT fhgesflog1

//#define LOG_PDP_C LOG_INFO
//#define LOG_ESFEVENT_C LOG_INFO
//#define LOG_ACTION_C LOG_INFO
//#define LOG_FORMULA_C LOG_INFO

//#define LOG_PDP_C_OUTPUT fhgesflogrd
//#define LOG_EVENT_C_OUTPUT fhgesflogrd
//#define LOG_ESFEVENT_C_OUTPUT fhgesflogrd
//#define LOG_ACTION_C_OUTPUT fhgesflogrd
//#define LOG_OPENBSD_UTILS_C_OUTPUT fhgesflogrd
//#define LOG_XML_UTILS_C_OUTPUT fhgesflogrd
//#define LOG_MECHANISM_C_OUTPUT fhgesflogrd
//#define LOG_FORMULA_C_OUTPUT fhgesflogrd
//#define LOG_TIMESTAMP_C_OUTPUT fhgesflogrd
//#define LOG_SOUP_UTILS_C_OUTPUT fhgesflogrd
//#define LOG_SYSCALL_HANDLER_C_OUTPUT fhgesflogrd
//#define LOG_SYSCALLHANDLER_C LOG_DEBUG
//#define LOG_SYSTRACEPEP_C LOG_DEBUG



#endif
