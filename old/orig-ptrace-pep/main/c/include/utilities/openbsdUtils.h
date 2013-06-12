/**
 * @file openbsdUtils.h
 * @brief Auxiliary utilities for OpenBSD (systrace)
 *
 * @author  Ricardo Neisse
**/

#ifndef _openbsdutils_h
#define _openbsdutils_h

#include <pwd.h>
#include <kvm.h>
#include <sys/param.h>
#include <sys/sysctl.h>
#include <string.h>
#include "base.h"

struct passwd *getUserInfo(int pid, struct passwd *user_info);
char          *getProcCommand(int pid);

#endif
