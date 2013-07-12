/*
 * ucPIP_main.c
 *
 *  Created on: Jul 2, 2013
 *      Author: Florian Kelbert
 */

#include "ucPIP_main.h"


struct timeval syscalltime;
FILE *outstream;

void ucPIP_main_init() {
	outstream = stdout;

#ifdef UC_SEMANTICS_H_
	ucSemantics__init();
#endif

#if defined(UC_DECLASS_ENABLED) && UC_DECLASS_ENABLED
	ucDeclass__init();
#endif

	ucPIP_init();

	ucPIP_addInitialData("/tmp/foo");
	ucPIP_addInitialData("/tmp/foo2");
	ucPIP_addInitialData("/home/www/nginx/html/index.html");
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

#if defined(UC_DECLASS_ENABLED) && UC_DECLASS_ENABLED
		gettimeofday(&syscalltime, NULL);
#endif

		// This call will update both (s(),l(),f()) and s+() as defined
		// for the corresponding system call in ucDataFlowSemantics.c
		ucSemanticsFunct[tcp->scno](tcp);

		ucPIP_printF();
		ucPIP_printS();
		ucPIP_printL();
		ucDeclass_printSPlus(tcp->pid);
	}
}


