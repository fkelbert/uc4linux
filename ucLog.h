#ifndef UC_LOG_H
#define UC_LOG_H

#include <stdarg.h>
#include <stdio.h>

#include "ucSettings.h"

#define outstream stdout

// Depending on whether debug mode is enabled or disabled, the
// following code either defines functions that get executed or
// replaces the corresponding by nothing
#if defined(UC_DEBUG) && UC_DEBUG
	#define uc_log(format, ...) uc_log_impl(format, ##__VA_ARGS__)
	void uc_log_impl(const char* format, ...);
#else
	#define uc_log(...)
#endif

#endif
