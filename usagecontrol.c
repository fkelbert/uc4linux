#include "defs.h"
#include "usagecontrol.h"
#include "syscall.h"


void ucUpdatePIP(struct tcb *tcp) {
	int (*func)() = tcp->s_ent->sys_func;

	/* weird. See comment below on sys_execve*/
	if (exiting(tcp)) {
		if (func == sys_write) {
			printf("PIP update %s\n", tcp->s_ent->sys_name);
		}
	}
	else {
		if (func == sys_write) {
			printf("after\n");
		}
	}
}

// Kelbert
int ucBeforeSyscallEnter(struct tcb *tcp) {
	int retval = ucAskPDP(tcp);
	switch(retval) {
		case UC_PDP_ALLOW:
		case UC_PDP_MODIFY:	// modify assumes that the syscall has already been modified transparently		
			ucUpdatePIP(tcp);
			break;
		case UC_PDP_INHIBIT:
			// TODO: write code
			break;
		case UC_PDP_DELAY:
			// TODO: write code
			break;
	}

	return retval;
}

// Kelbert
int ucAfterSyscallExit(struct tcb *tcp) {
	int retval = ucAskPDP(tcp);

	switch(retval) {
		case UC_PDP_ALLOW:		
			ucUpdatePIP(tcp);
			break;
		case UC_PDP_DELAY:
		case UC_PDP_MODIFY:
		case UC_PDP_INHIBIT:
			// TODO: does it make sense to modify/delay/inhibit after the call has been executed???
			break;
	}

	return retval;
}
