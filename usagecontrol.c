#include "usagecontrol.h"

void notifyEventToPdp(event *ev) {
#if UC_JNI
	notifyEventToPdpJni(ev);
#elif UC_THRIFT
	notifyEventToPdpThrift(ev);
#else
	printf("Unknown option.\n");
#endif
}

void ucEnd() {
//	while(1);
}


void ucInit() {
#if UC_JNI
	ucInitJni();
	ucTypesInit();
#elif UC_THRIFT
	ucInitThrift();
	ucTypesInit();
#else
	printf("Unknown option.\n");
	exit(1);
#endif
}

bool ignoreExecve = true;

void notifySyscall(struct tcb *tcp) {
	if (!tcp || !tcp->s_ent || !tcp->s_ent->sys_name) {
		return;
	}

	/* It is kind of weird, that we need to use exiting()
	 * in this way here. My guess is, that execve (which is stopped
	 * three times in the beginning) makes this necessary.
	 * TODO: Handle this case more seriously;
	 * maybe use syscall_fixup_on_sysenter() in syscall.c
	 * This also needs to be fixed in ucPIP_main.c::ucPIP_update() then.
	 */
	bool actual = is_actual(tcp);

	uc_log("==== %3d (%05d, %c) %s ... ", tcp->scno, tcp->pid, actual ? 'A' : 'D', tcp->s_ent->sys_name);

	if (!ucSemanticsDefined(tcp->scno)) {
		uc_log("ignoring.\n");
		return;
	}

	event *ev = ucSemanticsFunct[tcp->scno](tcp);
	if (ev == NULL) {
		uc_log("returned NULL.\n");
		return;
	}

	/*
	 * Always signal these calls as actual event.
	 */
	switch (tcp->scno) {
		case SYS_execve:
			ignoreExecve = !ignoreExecve;
			if (ignoreExecve) {
				uc_log("ignoring.\n");
				return;
			}
			/* no break */
		case SYS_exit:
		case SYS_exit_group:
			ev->isActual = true;
			break;
		default:
			ev->isActual = actual;
	}

	uc_log("notifying PDP... ");
	notifyEventToPdp(ev);
	uc_log("done.");

	destroyEvent(ev);
	uc_log("\n");
}

