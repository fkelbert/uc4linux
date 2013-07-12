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
//	ucPIP_addInitialData("/tmp/foo2");
	ucPIP_addInitialData("/home/www/nginx/html/index.html");
//
//	ucPIP_addIdentifier("/tmp/foo", "a");
//
//	ucPIP_addIdentifier("b", NULL);
//	ucPIP_addIdentifier("c", NULL);
//	ucPIP_addIdentifier("d", NULL);
//	ucPIP_addIdentifier("e", NULL);
//	ucPIP_addIdentifier("f", NULL);
//	ucPIP_addIdentifier("g", NULL);
//
//
//	ucPIP_addAlias("b", "c");
//	ucPIP_addAlias("b", "d");
//	ucPIP_addAlias("b", "e");
//
//	ucPIP_addAlias("c", "f");
//	ucPIP_addAlias("d", "g");
//	ucPIP_addAlias("e", "g");
//
//	ucPIP_addAlias("b", "b");
//
//	ucPIP_printF();
//	ucPIP_printS();
//	ucPIP_printL();
//
//	ucPIP_copyData("a", "b", UC_COPY_INTO_ALL, NULL);
//
//
//
//	ucPIP_printF();
//	ucPIP_printS();
//	ucPIP_printL();
//
//	exit(1);
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


