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


/**
 * Converts the relative filename relFilename to an absolute filename. The current working directory (cwd) of the specified
 * process with process ID pid is taken as a baseline. Basically, cwd of pid and the relative filename are
 * concatenated and the result is canonicalized. mustExist specifies whether the resulting absolute file must in fact exist.
 *
 * The resulting absolute filename is written into absFilename of length absFilenameLen.
 * The address of absFilename is returned, NULL on error.
 *
 * @param pid the cwd of this process is taken as a baseline to build the absolute filename
 * @param relFilename the filename relative to cwd
 * @param absFilename a pre-allocated memory area which will return the result
 * @param absFilenameLen the length of absFilename
 * @param mustExist whether the resolved absolute filename must exist. 1 if it must, 0 if it may not.
 * @return the address of absFilename
 */
/// FIXME: this will most likely fail for chrooted processes
char *fsAbsoluteFilename(long pid, char *relFilename, char *absFilename, int absFilenameLen, int mustExist) {
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
	if (!(absNew = realpath(concatPath, NULL )) || strlen(absNew) >= absFilenameLen) {
		if (errno == ENOENT && !mustExist) {

			// In some cases the file we asked for may be missing and still we want to know its absolute filename (e.g. for sys_rename())
			// In this case we canonicalize the path to the file and append the filename manually as follows:

			int lastSlashIndex = strlen(concatPath) - 1;
			while (concatPath[lastSlashIndex] != '/') {
				lastSlashIndex--;
			}
			if (lastSlashIndex < 0) {
				return NULL;
			}


			char concatPath2[2 * PATH_MAX];
			strncpy(concatPath2, concatPath, lastSlashIndex);
			concatPath2[lastSlashIndex] = '\0';

			if (!(absNew = realpath(concatPath2, NULL )) || strlen(absNew) + strlen(concatPath + lastSlashIndex) >= absFilenameLen) {
				return NULL;
			}

			strncpy(absFilename, absNew, absFilenameLen);
			strncpy(absFilename + strlen(absFilename), concatPath + lastSlashIndex, absFilenameLen - strlen(absFilename));
		}
		else {
			return NULL ;
		}
	}
	else {
		strncpy(absFilename, absNew, absFilenameLen);
	}

	if (absNew) {
		free(absNew);
	}

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
	if (tcp->u_arg[0] < 0) {
		return;
	}

	ucPIP_copyData(
			getIdentifierPID(tcp->pid, identifier, sizeof(identifier)),
			getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier2, sizeof(identifier2)));

	printf("write(): %s --> %s\n",identifier, identifier2);
}

// todo: write into aliases
void ucDataFlowSemanticsRead(struct tcb *tcp) {
	if (tcp->u_arg[0] < 0) {
		return;
	}

	ucPIP_copyData(
			getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)),
			getIdentifierPID(tcp->pid, identifier2, sizeof(identifier2)));

	printf("read(): %d <-- %s\n",tcp->pid, identifier);
}

void ucDataFlowSemanticsExit(struct tcb *tcp) {
	int count;
	int *openfds;

	getIdentifierPID(tcp->pid, identifier, sizeof(identifier));

	ucPIP_removeContainer(identifier);
	ucPIP_removeIdentifier(identifier);

	// delete all of the processes' open file descriptors
	if ((openfds = getListOfOpenFileDescriptors(tcp->pid, &count))) {
		while (count-- > 0) {
			getIdentifierFD(tcp->pid, openfds[count], identifier, sizeof(identifier));

			// check whether this is the last identifier for that container
			// if it is, then also remove the container and its storage
			// TODO: This operation is expensive, because countIdentifiers() loops over all identifiers
			if (ucPIP_countIdentifiers(identifier) == 1) {
				ucPIP_removeContainer(identifier);
			}

			ucPIP_removeIdentifier(identifier);
		}
		free(openfds);
	}

	printf("exit(): %s\n", identifier);

	// TODO: delete aliases
}

void ucDataFlowSemanticsExecve(struct tcb *tcp) {
	// TODO: man 2 execve
	// Also consider man 2 open and fcntl: some file descriptors close automatically on exeve()
}


