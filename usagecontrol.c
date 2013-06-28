#include "usagecontrol.h"

void ucInit() {
	ucDataFlowSemantics__init();
	ucPIP_init();

	ucPIP_addInitialData("/tmp/foo");
}

// Kelbert
int ucBeforeSyscallEnter(struct tcb *tcp) {
	int retval = ucPDPask(tcp);

	switch(retval) {
		case UC_PDP_ALLOW:
		case UC_PDP_MODIFY:	// modify assumes that the syscall has already been modified transparently		
			if (ucPIPupdateBefore(tcp)) {
				ucPIPupdate(tcp);
				ucPIP_printF();
				ucPIP_printS();
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
				ucPIPupdate(tcp);
				ucPIP_printF();
				ucPIP_printS();
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
