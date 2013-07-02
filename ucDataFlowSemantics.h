/*
 * ucSemantics.h
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#ifndef UC_SEMANTICS_H_
#define UC_SEMANTICS_H_

#include "defs.h"
#include "ucDeclass.h"
#include "ucPIP.h"
#include "syscall.h"

#include <sys/fcntl.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <glib.h>

#define UC_SEMANTICS_DEBUG 1

#define IDENTIFIER_MAX_LEN 512

#define IS_O_RDWR(flag)		((flag & O_RDWR) 	== O_RDWR)
#define IS_O_WRONLY(flag)	((flag & O_WRONLY) 	== O_WRONLY)
#define IS_O_TRUNC(flag)	((flag & O_TRUNC) 	== O_TRUNC)

#define ucSemantics_errorExit(msg) { \
			fprintf(stderr, "%s\n", msg); \
			fprintf(stderr, "Happened in: %s:%d\n", __FILE__, __LINE__); \
			fprintf(stderr, "Exiting.\n"); \
			exit (1); \
		}

extern int *procMem;

#define isAbsolute(string) (string && *string == '/')

#define streq(str1, str2) (strcmp(str1,str2) == 0)
#define strneq(str1, str2, n) (strncmp(str1,str2,n) == 0)

#define PROCFS_MNT "/proc"

#define isProcessFirstCall(pid) (procMem[pid] == 0)

#define ucSemanticsDefined(syscallno) (ucSemanticsFunct[syscallno] != NULL)

#define ucPIPupdateBefore(tcp) (tcp->s_ent->sys_func == sys_execve || tcp->s_ent->sys_func == sys_exit || tcp->s_ent->sys_func == sys_write)
#define ucPIPupdateAfter(tcp) (!(ucPIPupdateBefore(tcp)))

#define fdDup(fdPtr, value) if (!(fdPtr = malloc(sizeof(int)))) { ucDeclass_errorExitMemory();	} *fdPtr = value

void ucSemantics_do_close(pid_t pid, int fd);

void ucSemantics_accept(struct tcb *tcp);
void ucSemantics_clone(struct tcb *tcp);
void ucSemantics_close(struct tcb *tcp);
void ucSemantics_connect(struct tcb *tcp);
void ucSemantics_dup2(struct tcb *tcp);
void ucSemantics_dup(struct tcb *tcp);
void ucSemantics_eventfd(struct tcb *tcp);
void ucSemantics_execve(struct tcb *tcp);
void ucSemantics_exit(struct tcb *tcp);
void ucSemantics_exit_group(struct tcb *tcp);
void ucSemantics_fcntl(struct tcb *tcp);
void ucSemantics_ftruncate(struct tcb *tcp);
void ucSemantics_kill(struct tcb *tcp);
void ucSemantics_mmap(struct tcb *tcp);
void ucSemantics_munmap(struct tcb *tcp);
void ucSemantics_openat(struct tcb *tcp);
void ucSemantics_open(struct tcb *tcp);
void ucSemantics_pipe(struct tcb *tcp);
void ucSemantics_read(struct tcb *tcp);
void ucSemantics_rename(struct tcb *tcp);
void ucSemantics_shutdown(struct tcb *tcp);
void ucSemantics_socketpair(struct tcb *tcp);
void ucSemantics_socket(struct tcb *tcp);
void ucSemantics_splice(struct tcb *tcp);
void ucSemantics_unlink(struct tcb *tcp);
void ucSemantics_write(struct tcb *tcp);

void ucSemantics_cloneFirstAction(struct tcb *tcp);

extern void (*ucSemanticsFunct[])(struct tcb *tcp);

// Depending on whether debug mode is enabled or disabled, the
// following code either defines functions that get executed or
// replaces the corresponding by nothing
#if defined(UC_SEMANTICS_DEBUG) && UC_SEMANTICS_DEBUG
	#define ucSemantics_log(format, ...) ucSemantics_log_impl(format, ##__VA_ARGS__)
	#define ucSemantics_IGNORE ucSemantics_IGNORE_impl
	void ucSemantics_log_impl(const char* format, ...);
	void ucSemantics_IGNORE_impl(struct tcb *tcp);
#else
	#define ucSemantics_log(...)
	#define ucSemantics_IGNORE NULL
#endif

#endif /* UC_SEMANTICS_H_ */

