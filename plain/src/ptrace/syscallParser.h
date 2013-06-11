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

#define int_to_str(i,buf,len) snprintf(buf,len,"%d",i)
#define long_to_str(i,buf,len) snprintf(buf,len,"%ld",i)
#define long_to_hex(i,buf,len) snprintf(buf,len,"%lx",i)



event_ptr parseSyscall(event_ptr event, const int pid, long *syscallcode,
		struct user_regs_struct *regs);

char *getString(pid_t child, long addr, char *dataStr, int len_buf);


#endif /* SYSCALLPARSER_H_ */
