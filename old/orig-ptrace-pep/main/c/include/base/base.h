/**
 * @file  base.h
 * @brief Basic header for PDP implementation
 *
 * Provides generic definitions and auxiliary macros
 *
 * @author cornelius moucha
 **/

#ifndef const_h
#define const_h

#include <stdint.h>
#include <sys/types.h>
#include "pdpConfig.h"

#ifndef TRUE
  #define TRUE     1
#endif

#ifndef FALSE
  #define FALSE    0
#endif

/// static textual representation of boolean values; used for logging
static char* const boolStr[]={"false", "true"};

#define R_SUCCESS  0
#define R_ERROR    1

/// static textual representation of return values; used for logging
static char* const returnStr[]={"SUCCESS", "ERROR"};
/// static textual representation of error return value; used for logging
static char* const errorStr="ERROR";


#ifndef __cplusplus
  #ifndef bool
    typedef unsigned char bool;
  #endif
#endif

#define UNREACHABLE __builtin_unreachable()

#define pef_typedef(X) \
  typedef struct X##_s X##_t; \
  typedef X##_t *X##_ptr

/// auxiliary macro for checking a pointer for NULL with returning integer response R_SUCCESS/R_ERROR
#define checkNullInt(X, ...) \
  if(X == NULL) {log_error(__VA_ARGS__); return R_ERROR;}

/// auxiliary macro for checking a pointer for NULL with returning NULL pointer on error
#define checkNullPtr(X, ...) \
  if(X == NULL) {log_error(__VA_ARGS__); return NULL;}

/// auxiliary macro for checking a pointer for NULL with returning given RETURNVALUE
#define checkNull(X, RETURNVALUE, CODE, ...) \
  if(X == NULL) {log_error(__VA_ARGS__); CODE; return RETURNVALUE;}

#define LIBEXPORT

#ifdef __WIN32__
  #define size_t _ssize_t

  // commented because otherwise only these methods are exported in shared library;
  // in general ALL methods are exported except if at least on is explicitly exported using dllexport directive
  // which is given for JNI-methods --> added -Wl,--export-all-symbols to build script
  //#undef LIBEXPORT
  //#define LIBEXPORT __declspec(dllexport)
	#include <windows.h>
  #include <windef.h>
#endif

#endif
