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

	if (tracee) {
		// the value of res will be useless, but it will be NULL in case one allocation failed
		uintptr_t res = 0xFFFFFF;

		res &= (uintptr_t) (tracee->command = calloc(1, 512));
		res &= (uintptr_t) (tracee->user_info = (struct passwd *) getUserInfo(
				pid));
		res &= (uintptr_t) (tracee->status = calloc(1,
				sizeof(struct tracee_status)));

		if (res) {
			res &= (uintptr_t) (tracee->status->regs = calloc(1,
					sizeof(struct user_regs_struct)));
		}

		if (res) {
			tracee->pid = pid;
			tracee->status->in_out = SYS_STATUS_IN;
			tracee->status->skipNext = 0;
			getCmdline(pid, tracee->command, 512);
		}
		else {
			// at least one allocation went wrong
			if (!tracee->status->regs) {
				free(tracee->status->regs);
			}

			if (!tracee->status) {
				free(tracee->status);
			}

			if (!tracee->user_info) {
				free(tracee->user_info);
			}

			if (!tracee->command) {
				free(tracee->command);
			}
		}
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
