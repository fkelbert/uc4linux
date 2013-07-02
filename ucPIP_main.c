/*
 * ucPIP_main.c
 *
 *  Created on: Jul 2, 2013
 *      Author: Florian Kelbert
 */

#include "ucPIP_main.h"

void ucPIP_main_init() {
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

void ucPIP_update(struct tcb *tcp) {
	if (!ucSemanticsDefined(tcp->scno)) {
		return;
	}

	if (isProcessFirstCall(tcp->pid)) {
		ucSemanticsFunct[SYS_cloneFirstAction](tcp);
	}

	if ((exiting(tcp) && ucPIPupdateBefore(tcp))
			|| (!exiting(tcp) && ucPIPupdateAfter(tcp))) {
		// This call will update both (s(),l(),f()) and s+() as defined
		// for the corresponding system call in ucDataFlowSemantics.c
		printf("%s\n",tcp->s_ent->sys_name);fflush(stdout);
		ucSemanticsFunct[tcp->scno](tcp);

		ucPIP_printF(stdout);
		ucPIP_printS(stdout);
	}
}


