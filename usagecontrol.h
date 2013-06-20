#ifndef USAGECONTROL_H
#define USAGECONTROL_H


#include "defs.h"
#include "syscall.h"

#define UC_ENABLED
#define UC_PDP_ALLOW 1
#define UC_PDP_INHIBIT 2
#define UC_PDP_MODIFY 3
#define UC_PDP_DELAY 4
#define ucAskPDP(tcp) UC_PDP_ALLOW

void ucUpdatePIP(struct tcb *tcp);
int ucBeforeSyscallEnter(struct tcb *tcp);
int ucAfterSyscallExit(struct tcb *tcp);

#endif /* USAGECONTROL_H */
