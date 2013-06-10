
#ifndef TRACEE_H_
#define TRACEE_H_

#include <stdlib.h>

#include "constants.h"
#include "helpers.h"

struct tracee {
	pid_t pid;
	int status;
	char *command;
	struct passwd *user_info;
};

struct tracee *traceeCreate(pid_t pid);
void traceeDestroy(struct tracee *tracee);

#endif /* CONSTANTS_H_ */