// done
void ucDataFlowSemanticsClose(struct tcb *tcp) {
	if (tcp->u_arg[0] < 0) {
		return;
	}

	ucPIP_removeIdentifier(getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)));

	printf("close(): %s\n", identifier);
}


// TODO: handle truncation flag
void ucDataFlowSemanticsOpen(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX];

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

	ucPIP_addIdentifier(
		fsAbsoluteFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename), 1),
		getIdentifierFD(tcp->pid, tcp->u_rval, identifier, sizeof(identifier)));

	printf("open(): %d: %s --> %s\n",tcp->pid,absFilename,identifier);
}



void ucDataFlowSemanticsOpenat(struct tcb *tcp) {
	// TODO. man 2 openat
}

void ucDataFlowSemanticsSocket(struct tcb *tcp) {
	// TODO. man 2 socket
}

void ucDataFlowSemanticsSocketpair(struct tcb *tcp) {
	// TODO. man 2 socketpair
}

void ucDataFlowSemanticsFcntl(struct tcb *tcp) {
	// TODO. man 2 fcntl64
}

void ucDataFlowSemanticsShutdown(struct tcb *tcp) {
	// TODO. man 2 shutdown
}

void ucDataFlowSemanticsEventfd(struct tcb *tcp) {
	// TODO. man 2 eventfd
}

void ucDataFlowSemanticsMmap(struct tcb *tcp) {
	// TODO. man 2 mmap
}

void ucDataFlowSemanticsKill(struct tcb *tcp) {
	// TODO. man 2 kill
}

void ucDataFlowSemanticsAccept(struct tcb *tcp) {
	// TODO. man 2 accept
}

void ucDataFlowSemanticsConnect(struct tcb *tcp) {
	// TODO. man 2 connect
}

void ucDataFlowSemanticsRename(struct tcb *tcp) {
	char oldRelFilename[FILENAME_MAX];
	char newRelFilename[FILENAME_MAX];
	char oldAbsFilename[FILENAME_MAX];
	char newAbsFilename[FILENAME_MAX];

	if (tcp->u_rval < 0) {
		return;
	}

	// retrieve old filename
	if (!umovestr(tcp, tcp->u_arg[0], sizeof(oldRelFilename), oldRelFilename)) {
		oldRelFilename[sizeof(oldRelFilename) - 1] = '\0';
	}
	if (oldRelFilename[0] == '\0') {
		return;
	}

	// retrieve new filename
	if (!umovestr(tcp, tcp->u_arg[1], sizeof(newRelFilename), newRelFilename)) {
		newRelFilename[sizeof(newRelFilename) - 1] = '\0';
	}
	if (newRelFilename[0] == '\0') {
		return;
	}

	printf("old rel filename: %s\n",oldRelFilename);
	fsAbsoluteFilename(tcp->pid, oldRelFilename, oldAbsFilename, sizeof(oldAbsFilename), 0);
	fsAbsoluteFilename(tcp->pid, newRelFilename, newAbsFilename, sizeof(newAbsFilename), 1);

	ucPIP_removeContainer(newAbsFilename);
	ucPIP_addIdentifier(oldAbsFilename, newAbsFilename);
	ucPIP_removeIdentifier(oldAbsFilename);

	printf("rename(): %s --> %s\n", oldAbsFilename, newAbsFilename);
}

void ucDataFlowSemanticsClone(struct tcb *tcp) {
	// TODO. man 2 clone
}

void ucDataFlowSemanticsFtruncate(struct tcb *tcp) {
	// TODO. man 2 ftruncate; do sth if length == 0
}

void ucDataFlowSemanticsUnlink(struct tcb *tcp) {
	// TODO. man 2 unlink
}

void ucDataFlowSemanticsSplice(struct tcb *tcp) {
	// TODO. man 2 splice
}

void ucDataFlowSemanticsMunmap(struct tcb *tcp) {
	// TODO. man 2 munmap
	// is it possible to do something useful here?
}



