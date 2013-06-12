/**
 * @file  testpdp.c
 * @brief Exemplary execution test for native PDP
 *
 * @author Adrià Puigdomènech
 **/

#include <sys/stat.h>
#include <unistd.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <sys/user.h>
#include <string.h>
#include <stdlib.h>
#include <mxml.h>
#include "syscall_handler.h"

char *policyFileName = "/home/user/Desktop/running/policies/pdpPolicy.xml";
char* defaultPolicyFile =
		"/home/user/Desktop/running/policies/defaultPolicy.xml";
char* APPLICATION_PATH() {
	return "/usr/bin/gnome-terminal";
}

/*
 * Signal handler for SIGUSR2 signal.
 *
 * @param sig_num The id of the received signal.
 *
 */
void handle_usrsignal(int sig_num) {
	/* Reset for next time */
	signal(SIGUSR2, handle_usrsignal);
	/* Call to init again to load the file */
	initDefaultAnswerTable(defaultPolicyFile);
	log_info("Reloaded the default policy information.");
}

/*
 * Prints the usage of this program.
 */
void printUsage() {
	fprintf(stderr,
			"This program should be executed the following way:\n"
					"testpdp [program] [stop]\nAll arguments are optional and indicate"
					"if we want a specific program to be used or we want to stop at every"
					"captured event.");
}

#define SYSIN 0
#define SYSOUT 1
#define SYSEXECVE 2

int main(int argc, char **argv) {
	pid_t child;
	long newpid;
	long orig_eax, aux, aux2;
	int status, i, caller_id, action, direct;
	/* Arguments for the execve command */
	char *newargv[] = { APPLICATION_PATH(), NULL };
	int ADDRESS_SIZE = 4;
	char *newenviron[] = { NULL };
	/* Struct that contains the information related to an event (system call) */
	event_ptr event = NULL;
	/* Struct that gather all the registers of the computer */
	struct user_regs_struct regs;
	notifyResponse_ptr response;
	struct inCall *inc;

	int exitSignal = 0;
	log_warn("Starting native Test-PDP");
	int ret = pdpStart();
	log_trace("PDP started with result=[%s]", returnStr[ret]);

	ret = pdpDeployPolicy(policyFileName);
	log_trace("deploy returned=[%d]", ret);
	/* Decide whether the user wants to stop or not */
	int stopping = 0;
	/* Parse the arguments */
	char *programToExecute;
	if (argc > 1) {
		if (strcmp(argv[1], "stop")) {
			programToExecute = strdup(argv[1]);
		} else {
			programToExecute = strdup("/usr/bin/gnome-terminal");
			stopping = 1;
			if (argc > 2) {
				printUsage();
				exit(-1);
			}
		}
		if (argc > 2) {
			stopping = 1;
			if (argc > 3) {
				printUsage();
				exit(-1);
			}
		}
	} else {
		programToExecute = strdup("/usr/bin/gnome-terminal");
	}
	/* Set the handler */
	signal(SIGUSR2, handle_usrsignal);
	log_info("The process id is %d", getpid());
	getchar();
	/* Forking the process */
	child = fork();
	if (child == 0) {
		/* Child process */
		ptrace(PTRACE_TRACEME, 0, NULL, NULL);
		/* Execute the program specified */
		//execve(APPLICATION_PATH(),newargv, newenviron);
		execl(programToExecute, 0);
		//execl("/home/kosklain/tests/helloWorld/helloWorld", 0);
		//execl(Constants::APPLICATION_PATH(), Constants::APPLICATION_PATH(), NULL);
	} else {
		/* Initialize all the hash tables */
		initSyscallTable();
		initializeFDTable();
		initializePidTable();
		initDefaultAnswerTable(defaultPolicyFile);
		//create_pid(child);
		/* Parent process */
		caller_id = child;
		/* The executing command of the main program is the first
		 * program we want to execute */
		create_command(caller_id, programToExecute);
		/* Paranoia */
		change_command_value(caller_id, programToExecute);
		while (1) {
			caller_id = waitforCall(caller_id, -1);
			if (caller_id == -1)
				break;
			inc = get_pid_value(caller_id);
			/* EXECVE stops three times, and this is the second time */
			if (inc->inCall == SYSEXECVE) {
				inc->inCall = SYSOUT;
				continue;
			}
			if (inc->inCall == SYSIN) {
				/* It is a inCall */
				/* Get the system call code of the child process */
				orig_eax = ptrace(PTRACE_PEEKUSER, caller_id,
						ADDRESS_SIZE * ORIG_EAX, NULL);
				/* Parse the arguments of the system call */
				event = parseSyscall(caller_id, orig_eax, 1);
				if (event != NULL) {
					log_info("FK event\n");

					// we need to track the data flow BEFORE
					// the system call is executed in the following cases
					if (DATA_FLOW_ENABLED) {
						log_info("FK DATA_FLOW_ENABLED\n");
						switch (orig_eax) {
						case SYS_execve:
						case SYS_exit:
						case SYS_write:
							log_info("FK IF_update\n");
							IF_update(event);
							break;
						}
					}

					/* If it is one of the recognized system calls,
					 * ask the pdp for a response or prepare a response,
					 * depending on the value in our hash table.
					 */
					action = syscallAction(orig_eax);
					if (action == ACTION_ASK) {
						response = pdpNotifyEvent(event);
						direct = 0;
					} else {
						response = prepareResponse(event, action);
						direct = 1;
					}

					/* Do the actions needed before continuing */
					processResponseBefore(response, caller_id, direct);
					inc->eventResponse = response;

//					// TODO 2013/06/07, FK: is this really needed?
//					if (DATA_FLOW_ENABLED && get_deny_because_of_if()) {
//						printf("Syscall denied because of IF's requirements violation\n");
//						reset_deny_because_of_if();
//					}

				} else {
					inc->eventResponse = NULL;
				}
				/* Next time we will see the return of the system call */
				inc->inCall = SYSOUT;
				inc->code = orig_eax;
				/* SYS_exit does not come back! */
				if (orig_eax == SYS_exit) {
					inc->inCall = SYSIN;
				}
				/* Execve stops three times, let us skip next time */
				if (orig_eax == SYS_execve) {
					inc->inCall = SYSEXECVE;
				}
				change_pid_value(caller_id, inc);
			} else {
				/* It is the exit of a system call */
				/* If it was one of the parsed events,
				 * we process the given pdp response. */
				if (inc->eventResponse != NULL) {
					orig_eax = ptrace(PTRACE_PEEKUSER, caller_id,
							ADDRESS_SIZE * ORIG_EAX, NULL);
					if (syscallAction(orig_eax) == ACTION_ASK) {
						processResponseAfter(response, caller_id, 0);
					} else {
						processResponseAfter(response, caller_id, 1);
					}
					//notifyResponseFree(inc->eventResponse);
				}
				inc->inCall = SYSIN;

				change_pid_value(caller_id, inc);
				if (stopping == 1) {
					getchar();
				}
			}
		}
	}

	log_warn("stopping PDP=[%d]\n", pdpStop());

	return 0;
}

