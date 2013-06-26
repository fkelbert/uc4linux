/*
 * ucSemantics.c
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#include "ucDataFlowSemantics.h"

#define IDENTIFIER_MAX_LEN 512

char identifier[IDENTIFIER_MAX_LEN];
char identifier2[IDENTIFIER_MAX_LEN];

#define ucSemantics_errorExit(msg) \
			fprintf(stderr, "%s\n", msg); \
			fprintf(stderr, "Happened in: %s:%d\n", __FILE__, __LINE__); \
			fprintf(stderr, "Exiting.\n"); \
			exit (1);

#define isAbsolute(string) *string == '/'


/**
 * Returns a list of open filedescriptors for the process identified with the specified process ID.
 * Memory for the result will be allocated by this function and needs to be freed by the caller.
 * @param pid the process id
 * @param count a pointer that will return the number of entries in the returned int*
 * @return an int-Array containing all open file descriptors of that process. The size of this array is returned in count.
 */
int *getListOfOpenFileDescriptors(long pid, int *count) {
	char procfsPath[PATH_MAX];
	DIR *dir;
	struct dirent *ent;
	int *fds;
	int size = 8;		// do not init to 0!
	*count = 0;
	int ret;

	ret = snprintf(procfsPath, sizeof(procfsPath), "/proc/%ld/fd", pid);

	if (ret >= sizeof(procfsPath)) {
		ucSemantics_errorExit("Buffer overflowed.");
	}
	else if (ret < 0) {
		ucSemantics_errorExit("Error while writing to buffer.");
	}

	if (!(fds = malloc(size * sizeof(int)))) {
		ucSemantics_errorExit("Unable to allocate memory.");
	}

	if ((dir = opendir(procfsPath))) {

		while ((ent = readdir (dir))) {

			if (*count + 1 == size) {
				size *= 2;
				if (!(fds = realloc(fds, size * sizeof(int)))) {
					ucSemantics_errorExit("Unable to allocate memory.");
				}
			}

			// this condition returns != 1, if the entry is not an integer,
			// in particular for directories . and ..
			if (sscanf(ent->d_name, "%d", (fds + *count)) == 1) {
				(*count)++;
			}
		}

	}
	else {
		ucSemantics_errorExit("Failed to open procfs directory");
	}

	return (fds);
}


/// FIXME: this will most likely fail for chrooted processes
char *fsAbsoluteFilename(long pid, char *relFilename, char *absFilename, int absFilenameLen) {
	ssize_t read;
	char *absNew;
	char procfsPath[PATH_MAX];
	char cwd[PATH_MAX];
	char concatPath[2 * PATH_MAX];

	// we are done!
	if (isAbsolute(relFilename)) {
		strncpy(absFilename, relFilename, absFilenameLen - 1);
		absFilename[absFilenameLen - 1] = '\0';
		return (absFilename);
	}

	if (absFilenameLen < 1) {
		return NULL ;
	}

	// get the processes' current working directory
	snprintf(procfsPath, sizeof(procfsPath), "/proc/%ld/cwd", pid);
	if ((read = readlink(procfsPath, cwd, sizeof(cwd) - 1)) == -1) {
		return NULL ;
	}
	cwd[read] = '\0';

	// concatenate cwd and relative filename and convert it to an absolute filename
	snprintf(concatPath, sizeof(concatPath), "%s/%s", cwd, relFilename);

	// was resolving successful and is the provided buffer large enough?
	if ((absNew = realpath(concatPath, NULL )) == NULL || strlen(absNew) >= absFilenameLen) {
		return NULL ;
	}

	// as we have tested the size before, an additional null byte is written by strncpy()
	strncpy(absFilename, absNew, absFilenameLen);

	free(absNew);

	return (absFilename);
}


/**
 * Makes an identifier out of the specified PIDxFD and returns it in ident of size len.
 * This function always returns ident.
 */
char *getIdentifierFD(int pid, int fd, char *ident, int len) {
	int ret = snprintf(ident, len, "FD %dx%d", pid, fd);

	if (ret >= len) {
		ucSemantics_errorExit("Buffer overflowed.");
	}
	else if (ret < 0) {
		ucSemantics_errorExit("Error while writing to buffer.");
	}

	return (ident);
}

/**
 * Makes an identifier out of the specified PID and returns it in ident of size len.
 * This function always returns ident.
 */
char *getIdentifierPID(int pid, char *ident, int len) {
	int ret = snprintf(ident, len, "PID %d", pid);

	if (ret >= len) {
		ucSemantics_errorExit("Buffer overflowed.");
	}
	else if (ret < 0) {
		ucSemantics_errorExit("Error while writing to buffer.");
	}

	return (ident);
}

