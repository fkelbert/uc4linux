/*
 * ucSemantics.c
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#include "ucDataFlowSemantics.h"

#define IDENTIFIER_MAX_LEN 512

char identifier[IDENTIFIER_MAX_LEN];
char identifier2[IDENTIFIER_MAX_LEN];

char *getIdentifierFD(int pid, int fd, char *ident, int len) {
	snprintf(ident, len, "FD %dx%d", pid, fd);
	return (ident);
}

char *getIdentifierPID(int pid, char *ident, int len) {
	snprintf(ident, len, "PID %d", pid);
	return (ident);
}


void ucDataFlowSemanticsWrite(struct tcb *tcp) {
	printf("%d writes to %s\n",tcp->pid,identifier);

	ucPIP_copyData(getIdentifierPID(tcp->pid, identifier, sizeof(identifier)),
			getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier2, sizeof(identifier2)));
	ucPIP_printF();
	ucPIP_printS();
}

void ucDataFlowSemanticsRead(struct tcb *tcp) {
	printf("%d reads from %s\n",tcp->pid, identifier);
	ucPIP_copyData(getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)),
			getIdentifierPID(tcp->pid, identifier2, sizeof(identifier2)));
	ucPIP_printF();
	ucPIP_printS();
}

void ucDataFlowSemanticsExit(struct tcb *tcp) {
	getIdentifierPID(tcp->pid, identifier, sizeof(identifier));
	printf("exiting %s\n", identifier);
}

void ucDataFlowSemanticsExecve(struct tcb *tcp) {
	printf("UUUppdating execve\n");
}

void ucDataFlowSemanticsClose(struct tcb *tcp) {
	ucPIP_removeIdentifier(getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)));
}

void ucDataFlowSemanticsOpen(struct tcb *tcp) {
	char filename[FILENAME_MAX];

	if (tcp->u_rval < 0) {
		return;
	}

	// retrieve the filename
	if (!umovestr(tcp, tcp->u_arg[0], sizeof(filename), filename)) {
		filename[sizeof(filename) - 1] = '\0';
	}

	if (filename[0] == '\0') {
		return;
	}

	ucPIP_addIdentifier(filename, getIdentifierFD(tcp->pid, tcp->u_rval, identifier, sizeof(identifier)));

	ucPIP_printF();
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
