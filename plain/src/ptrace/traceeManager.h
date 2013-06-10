
#ifndef PIDFDTABLE_H_
#define PIDFDTABLE_H_

#include <stddef.h>
#include <stdlib.h>
#include <glib/ghash.h>
#include "constants.h"
#include "helpers.h"

extern GHashTable *tracees;

struct tracee {
	pid_t pid;
	int status;
	char *command;
	struct passwd *user_info;
};

void tmInit();
void tmNewTracee(int pid);
void tmDeleteTracee(int pid);
struct tracee *tmGetTracee(int pid);
int tmIsEmpty();

#endif /* PIDFDTABLE_H_ */
