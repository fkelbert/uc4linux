/*
 * ucSemantics.c
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#include "ucDataFlowSemantics.h"

void ucDataFlowSemanticsWrite(struct tcb *tcp) {
	printf("UUUppdating write\n");
}

void ucDataFlowSemanticsRead(struct tcb *tcp) {
	printf("UUUppdating read\n");
}

void ucDataFlowSemanticsExit(struct tcb *tcp) {
	printf("UUUppdating exit\n");
}

void ucDataFlowSemanticsExecve(struct tcb *tcp) {
	printf("UUUppdating execve\n");
}

void ucDataFlowSemanticsClose(struct tcb *tcp) {
	printf("UUUppdating close\n");
}

void ucDataFlowSemanticsOpen(struct tcb *tcp) {
	printf("UUUppdating open\n");
}

void ucPIPupdate(struct tcb *tcp) {
	// pointer to the function to execute in order to update the PIP
	void(*ucDataFlowSemanticsFunc)(struct tcb *tcp) = NULL;

	if (tcp->s_ent->sys_func == sys_write) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsWrite;
	}
	else if (tcp->s_ent->sys_func == sys_read) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsRead;
	}
	else if (tcp->s_ent->sys_func == sys_exit) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsExit;
	}
	else if (tcp->s_ent->sys_func == sys_execve) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsExecve;
	}
	else if (tcp->s_ent->sys_func == sys_close) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsClose;
	}
	else if (tcp->s_ent->sys_func == sys_open) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsOpen;
	}


	// finally, update the PIP
	if (ucDataFlowSemanticsFunc) {
		(*ucDataFlowSemanticsFunc)(tcp);
	}
}
