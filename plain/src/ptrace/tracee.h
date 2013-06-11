
#ifndef TRACEE_H_
#define TRACEE_H_

#include <stdlib.h>
#include <sys/user.h>

#include "constants.h"
#include "helpers.h"

struct tracee_status {
	int in_out;
	long syscallcode;
	struct user_regs_struct *regs;
};

struct tracee {
	pid_t pid;
	char *command;
	struct tracee_status *status;
	struct passwd *user_info;

};

struct tracee *traceeCreate(pid_t pid);
void traceeDestroy(struct tracee *tracee);

#endif /* TRACEE_H_ */