void ucDataFlowSemanticsPipe(struct tcb *tcp) {
	int fds[2];

	if (tcp->u_rval < 0) {
		return;
	}

	if (umoven(tcp, tcp->u_arg[0], sizeof fds, (char *) fds) < 0) {
		return;
	}

	ucPIP_addIdentifier(
			getIdentifierFD(tcp->pid, fds[0], identifier, sizeof(identifier)),
			getIdentifierFD(tcp->pid, fds[1], identifier2, sizeof(identifier2)));

	printf("pipe(): %s %s\n",identifier, identifier2);
}

void ucDataFlowSemanticsDup(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	ucPIP_addIdentifier(
			getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)),
			getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2)));

	printf("dup(): %s --> %s\n", identifier, identifier2);
}



void ucPIPupdate(struct tcb *tcp) {
	// pointer to the function to execute in order to update the PIP
	void(*ucDataFlowSemanticsFunc)(struct tcb *tcp) = NULL;

	// Note: Do not(!) compare function pointer. This will not work out,
	// e.g. for dup() which is mapped to the internal sys_open()!
	if (strcmp(tcp->s_ent->sys_name, "write") == 0
		||	 strcmp(tcp->s_ent->sys_name, "writev") == 0
		||	 strcmp(tcp->s_ent->sys_name, "pwrite") == 0
		||	 strcmp(tcp->s_ent->sys_name, "pwritev") == 0
		||	 strcmp(tcp->s_ent->sys_name, "pwrite64") == 0
		||	 strcmp(tcp->s_ent->sys_name, "send") == 0
		||	 strcmp(tcp->s_ent->sys_name, "sendto") == 0
		||	 strcmp(tcp->s_ent->sys_name, "sendmsg") == 0
		||	 strcmp(tcp->s_ent->sys_name, "sendmmsg") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsWrite;
	}
	else if (strcmp(tcp->s_ent->sys_name, "read") == 0
		||	 strcmp(tcp->s_ent->sys_name, "readv") == 0
		||	 strcmp(tcp->s_ent->sys_name, "pread") == 0
		||	 strcmp(tcp->s_ent->sys_name, "pread64") == 0
		||	 strcmp(tcp->s_ent->sys_name, "preadv") == 0
		||	 strcmp(tcp->s_ent->sys_name, "recv") == 0
		||	 strcmp(tcp->s_ent->sys_name, "recvfrom") == 0
		||	 strcmp(tcp->s_ent->sys_name, "recvmsg") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsRead;
	}
	else if (strcmp(tcp->s_ent->sys_name, "exit") == 0 ||
			strcmp(tcp->s_ent->sys_name, "_exit") == 0 ||
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
	else if (strcmp(tcp->s_ent->sys_name, "openat") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsOpenat;
	}
	else if (strcmp(tcp->s_ent->sys_name, "socket") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsSocket;
	}
	else if (strcmp(tcp->s_ent->sys_name, "socketpair") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsSocketpair;
	}
	else if (strcmp(tcp->s_ent->sys_name, "accept") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsAccept;
	}
	else if (strcmp(tcp->s_ent->sys_name, "connect") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsConnect;
	}
	else if (strcmp(tcp->s_ent->sys_name, "fcntl64") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsFcntl;
	}
	else if (strcmp(tcp->s_ent->sys_name, "shutdown") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsShutdown;
	}
	else if (strcmp(tcp->s_ent->sys_name, "splice") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsSplice;
	}
	else if (strcmp(tcp->s_ent->sys_name, "rename") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsRename;
	}
	else if (strcmp(tcp->s_ent->sys_name, "eventfd2") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsEventfd;
	}
	else if (strcmp(tcp->s_ent->sys_name, "unlink") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsUnlink;
	}
	else if (strcmp(tcp->s_ent->sys_name, "kill") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsKill;
	}
	else if (strcmp(tcp->s_ent->sys_name, "munmap") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsMunmap;
	}
	else if (strcmp(tcp->s_ent->sys_name, "clone") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsClone;
	}
	else if (strcmp(tcp->s_ent->sys_name, "ftruncate") == 0
		||	 strcmp(tcp->s_ent->sys_name, "ftruncate64") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsFtruncate;
	}
	else if (strcmp(tcp->s_ent->sys_name, "mmap")  == 0
		||	 strcmp(tcp->s_ent->sys_name, "mmap2") == 0) {
		ucDataFlowSemanticsFunc = ucDataFlowSemanticsMmap;
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

	// TODO: sys_fstatfs may be useful to find out information about mounted file systems


	// finally, update the PIP
	if (ucDataFlowSemanticsFunc) {
		(*ucDataFlowSemanticsFunc)(tcp);
	}
	else {
		// this calls have been checked to not influence data flow.
		// they can be ignored
		if (strcmp(tcp->s_ent->sys_name, "brk") != 0 &&

			// stat family & file meta operations
			strcmp(tcp->s_ent->sys_name, "stat64") != 0 &&
			strcmp(tcp->s_ent->sys_name, "statfs64") != 0 &&
			strcmp(tcp->s_ent->sys_name, "statfs") != 0 &&
			strcmp(tcp->s_ent->sys_name, "fstatfs") != 0 &&
			strcmp(tcp->s_ent->sys_name, "fstat64") != 0 &&
			strcmp(tcp->s_ent->sys_name, "lstat64") != 0 &&
			strcmp(tcp->s_ent->sys_name, "chmod") != 0 &&
			strcmp(tcp->s_ent->sys_name, "chown32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "umask") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getxattr") != 0 &&
			strcmp(tcp->s_ent->sys_name, "lgetxattr") != 0 &&
			strcmp(tcp->s_ent->sys_name, "fgetxattr") != 0 &&
			strcmp(tcp->s_ent->sys_name, "readlink") != 0 &&
			strcmp(tcp->s_ent->sys_name, "lseek") != 0 &&
			strcmp(tcp->s_ent->sys_name, "fsync") != 0 &&
			strcmp(tcp->s_ent->sys_name, "utime") != 0 &&
			strcmp(tcp->s_ent->sys_name, "flock") != 0 &&
			strcmp(tcp->s_ent->sys_name, "symlink") != 0 &&
			strcmp(tcp->s_ent->sys_name, "faccessat") != 0 &&

			// directory meta operations
			strcmp(tcp->s_ent->sys_name, "chdir") != 0 &&
			strcmp(tcp->s_ent->sys_name, "mkdir") != 0 &&
			strcmp(tcp->s_ent->sys_name, "rmdir") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getcwd") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getdents") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getdents64") != 0 &&

			// user IDs, group IDs, ...
			strcmp(tcp->s_ent->sys_name, "getuid32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getgid32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "geteuid32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getegid32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getresuid32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getresgid32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setresuid32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setresgid32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setuid32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setgid32") != 0 &&

			// time
			strcmp(tcp->s_ent->sys_name, "clock_getres") != 0 &&
			strcmp(tcp->s_ent->sys_name, "clock_gettime") != 0 &&
			strcmp(tcp->s_ent->sys_name, "clock_settime") != 0 &&
			strcmp(tcp->s_ent->sys_name, "gettimeofday") != 0 &&
			strcmp(tcp->s_ent->sys_name, "time") != 0 &&
			strcmp(tcp->s_ent->sys_name, "times") != 0 &&

			// socket operations & information
			strcmp(tcp->s_ent->sys_name, "bind") != 0 &&
			strcmp(tcp->s_ent->sys_name, "listen") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setsockopt") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getsockopt") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getpeername") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getsockname") != 0 &&

			// system limits
			strcmp(tcp->s_ent->sys_name, "getrlimit") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setrlimit") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getprlimit64") != 0 &&

			// signal handling
			strcmp(tcp->s_ent->sys_name, "alarm") != 0 &&
			strcmp(tcp->s_ent->sys_name, "sigreturn") != 0 &&
			strcmp(tcp->s_ent->sys_name, "sigaltstack") != 0 &&
			strcmp(tcp->s_ent->sys_name, "rt_sigreturn") != 0 &&
			strcmp(tcp->s_ent->sys_name, "rt_sigaction") != 0 &&
			strcmp(tcp->s_ent->sys_name, "rt_sigprocmask") != 0 &&	// handling this call may slightly reduce overapproximations, because less signals get delivered

			// kernel advises
			strcmp(tcp->s_ent->sys_name, "fadvise64") != 0 &&
			strcmp(tcp->s_ent->sys_name, "madvise") != 0 &&

			// wait
			strcmp(tcp->s_ent->sys_name, "wait4") != 0 &&		// TODO: Actually, upon wait we get the return value of the child. Therefore,
			strcmp(tcp->s_ent->sys_name, "waitpid") != 0 &&		// we should handle these calls. But they will most likely kill data flow tracking due to overapproximations
			strcmp(tcp->s_ent->sys_name, "select") != 0 &&
			strcmp(tcp->s_ent->sys_name, "poll") != 0 &&

			// thread management
			strcmp(tcp->s_ent->sys_name, "gettid") != 0 &&
			strcmp(tcp->s_ent->sys_name, "capset") != 0 &&
			strcmp(tcp->s_ent->sys_name, "capget") != 0 &&
			strcmp(tcp->s_ent->sys_name, "get_robust_list") != 0 &&
			strcmp(tcp->s_ent->sys_name, "set_robust_list") != 0 &&
			strcmp(tcp->s_ent->sys_name, "set_thread_area") != 0 &&
			strcmp(tcp->s_ent->sys_name, "set_tid_address") != 0 &&

			// process & system operations
			strcmp(tcp->s_ent->sys_name, "getpid") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getppid") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getpgid") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getpgrp") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getrusage") != 0 &&
			strcmp(tcp->s_ent->sys_name, "prctl") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getpriority") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setpriority") != 0 &&
			strcmp(tcp->s_ent->sys_name, "sysinfo") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setgroups") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getgroups") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getgroups32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setgroups32") != 0 &&
			strcmp(tcp->s_ent->sys_name, "getsid") != 0 &&
			strcmp(tcp->s_ent->sys_name, "setsid") != 0 &&
			strcmp(tcp->s_ent->sys_name, "sched_getaffinity") != 0 &&
			strcmp(tcp->s_ent->sys_name, "sched_setaffinity") != 0 &&

			// shared memory
			strcmp(tcp->s_ent->sys_name, "shmdt") != 0 &&		// TODO: we should handle them!
			strcmp(tcp->s_ent->sys_name, "shmat") != 0 &&
			strcmp(tcp->s_ent->sys_name, "shmctl") != 0 &&
			strcmp(tcp->s_ent->sys_name, "shmget") != 0 &&

			// inotify(7)
			strcmp(tcp->s_ent->sys_name, "inotify_init") != 0 &&
			strcmp(tcp->s_ent->sys_name, "inotify_init1") != 0 &&
			strcmp(tcp->s_ent->sys_name, "inotify_rm_watch") != 0 &&
			strcmp(tcp->s_ent->sys_name, "inotify_add_watch") != 0 &&

			// epoll(7)
			strcmp(tcp->s_ent->sys_name, "epoll_create") != 0 &&
			strcmp(tcp->s_ent->sys_name, "epoll_create1") != 0 &&
			strcmp(tcp->s_ent->sys_name, "epoll_ctl") != 0 &&
			strcmp(tcp->s_ent->sys_name, "epoll_wait") != 0 &&

			// misc.
			strcmp(tcp->s_ent->sys_name, "quotactl") != 0 &&
			strcmp(tcp->s_ent->sys_name, "readahead") != 0 &&
			strcmp(tcp->s_ent->sys_name, "access") != 0 &&
			strcmp(tcp->s_ent->sys_name, "fallocate") != 0 &&
			strcmp(tcp->s_ent->sys_name, "mprotect") != 0 &&
			strcmp(tcp->s_ent->sys_name, "futex") != 0 &&
			strcmp(tcp->s_ent->sys_name, "uname") != 0 &&
			strcmp(tcp->s_ent->sys_name, "_llseek") != 0 &&
			strcmp(tcp->s_ent->sys_name, "ioctl") != 0 &&
			strcmp(tcp->s_ent->sys_name, "restart_syscall") != 0
			) {
			printf("unhandled %s\n", tcp->s_ent->sys_name);
		}

	}
}
