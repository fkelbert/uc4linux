
#ifndef TRACEE_H_
#define TRACEE_H_

#include <pwd.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/user.h>

#include "procfsUtils.h"


#define TRACEE_STATUS_IN 0
#define TRACEE_STATUS_OUT 1

#define SKIP_NO 0
#define SKIP_DESIRED 1
#define SKIP_ACTUAL 2

struct tracee_status {
	int in_out;
	int skipNext;
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

/**
 * Changes the command associated to the specified tracee.
 * Returns a pointer to the tracee, or NULL if changing failed.
 */
struct tracee *traceeChangeCommand(struct tracee *tracee, char *newcmd);

#endif /* TRACEE_H_ */
