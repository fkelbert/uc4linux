
#ifndef TRACEE_H_
#define TRACEE_H_

#include <stdlib.h>
#include <stdint.h>
#include <sys/user.h>

#include "constants.h"
#include "helpers.h"

#define SKIP_NO 0
#define SKIP_DESIRED 1
#define SKIP_ACTUAL 2

struct tracee_status {
	int in_out;
	int skipNext;
	int lastCall;
	long currentCall;
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
