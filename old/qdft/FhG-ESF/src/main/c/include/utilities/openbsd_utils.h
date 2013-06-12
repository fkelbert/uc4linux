/*
 * openbsd_utils.h
 *      Author: Ricardo Neisse
 */
#ifndef _openbsd_utils_h
#define _openbsd_utils_h

#include <pwd.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <string.h>
#include "base.h"

struct passwd *getUserInfo(int pid, struct passwd *user_info);
char *getProcCommand(int pid);

#endif
