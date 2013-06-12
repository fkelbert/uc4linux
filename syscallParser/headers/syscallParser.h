/*
 * syscallParser.h
 *
 *  Created on: Jun 10, 2013
 *      Author: Florian Kelbert
 */

#ifndef SYSCALLPARSER_H_
#define SYSCALLPARSER_H_

#include <fcntl.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/syscall.h>
#include <sys/ptrace.h>
#include <sys/user.h>

#include "event.h"
#include "syscallNames.h"


#define BUFLEN_INT 		24
#define BUFLEN_LONG 	48

#define LONG_SIZE sizeof(long)

#define int_to_str(i,buf,len) 	snprintf(buf,len,"%d",i)
#define long_to_str(i,buf,len) 	snprintf(buf,len,"%ld",i)
#define long_to_hex(i,buf,len) 	snprintf(buf,len,"%lx",i)


#define MAX_FILENAME_LEN 	4096
#define ADDRESS_WIDTH 		sizeof(intptr_t)

#define MULT_ADDR_SIZE(INT) (INT * ADDRESS_WIDTH)

event_ptr parseSyscall(event_ptr event, const int pid, long *syscallcode,
		struct user_regs_struct *regs);

char *getString(pid_t child, long addr, char *dataStr, int len_buf);


#endif /* SYSCALLPARSER_H_ */
