#include "usagecontrol.h"


void ucInit() {
#ifdef UC_SEMANTICS_H_
	ucSemantics__init();
#endif

#ifdef UC_DECLASS_H_
	ucDeclass__init();
#endif

	ucPIP_init();

	ucPIP_addInitialData("/tmp/foo");
	ucPIP_addInitialData("/tmp/foo2");
}

void do_dft(struct tcb *tcp) {
	if (ucSemanticsFunct[tcp->scno]) {
		ucSemanticsFunct[tcp->scno](tcp);
		ucPIP_printF();
		ucPIP_printS();

	}
	else {
//					fprintf(stderr, "Unhandled %s (%ld)\n", tcp->s_ent->sys_name, tcp->scno);
	}
}

// Kelbert
int ucBeforeSyscallEnter(struct tcb *tcp) {
	int retval = ucPDPask(tcp);

	switch(retval) {
		case UC_PDP_ALLOW:
		case UC_PDP_MODIFY:	// modify assumes that the syscall has already been modified transparently		
			if (ucPIPupdateBefore(tcp)) {
				do_dft(tcp);
			}
			break;
		case UC_PDP_INHIBIT:
			// TODO: write code
			break;
		case UC_PDP_DELAY:
			// TODO: write code
			break;
	}

	return (retval);
}

// Kelbert
int ucAfterSyscallExit(struct tcb *tcp) {
	int retval = ucPDPask(tcp);

	switch(retval) {
		case UC_PDP_ALLOW:		
			if (ucPIPupdateAfter(tcp)) {
				do_dft(tcp);
			}
			break;
		case UC_PDP_DELAY:
		case UC_PDP_MODIFY:
		case UC_PDP_INHIBIT:
			// TODO: does it make sense to modify/delay/inhibit after the call has been executed???
			break;
	}

	return (retval);
}


void notifyNewProcess(struct tcb *tcp) {
	ucSemanticsFunct[SYS_cloneFirstAction](tcp);
}

void notifySyscall(struct tcb *tcp) {
	if (tcp
#if UC_PERFORMANCE_MODE
			&& ucHandleSyscall(tcp->scno)
#endif
			&& tcp->s_ent && tcp->s_ent->sys_name) {
		/* It is kind of weird, that we need to use exiting()
		 * in this way here. My guess is, that execve (which is stopped
		 * three times in the beginning) makes this necessary.
		 * TODO: Handle this case more seriously; maybe use syscall_fixup_on_sysenter() in syscall.c
		 */
		if (exiting(tcp)) {
			ucBeforeSyscallEnter(tcp);
		}
		else {
			ucAfterSyscallExit(tcp);
		}
	}
}

