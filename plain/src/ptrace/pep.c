#include "pep.h"

/**
 * This is the main monitor loop.
 * Wait for intercepted system calls.
 */
void run() {
	int pid;
	int status;
	struct tracee *tracee;
	long syscallnr;
	struct user_regs_struct regs;

	while (1) {
		// wait for child's signal
		pid = waitpid(-1, &status, __WALL);

		if (!PTRACE_ONLY) {

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

			syscallnr = ptrace(PTRACE_PEEKUSER, pid, MULT4(ORIG_EAX), NULL);

			if ((tracee = tmGetTracee(pid)) == NULL) {
				tmNewTracee(pid);
			}

			// on fork: attach to the new process
			if (syscallnr == SYS_clone || syscallnr == SYS_fork
					|| syscallnr == SYS_vfork) {

				// the parent's return code is the child's pid
				ptrace(PTRACE_GETREGS, pid, NULL, &regs);
				if (regs.eax > 0) {
					ptrace(PTRACE_ATTACH, regs.eax, NULL, NULL);
				}
			}

			printf("sc %d\n", pid);

			switch (tracee->status) {
			case SYSSKIP:
				// EXECVE stops three times, and this is the second time
				tracee->status = SYSOUT;
				break;
			case SYSIN:
				// syscall call (into kernel)

				// FIXME: handle the call

				switch (syscallnr) {
				case SYS_exit:
					// SYS_exit does not come back
					tracee->status = SYSIN;
					break;
				case SYS_execve:
					// Execve stops three times, let us skip next time
					tracee->status = SYSSKIP;
					break;
				default:
					// Next time we will see the return of the system call
					tracee->status = SYSOUT;
					break;
				}

				break;
			case SYSOUT:
				// syscall response (into userland)

				// FIXME: handle the call

				tracee->status = SYSIN;
				break;
			}
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

