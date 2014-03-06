#ifndef UC_EVENTS_H
#define UC_EVENTS_H

#include <stdbool.h>

#include "defs.h"
#include "ucTypes.h"

#define ucSemantics_IGNORE NULL
#define ucSemanticsDefined(syscallno) (ucSemanticsFunct[syscallno] != NULL)

#define PID_LEN 8
#define FD_LEN 8

#define toPid(str,len,i) snprintf(str,len,"%d",i);
#define toFd(str,len,i) snprintf(str,len,"%d",i);

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

#endif
