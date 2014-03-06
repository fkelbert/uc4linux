#include <defs.h>

#define ucSemantics_IGNORE NULL

extern void (*ucSemanticsFunct[])(struct tcb *tcp);

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

//void ucSemantics_cloneFirstAction(struct tcb *tcp);
