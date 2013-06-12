/**
 * @file logger.h
 * @brief header for logger module
 * header file for logger module
 *
 * defines different logging levels and global logging level and output; \n
 * method declaration for logger
 *
 * @author Ricardo Neisse, Cornelius Moucha
 **/

#ifndef _logger_h
#define _logger_h

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
// include base.h is required for win32 (to include windows.h)
#include "base.h"
#include "loggerconf.h"

/// nothing is logged; *obsolete* will be replaced by LOG_DISABLE
#define LOG_NONE    0
/// trace messages that show all the calls made by the program
#define LOG_TRACE   1
/// debugging messages used when something is not working
#define LOG_DEBUG   2
/// information messages is the maximum output that should be easily readable
#define LOG_INFO    3
/// warning when something is wrong but do not compromises the program
#define LOG_WARN    4
/// error when something is wrong and compromises the program, however, it is possible to continue execution
#define LOG_ERROR   5
/// error that dos not allow program to continue execution
#define LOG_FATAL   6
/// nothing is logged
#define LOG_DISABLE 7

/// auxiliary function for quoting value of other preprocessor directives; used for automatic generation of logger configuration
#define QUOTEME_(x) #x
///auxiliary function for quoting value of other preprocessor directives; \n indirect quoting necessary for getting value instead of the name of directives
#define QUOTEME(x)  QUOTEME_(x)

static char buffer[2048];
static char* const loglevels[]={"TRACE","DEBUG","INFO ","WARN ","ERROR","FATAL"};

// to avoid compiler warning: incompatible implicit declaration of built-in function strlen
extern size_t strlen(const char *);

void rlog(FILE *fp, char *modulename, unsigned int level, char *msg, ...);
void rlognl(FILE *fp, unsigned int level, char *msg, ...);
#endif
