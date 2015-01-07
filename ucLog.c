#include "ucLog.h"


void uc_log_impl(const char* format, ...) {
	va_list argptr;
	va_start(argptr, format);
	vfprintf(outstream, format, argptr);
	va_end(argptr);
	fflush(outstream);
}
