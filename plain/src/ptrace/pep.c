#include "pep.h"

char int_to_str_buf[BUFLEN_INT];


void setTraceeStatus(struct tracee *tracee) {
	tracee->status->syscallcode = ptrace(PTRACE_PEEKUSER, tracee->pid,
			MULT4(ORIG_EAX), NULL);
	ptrace(PTRACE_GETREGS, tracee->pid, NULL, tracee->status->regs);
}

/**
 * This is the main monitor loop.
 * Wait for intercepted system calls.
 */
void run() {
	int pid;
	int status;
	struct tracee *tracee;
	event_ptr event;

	while (1) {
		// wait for child's signal
		pid = waitpid(-1, &status, __WALL);

		if (!PTRACE_ONLY) {

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

			if (tracee->status->syscallcode < 0) {
				ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
				continue;
			}

			// on fork: attach to the new process
			if (tracee->status->syscallcode == SYS_clone
					|| tracee->status->syscallcode == SYS_fork
					|| tracee->status->syscallcode == SYS_vfork) {

				// the parent's return code is the child's pid
				if (tracee->status->regs->eax > 0) {
					ptrace(PTRACE_ATTACH, tracee->status->regs->eax, NULL,
							NULL);
				}
			}

			// EXECVE stops three times, and this is the second time
			if (tracee->status->in_out == SYSSKIP) {
				tracee->status->in_out = SYSOUT;
				ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
				continue;
			}

			// parse system call
			// FIXME do only parse if we are interested at this point
			// (i.e.: consider variable in/out)
			event = eventCreate();

			int_to_str(pid,int_to_str_buf,BUFLEN_INT);
			eventAddParam(event, "pid", int_to_str_buf);
			eventAddParam(event, "command", tracee->command);
			eventAddParam(event, "user", tracee->user_info->pw_name);
			eventAddParam(event, "syscall",
					syscallTable[tracee->status->syscallcode]);
			eventAddParam(event, "desired",
					(tracee->status->in_out == SYSIN) ? "true" : "false");

			parseSyscall(event, tracee->pid, &tracee->status->syscallcode,
					tracee->status->regs);
			eventPrint(event);

			switch (tracee->status->in_out) {
			case SYSIN:
				// syscall call (into kernel)

				// FIXME: handle the call

				switch (tracee->status->syscallcode) {
				case SYS_exit:
					// SYS_exit does not come back
					tracee->status->in_out = SYSIN;
					break;
				case SYS_execve:
					// Execve stops three times, let us skip next time
					tracee->status->in_out = SYSSKIP;
					break;
				default:
					// Next time we will see the return of the system call
					tracee->status->in_out = SYSOUT;
					break;
				}

				break;
			case SYSOUT:
				// syscall response (into userland)

				// FIXME: handle the call

				tracee->status->in_out = SYSIN;
				break;
			}

			eventDestroy(event);
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
		return (1);
	}

	if (child == 0) {
		// FIXME: We need to wait here for a signal of the monitor that everything has been started up
		ptrace(PTRACE_TRACEME, 0, NULL, PTRACE_USED_OPTIONS);
		execl(MONITORED_APPLICATION, MONITORED_APPLICATION, NULL);
	}

	// Child has been executed.
	// The monitor is on its own ...

	pdp_init();
	pdp_deployPolicy();

	tmInit();
	tmNewTracee(child);

	run();
	return (0);
}

