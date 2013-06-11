/*
 * syscallParser.h
 *
 *  Created on: Jun 10, 2013
 *      Author: Florian Kelbert
 */

#ifndef SYSCALLPARSER_H_
#define SYSCALLPARSER_H_

#include <stddef.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include "event.h"
#include "syscalls.h"
#include "constants.h"



event_ptr parseSyscall(event_ptr event, const int pid, long *syscallcode,
		struct user_regs_struct *regs);



#endif /* SYSCALLPARSER_H_ */
