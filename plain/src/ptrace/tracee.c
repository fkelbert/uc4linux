
#include "tracee.h"

/**
 * Creates a tracee with the specified process id.
 *
 * @pid the tracees process id
 *
 * @return a pointer to the created tracee or NULL if creation failed.
 */
struct tracee *traceeCreate(pid_t pid) {
	struct tracee *tracee = (struct tracee *) calloc(1, sizeof(struct tracee));

	if (tracee == NULL) {
		return NULL;
	}

	tracee->pid = pid;
	tracee->status = SYSIN;
	tracee->command = strdup("COMMAND");	//  FIXME: put real command here
	tracee->user_info = (struct passwd *) getUserInfo(pid);

	return (tracee);
}



void traceeDestroy(struct tracee *tracee) {
	free(tracee->command);
	free(tracee);
}
