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


int ucDesired(struct tcb *tcp) {
	int retval = ucPDPask(tcp);

	switch(retval) {
		case UC_PDP_ALLOW:
			if (ucPIPupdateBefore(tcp)) {
				do_dft(tcp);
			}
			break;
		case UC_PDP_MODIFY:
			break;
		case UC_PDP_INHIBIT:
			break;
		case UC_PDP_DELAY:
			break;
	}

	return (retval);
}


int ucActual(struct tcb *tcp) {
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
			// TODO: does it make sense to modify/delay/inhibit
			// after the call has been executed???
			break;
	}

	return (retval);
}


/**
 * This function is used by strace to notify our
 * usage control framework about the fact that a system call
 * is happening. This call may be both desired or actual.
 *
 * If firstCall == 0, this method will invoke all
 * corresponding usage control functionality.
 * If firstCall == 1, then
 *
 *
 */
void notifySyscall(struct tcb *tcp) {
	if (!tcp || !ucSemanticsDefined(tcp->scno) || !tcp->s_ent || !tcp->s_ent->sys_name) {
		return;
	}

	if (isProcessFirstCall(tcp->pid)) {
		ucSemanticsFunct[SYS_cloneFirstAction](tcp);
	}

	/* It is kind of weird, that we need to use exiting()
	 * in this way here. My guess is, that execve (which is stopped
	 * three times in the beginning) makes this necessary.
	 * TODO: Handle this case more seriously; maybe use syscall_fixup_on_sysenter() in syscall.c
	 */
	if (exiting(tcp)) {
		ucDesired(tcp);
	}
	else {
		ucActual(tcp);
	}
}

