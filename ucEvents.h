#ifndef UC_EVENTS_H
#define UC_EVENTS_H

#include <stdbool.h>
#include <sys/fcntl.h>
#include <limits.h>
#include <dirent.h>

#include "defs.h"
#include "ucTypes.h"

#define ucSemantics_IGNORE NULL
#define ucSemanticsDefined(syscallno) (ucSemanticsFunct[syscallno] != NULL)

#define PID_LEN 6
#define FD_LEN 6

#define toPid(str,len,i) snprintf(str,len,"%d",i);
#define toFd(str,len,i) snprintf(str,len,"%d",i);

#define IS_O_RDWR(flag)		((flag & O_RDWR) 	== O_RDWR)
#define IS_O_WRONLY(flag)	((flag & O_WRONLY) 	== O_WRONLY)
#define IS_O_TRUNC(flag)	((flag & O_TRUNC) 	== O_TRUNC)

extern event *(*ucSemanticsFunct[])(struct tcb *tcp);

event *ucSemantics_accept(struct tcb *tcp);
event *ucSemantics_clone(struct tcb *tcp);
event *ucSemantics_close(struct tcb *tcp);
event *ucSemantics_connect(struct tcb *tcp);
event *ucSemantics_dup2(struct tcb *tcp);
event *ucSemantics_dup(struct tcb *tcp);
event *ucSemantics_eventfd(struct tcb *tcp);
event *ucSemantics_execve(struct tcb *tcp);
event *ucSemantics_exit(struct tcb *tcp);
event *ucSemantics_exit_group(struct tcb *tcp);
event *ucSemantics_fcntl(struct tcb *tcp);
event *ucSemantics_ftruncate(struct tcb *tcp);
event *ucSemantics_kill(struct tcb *tcp);
event *ucSemantics_mmap(struct tcb *tcp);
event *ucSemantics_munmap(struct tcb *tcp);
event *ucSemantics_openat(struct tcb *tcp);
event *ucSemantics_open(struct tcb *tcp);
event *ucSemantics_pipe(struct tcb *tcp);
event *ucSemantics_read(struct tcb *tcp);
event *ucSemantics_rename(struct tcb *tcp);
event *ucSemantics_shutdown(struct tcb *tcp);
event *ucSemantics_socketpair(struct tcb *tcp);
event *ucSemantics_socket(struct tcb *tcp);
event *ucSemantics_splice(struct tcb *tcp);
event *ucSemantics_unlink(struct tcb *tcp);
event *ucSemantics_write(struct tcb *tcp);

//event *ucSemantics_cloneFirstAction(struct tcb *tcp);

// defined in strace:net.c
extern const struct xlat domains[];
extern const struct xlat socktypes[];

#endif
