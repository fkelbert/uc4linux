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

#include "event.h"
#include "tracee.h"
#include "syscalls.h"

#define SOCKET_OFFSET 347


event_ptr parseSyscall(event_ptr event, struct tracee *tracee);



#endif /* SYSCALLPARSER_H_ */
