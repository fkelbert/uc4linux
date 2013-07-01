/*
 * ucSemantics.h
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#ifndef UC_SEMANTICS_H_
#define UC_SEMANTICS_H_

#include "defs.h"
#include "syscall.h"

#include <sys/fcntl.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

#define UC_SEMANTICS_DEBUG 1

#define IDENTIFIER_MAX_LEN 512

#define IS_O_RDWR(flag)		((flag & O_RDWR) 	== O_RDWR)
#define IS_O_WRONLY(flag)	((flag & O_WRONLY) 	== O_WRONLY)
#define IS_O_TRUNC(flag)	((flag & O_TRUNC) 	== O_TRUNC)

#define ucSemantics_errorExit(msg) \
			fprintf(stderr, "%s\n", msg); \
			fprintf(stderr, "Happened in: %s:%d\n", __FILE__, __LINE__); \
			fprintf(stderr, "Exiting.\n"); \
			exit (1);

#define isAbsolute(string) *string == '/'

#define streq(str1, str2) (strcmp(str1,str2) == 0)

void ucPIPupdate(struct tcb *tcp);

void ucDataFlowSemantics_accept(struct tcb *tcp);
void ucDataFlowSemantics_clone(struct tcb *tcp);
void ucDataFlowSemantics_close(struct tcb *tcp);
void ucDataFlowSemantics_connect(struct tcb *tcp);
void ucDataFlowSemantics_dup2(struct tcb *tcp);
void ucDataFlowSemantics_dup(struct tcb *tcp);
void ucDataFlowSemantics_eventfd(struct tcb *tcp);
void ucDataFlowSemantics_execve(struct tcb *tcp);
void ucDataFlowSemantics_exit(struct tcb *tcp);
void ucDataFlowSemantics_fcntl(struct tcb *tcp);
void ucDataFlowSemantics_ftruncate(struct tcb *tcp);
void ucDataFlowSemantics_kill(struct tcb *tcp);
void ucDataFlowSemantics_mmap(struct tcb *tcp);
void ucDataFlowSemantics_munmap(struct tcb *tcp);
void ucDataFlowSemantics_openat(struct tcb *tcp);
void ucDataFlowSemantics_open(struct tcb *tcp);
void ucDataFlowSemantics_pipe(struct tcb *tcp);
void ucDataFlowSemantics_read(struct tcb *tcp);
void ucDataFlowSemantics_rename(struct tcb *tcp);
void ucDataFlowSemantics_shutdown(struct tcb *tcp);
void ucDataFlowSemantics_socketpair(struct tcb *tcp);
void ucDataFlowSemantics_socket(struct tcb *tcp);
void ucDataFlowSemantics_splice(struct tcb *tcp);
void ucDataFlowSemantics_unlink(struct tcb *tcp);
void ucDataFlowSemantics_write(struct tcb *tcp);


#endif /* UC_SEMANTICS_H_ */
