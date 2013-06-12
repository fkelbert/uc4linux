/*
 * base.h
 *
 *      Author: Ricardo Neisse, Cornelius Moucha
 */

#ifndef const_h
#define const_h

#include "config.h"

#include <stdint.h>
#include <sys/types.h>

#ifndef TRUE
  #define TRUE     1
#endif

#ifndef FALSE
  #define FALSE    0
#endif

static char* const bool_str[]={"false", "true"};

#define R_SUCCESS  0
#define R_ERROR    1

static unsigned char* const returnStr[]={"SUCCESS", "ERROR"};


#ifndef __cplusplus
  #ifndef bool
    typedef unsigned char bool;
  #endif
#endif

#define UNREACHABLE __builtin_unreachable()

#define esf_typedef(X) \
  typedef struct X##_s X##_t; \
  typedef X##_t *X##_ptr

#define check_null_int(X, ...) \
  if(X == NULL) {log_error(__VA_ARGS__); return R_ERROR;}

#define check_null_ptr(X, ...) \
  if(X == NULL) {log_error(__VA_ARGS__); return NULL;}

#define check_null(X, RETURNVALUE, CODE, ...) \
  if(X == NULL) {log_error(__VA_ARGS__); CODE; return RETURNVALUE;}

#define LIBEXPORT

// win32-mingw 
#ifdef __WIN32__
  typedef uint8_t uint;
  #undef LIBEXPORT
  #define LIBEXPORT __declspec(dllexport)
	#include <windows.h>

  // provide sleep-function from Windows-API (sleep X milliseconds)
  #define sleep(X) Sleep(X)
#endif

#endif
