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
//	char filename[FILENAME_MAX];
//
//	// retrieve the filename
//	umoven(tcp, tcp->u_arg[0], sizeof(filename), filename);
//	filename[sizeof(filename) - 1] = '\0';
//
//	if (filename[0] == '\0') {
//		return;
//	}

	if (tcp->u_rval >= 0) {
		char identifier[20];
		snprintf(identifier, 20, "%dx%ld", tcp->pid, tcp->u_rval);

		ucPIP_f_add(identifier, NULL);
		printf("%s --> %d\n", identifier, ucPIP_f_get(identifier));
	}
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