// todo: write into aliases
void ucDataFlowSemanticsWrite(struct tcb *tcp) {
	printf("write(): %d --> %s\n",tcp->pid,identifier);

	if (tcp->u_arg[0] > 0) {
		ucPIP_copyData(
				getIdentifierPID(tcp->pid, identifier, sizeof(identifier)),
				getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier2, sizeof(identifier2)));
	}
}

// todo: write into aliases
void ucDataFlowSemanticsRead(struct tcb *tcp) {
	printf("read(): %d <-- %s\n",tcp->pid, identifier);

	if (tcp->u_arg[0] > 0) {
		ucPIP_copyData(
				getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)),
				getIdentifierPID(tcp->pid, identifier2, sizeof(identifier2)));
	}
}

void ucDataFlowSemanticsExit(struct tcb *tcp) {
	int count;
	int *openfds;

	getIdentifierPID(tcp->pid, identifier, sizeof(identifier));

	printf("exit(): %s\n", identifier);

	ucPIP_removeDataSet(identifier);
	ucPIP_removeIdentifier(identifier);

	// delete all of the processes' open file descriptors
	if ((openfds = getListOfOpenFileDescriptors(tcp->pid, &count))) {
		while (count-- > 0) {
			getIdentifierFD(tcp->pid, openfds[count], identifier, sizeof(identifier));
			ucPIP_removeIdentifier(identifier);
		}
		free(openfds);
	}

	// TODO: delete aliases
}

void ucDataFlowSemanticsExecve(struct tcb *tcp) {
	printf("UUUppdating execve\n");
}


// done
void ucDataFlowSemanticsClose(struct tcb *tcp) {
	getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier));

	printf("close(): %s\n", identifier);

	if (tcp->u_arg[0] > 0) {
		ucPIP_removeIdentifier(identifier);
	}
}


// done
void ucDataFlowSemanticsOpen(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX];

	// invalid return value
	if (tcp->u_rval < 0) {
		return;
	}

	// retrieve the filename
	if (!umovestr(tcp, tcp->u_arg[0], sizeof(relFilename), relFilename)) {
		relFilename[sizeof(relFilename) - 1] = '\0';
	}

	if (relFilename[0] == '\0') {
		return;
	}

	fsAbsoluteFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename));
	getIdentifierFD(tcp->pid, tcp->u_rval, identifier, sizeof(identifier));

	printf("open(): %d: %s --> %s\n",tcp->pid,absFilename,identifier);

	ucPIP_addIdentifier(absFilename, identifier);
}

void ucDataFlowSemanticsPipe(struct tcb *tcp) {
	int fds[2];

	if (tcp->u_rval == -1) {
		return;
	}

	if (umoven(tcp, tcp->u_arg[0], sizeof fds, (char *) fds) < 0) {
		return;
	}

	printf("pipe()\n");

	ucPIP_addIdentifier(
			getIdentifierFD(tcp->pid, fds[0], identifier, sizeof(identifier)),
			getIdentifierFD(tcp->pid, fds[1], identifier2, sizeof(identifier2)));
}

void ucDataFlowSemanticsDup(struct tcb *tcp) {
	if (tcp->u_rval > 0) {

		ucPIP_addIdentifier(
				getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)),
				getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2)));

		printf("dup(): %s --> %s\n", identifier, identifier2);
	}
}



void ucPIPupdate(struct tcb *tcp) {
	// pointer to the function to execute in order to update the PIP
	void(*ucDataFlowSemanticsFunc)(struct tcb *tcp) = NULL;

	// Note: Do not(!) compare function pointer. This will not work out,
	// e.g. for dup() which is mapped to the internal sys_open()!
	if (strcmp(tcp->s_ent->sys_name, "write") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsWrite;
	}
	else if (strcmp(tcp->s_ent->sys_name, "read") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsRead;
	}
	else if (strcmp(tcp->s_ent->sys_name, "exit") == 0 ||
			strcmp(tcp->s_ent->sys_name, "exit_group") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsExit;
	}
	else if (strcmp(tcp->s_ent->sys_name, "execve") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsExecve;
	}
	else if (strcmp(tcp->s_ent->sys_name, "close") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsClose;
	}
	else if (strcmp(tcp->s_ent->sys_name, "open") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsOpen;
	}
	else if (strcmp(tcp->s_ent->sys_name, "pipe")  == 0
		||	 strcmp(tcp->s_ent->sys_name, "pipe2") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsPipe;
	}
	else if (strcmp(tcp->s_ent->sys_name, "dup") == 0
		||	 strcmp(tcp->s_ent->sys_name, "dup2") == 0
		||	 strcmp(tcp->s_ent->sys_name, "dup3") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsDup;
	}


	// finally, update the PIP
	if (ucDataFlowSemanticsFunc) {
		(*ucDataFlowSemanticsFunc)(tcp);
	}
}
