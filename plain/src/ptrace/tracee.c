
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

	if (tracee != NULL) {
		tracee->pid = pid;
		tracee->command = strdup("COMMAND");	//  FIXME: put real command here

		tracee->user_info = (struct passwd *) getUserInfo(pid);
		if (tracee->user_info == NULL) {
			free(tracee);
			return NULL;
		}

		tracee->status = calloc(1, sizeof(struct tracee_status));
		if (tracee->status == NULL) {
			free(tracee);
			return NULL;
		}

		tracee->status->regs = calloc(1, sizeof(struct user_regs_struct));
		if (tracee->status->regs == NULL) {
			free(tracee->status);
			free(tracee);
			return NULL;
		}

		tracee->status->in_out = SYSIN;
	}

	return (tracee);
}



void traceeDestroy(struct tracee *tracee) {
	if (tracee) {
		free(tracee->command);
		free(tracee->status->regs);
		free(tracee->status);
		free(tracee);
	}
}
