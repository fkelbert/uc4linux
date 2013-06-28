/*
 * ucSemantics.h
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#ifndef UCSEMANTICS_H_
#define UCSEMANTICS_H_

#include "defs.h"
#include "syscall.h"


#include <sys/fcntl.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

void ucPIPupdate(struct tcb *tcp);

void ucDataFlowSemantics_write(struct tcb *tcp);
void ucDataFlowSemantics_read(struct tcb *tcp);
void ucDataFlowSemantics_exit(struct tcb *tcp);
void ucDataFlowSemantics_clone(struct tcb *tcp);
void ucDataFlowSemantics_close(struct tcb *tcp);
void ucDataFlowSemantics_dup(struct tcb *tcp);
void ucDataFlowSemantics_dup2(struct tcb *tcp);
void ucDataFlowSemantics_eventfd(struct tcb *tcp);
void ucDataFlowSemantics_execve(struct tcb *tcp);
void ucDataFlowSemantics_fcntl(struct tcb *tcp);
void ucDataFlowSemantics_ftruncate(struct tcb *tcp);
void ucDataFlowSemantics_kill(struct tcb *tcp);
void ucDataFlowSemantics_mmap(struct tcb *tcp);
void ucDataFlowSemantics_unlink(struct tcb *tcp);
void ucDataFlowSemantics_munmap(struct tcb *tcp);
void ucDataFlowSemantics_openat(struct tcb *tcp);
void ucDataFlowSemantics_open(struct tcb *tcp);
void ucDataFlowSemantics_pipe(struct tcb *tcp);
void ucDataFlowSemantics_rename(struct tcb *tcp);
void ucDataFlowSemantics_splice(struct tcb *tcp);
void ucDataFlowSemantics_accept(struct tcb *tcp);
void ucDataFlowSemantics_connect(struct tcb *tcp);
void ucDataFlowSemantics_shutdown(struct tcb *tcp);
void ucDataFlowSemantics_socketpair(struct tcb *tcp);
void ucDataFlowSemantics_socket(struct tcb *tcp);

#endif /* UCSEMANTICS_H_ */
