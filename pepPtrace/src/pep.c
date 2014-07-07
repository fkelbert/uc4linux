#include "pep.h"

char int_to_str_buf[BUFLEN_INT];

void setTraceeStatus(struct tracee *tracee) {
	tracee->status->currentCall = ptrace(PTRACE_PEEKUSER, tracee->pid,
			MULT4(ORIG_EAX), NULL);
	ptrace(PTRACE_GETREGS, tracee->pid, NULL, tracee->status->regs);
}

/**
 * This is the main monitor loop.
 * Wait for intercepted system calls.
 */
void run() {
	int veryFirstCall = 1;
	int pid;
	int status;
	struct tracee *tracee;
	event_ptr event;

	while (1) {
		// wait for child's signal
		pid = waitpid(-1, &status, __WALL);

		if (PTRACE_ONLY) {
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			continue;
		}

		// FIXME: skip uninteresting syscalls as soon as possible

		if (pid == -1) {
			perror(__func__);
			exit(1);
		}

		// If the child exited, we stop tracing the process
		if (WIFEXITED(status) || WIFSIGNALED(status)) {
			if (pid == -1) {
				perror(__func__);
				exit(1);
			}
			tmDeleteTracee(pid);
			if (tmIsEmpty()) {
				exit(0);
			}
			continue;
		}

		// ignore if child is running
		if (!WIFSTOPPED(status)) {
			continue;
		}

		// new processes start with a SIGSTOP;
		// do some initialization
		if (WSTOPSIG(status) == SIGSTOP) {
			tmNewTracee(pid);
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			continue;
		}

		if ((tracee = tmGetTracee(pid)) == NULL) {
			tracee = tmNewTracee(pid);
		}

		// rely on tracee->status only from here
		setTraceeStatus(tracee);

		if (tracee->status->currentCall < 0) {
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			continue;
		}

		// on fork: attach to the new process
		if (tracee->status->currentCall == SYS_clone
				|| tracee->status->currentCall == SYS_fork
				|| tracee->status->currentCall == SYS_vfork) {

			// the parent's return code is the child's pid
			if (tracee->status->regs->eax > 0) {
				ptrace(PTRACE_ATTACH, tracee->status->regs->eax, NULL, NULL);
			}
		}


		// Skip the next desired event
		// (important for execve which is intercepted three times)
		if (tracee->status->skipNext == SKIP_DESIRED
				&& tracee->status->in_out == TRACEE_STATUS_IN) {

			switch (tracee->status->in_out) {
				case TRACEE_STATUS_OUT:
					tracee->status->in_out = TRACEE_STATUS_IN;
					break;
				case TRACEE_STATUS_IN:
					tracee->status->in_out = TRACEE_STATUS_OUT;
					break;
			}

			tracee->status->skipNext = SKIP_NO;
			ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
			continue;
		}

		// parse system call
		// FIXME do only parse if we are interested at this point
		// (i.e.: consider variable in/out)

		if (DO_INTERCEPT(tracee->status->currentCall)) {

			// the very first call makes some trouble:
			// if SYS_execve is intercepted, then it is NOT a desired call
			if (INTERCEPT_SYS_execve && veryFirstCall) {
				tracee->status->in_out = TRACEE_STATUS_OUT;
				veryFirstCall = 0;
			}

			event = eventCreate();

			int_to_str(pid, int_to_str_buf, BUFLEN_INT);
			eventAddParam(event, EVENT_PARAM_PID, int_to_str_buf);
			eventAddParam(event, EVENT_PARAM_COMMAND, tracee->command);
			eventAddParam(event, EVENT_PARAM_USER, tracee->user_info->pw_name);
			eventAddParam(event, EVENT_PARAM_SYSCALL, SYSCALLNAME(tracee->status->currentCall));
			eventAddParam(event, EVENT_PARAM_DESIRED,
					(tracee->status->in_out == TRACEE_STATUS_IN) ?
							EVENT_VALUE_TRUE : EVENT_VALUE_FALSE);

			parseSyscall(event, tracee->pid, &tracee->status->currentCall,
					tracee->status->regs);
			eventPrint(event);

			switch (tracee->status->in_out) {
				case TRACEE_STATUS_IN:
					// syscall call (into kernel)

					// FIXME: syscall handling happens here
					switch (tracee->status->currentCall) {
						case SYS_execve:
							traceeChangeCommand(tracee,
									eventGetParam(event, EVENT_PARAM_FILENAME));
							break;
					}

					// post processing
					switch (tracee->status->currentCall) {
						case SYS_exit:
							// SYS_exit does not come back
							tracee->status->in_out = TRACEE_STATUS_IN;
							break;

						case SYS_execve:
							// Execve stops three times, skip the next
							// desired time (the child executing exec)
							tracee->status->in_out = TRACEE_STATUS_OUT;
							tracee->status->skipNext = SKIP_DESIRED;
							break;

						default:
							// Next time we will see the return of the system call
							tracee->status->in_out = TRACEE_STATUS_OUT;
							break;
					}

					break;
				case TRACEE_STATUS_OUT:
					// syscall response (into userland)

					// FIXME: handle the call
					tracee->status->in_out = TRACEE_STATUS_IN;
					break;
			}

			eventDestroy(event);
		}
		else {
			printf("untracked call %s\n", SYSCALLNAME(tracee->status->currentCall));
		}

		// continue the intercepted process
		ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
	}

}

int main(int argc, char **argv) {
	pid_t child;

	child = fork();

	if (child == -1) {
		printf("Fork failed. Unable to start initial monitored child.");
		exit(1);
	}

	if (child == 0) {
		ptrace(PTRACE_TRACEME, 0, NULL, PTRACE_USED_OPTIONS);
		execl(MONITORED_APPLICATION, MONITORED_APPLICATION, NULL);
	}

// Child has been executed.
// The monitor is on its own ...

	pdpConnectorInit();
	pdpConnectorDeployPolicy();

	tmInit();

	run();
	return (0);
}

