#ifndef UC_EVENTS_H
#define UC_EVENTS_H

#include <stdbool.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <limits.h>
#include <dirent.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <errno.h>

#include "defs.h"
#include "ucTypes.h"
#include "ucSettings.h"

#define ucSemantics_IGNORE NULL
#define ucSemanticsDefined(syscallno) (ucSemanticsFunct[syscallno] != NULL)

#define PID_LEN 6
#define FD_LEN 6

#define toPid(str,i) snprintf(str,PID_LEN,"%u", (unsigned int)i)
#define toFd(str,i) snprintf(str,FD_LEN,"%u", (unsigned int)i)
#define toString(str,tcp,arg)	{ if (!umovestr(tcp, arg, sizeof(str), str)) { \
											str[sizeof(str) - 1] = '\0';\
									} }

#define isAbsolutePath(string) (string && *string == '/')

#define IS_FLAG_SET(field,flag)	((field & flag) == flag)

extern event *(*ucSemanticsFunct[])(struct tcb *tcp);

event *ucSemantics_accept(struct tcb *tcp);
event *ucSemantics_clone(struct tcb *tcp);
event *ucSemantics_close(struct tcb *tcp);
event *ucSemantics_connect(struct tcb *tcp);
event *ucSemantics_dup2(struct tcb *tcp);
event *ucSemantics_dup(struct tcb *tcp);
event *ucSemantics_execve(struct tcb *tcp);
event *ucSemantics_exit(struct tcb *tcp);
event *ucSemantics_exit_group(struct tcb *tcp);
event *ucSemantics_fork(struct tcb *tcp);
event *ucSemantics_fcntl(struct tcb *tcp);
event *ucSemantics_ftruncate(struct tcb *tcp);
event *ucSemantics_kill(struct tcb *tcp);
event *ucSemantics_mmap(struct tcb *tcp);
event *ucSemantics_munmap(struct tcb *tcp);
event *ucSemantics_open(struct tcb *tcp);
event *ucSemantics_pipe(struct tcb *tcp);
event *ucSemantics_read(struct tcb *tcp);
event *ucSemantics_rename(struct tcb *tcp);
event *ucSemantics_sendfile(struct tcb *tcp);
event *ucSemantics_shutdown(struct tcb *tcp);
event *ucSemantics_socketpair(struct tcb *tcp);
event *ucSemantics_socket(struct tcb *tcp);
event *ucSemantics_splice(struct tcb *tcp);
event *ucSemantics_tee(struct tcb *tcp);
event *ucSemantics_truncate(struct tcb *tcp);
event *ucSemantics_unlink(struct tcb *tcp);
event *ucSemantics_write(struct tcb *tcp);

// defined in strace:net.c
extern const struct xlat domains[];
extern const struct xlat socktypes[];

#endif
