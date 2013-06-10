
#ifndef PEP_H_
#define PEP_H_

#include <stddef.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <glib/ghash.h>
#include <errno.h>
#include <syscall.h>
#include <stdio.h>
#include <unistd.h>

#include "ptrace_interface_pdp.h"
#include "constants.h"
#include "traceeManager.h"

#define MULT4(INT) (INT << 2)
#define PTRACE_USED_OPTIONS PTRACE_O_TRACECLONE | PTRACE_O_TRACEFORK | PTRACE_O_TRACEVFORK


#endif /* PEP_H_ */
