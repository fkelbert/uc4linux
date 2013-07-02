	#ifndef USAGECONTROL_H
#define USAGECONTROL_H


#include "defs.h"
#include "ucSyscall.h"
#include "ucPIP.h"
#include "ucDataFlowSemantics.h"
#include "ucDeclass.h"

#define UC_ENABLED 1
#define UC_DEBUG_MODE 1
#define UC_PERFORMANCE_MODE !(UC_DEBUG_MODE)

#define UC_PDP_ALLOW 1
#define UC_PDP_INHIBIT 2
#define UC_PDP_MODIFY 3
#define UC_PDP_DELAY 4
#define ucPDPask(tcp) UC_PDP_ALLOW



#define ucPIPupdateBefore(tcp) (tcp->s_ent->sys_func == sys_execve || tcp->s_ent->sys_func == sys_exit || tcp->s_ent->sys_func == sys_write)
#define ucPIPupdateAfter(tcp) (!(ucPIPupdateBefore(tcp)))

#define ucHandleSyscall(syscallno) (ucSemanticsFunct[syscallno] != NULL)


void ucInit();

int ucBeforeSyscallEnter(struct tcb *tcp);
int ucAfterSyscallExit(struct tcb *tcp);




#endif /* USAGECONTROL_H */
