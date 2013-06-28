/*
 * ucSemantics.c
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#include "ucDataFlowSemantics.h"

#define IDENTIFIER_MAX_LEN 512

#define IS_O_RDWR(flag)		((flag & O_RDWR) 	== O_RDWR)
#define IS_O_WRONLY(flag)	((flag & O_WRONLY) 	== O_WRONLY)
#define IS_O_TRUNC(flag)	((flag & O_TRUNC) 	== O_TRUNC)

char identifier[IDENTIFIER_MAX_LEN];
char identifier2[IDENTIFIER_MAX_LEN];

#define ucSemantics_errorExit(msg) \
			fprintf(stderr, "%s\n", msg); \
			fprintf(stderr, "Happened in: %s:%d\n", __FILE__, __LINE__); \
			fprintf(stderr, "Exiting.\n"); \
			exit (1);

#define isAbsolute(string) *string == '/'

#define streq(str1, str2) (strcmp(str1,str2) == 0)

int *procMem;
int initialProcess = 1;

// FIXME reading from /proc/pid/mem seems to be much faster than reading using ptrace().
// Still, the code below may still have some flaws:
// When reading shared libraries, the old code for getString() returns something containing ELF, while this code returns garbage.
// Therefore we seem to have problems with binary content
char *getString(struct tcb *tcp, long addr, char *dataStr, int len_buf) {
	char filename[FILENAME_MAX];
	int fd;

	memset(dataStr, 0, len_buf);

	if (len_buf < 1) {
		return (dataStr);
	}

	// by default, the following block tries to read from the file descriptor procMem[pid],
	// which maps to /proc/<pid>/mem
	fd = procMem[tcp->pid];
	int r = 0;
	int pos = 0;
	int toread = len_buf - 1;
	do {
		pos += r;
		r = pread(fd, dataStr + pos, toread, addr + pos);
		toread -= r;
	}
	while (r > 0 && toread > 0);

	// if (for whatever reason) reading from /proc/<pid>/mem fails, then
	// fallback to a function provided by strace
	if (r <= 0) {
		printf("getstr fallback\n");
		if (!umovestr(tcp, addr, len_buf, dataStr)) {
			dataStr[len_buf - 1] = '\0';
		}
	}
	else {

		printf("getstr usual\n");
	}

	// safety
	dataStr[len_buf - 1] = '\0';

	return (dataStr);
}

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
	int size = 8; // do not init to 0!
	*count = 0;
	int ret;

	// TODO some test showed that this is slow! We should rather keep an internal mapping and avoid read from procfs (e.g. pid --> open fds)
	// difference for 1000000 executions: 2700ms (readlink /proc/self/fd/3) vs. 180ms (g_hash_table_insert() + g_hash_table_lookup())
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

		while ((ent = readdir(dir))) {

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

void addProcMem(int pid) {
	if (procMem[pid] == 0) {
		char foo[512];
		snprintf(foo, 512, "/proc/%d/mem", pid);
		procMem[pid] = open(foo, O_RDONLY);
	}
}

void removeProcMem(int pid) {
	close(procMem[pid]);
	procMem[pid] = 0;
}


/**
 * Returns the current working directory (cwd) of the process identified by the specified process id pid.
 * The result is written into buffer cwd of length len.
 * The address of cwd is returned, NULL on error.
 *
 * @param pid the process id
 * @param cwd the provided buffer in which the cwd is returned
 * @param len the length of the buffer
 * @result address of cwd on success, NULL on error
 */
char *getCwd(long pid, char *cwd, int len) {
	ssize_t read;
	char tmp[PATH_MAX];

	if (len <= 1) {
		return NULL;
	}

	snprintf(tmp, sizeof(tmp), "/proc/%ld/cwd", pid);
	if ((read = readlink(tmp, cwd, sizeof(cwd) - 1)) == -1) {
		return NULL ;
	}
	cwd[read] = '\0';

	return (cwd);
}



/**
 * Converts the relative filename relFilename to an absolute filename. The specified directory dir
 * is taken as a baseline. Basically, dirand the relative filename are
 * concatenated and the result is canonicalized. mustExist specifies whether the resulting absolute file must in fact exist.
 *
 * The resulting absolute filename is written into absFilename of length absFilenameLen.
 * The address of absFilename is returned, NULL on error.
 *
 * @param dir the baseline directory to build the absolute filename
 * @param relFilename the filename relative to cwd
 * @param absFilename a pre-allocated memory area which will return the result
 * @param absFilenameLen the length of absFilename
 * @param mustExist whether the resolved absolute filename must exist. 1 if it must, 0 if it may not.
 * @return the address of absFilename
 */
/// FIXME: this will most likely fail for chrooted processes
char *getAbsoluteFilename(char *dir, char *relFilename, char *absFilename, int absFilenameLen, int mustExist) {
	char *absNew;
	char procfsPath[PATH_MAX];
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

	// concatenate cwd and relative filename and convert it to an absolute filename
	snprintf(concatPath, sizeof(concatPath), "%s/%s", dir, relFilename);

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
				return NULL ;
			}

			char concatPath2[2 * PATH_MAX];
			strncpy(concatPath2, concatPath, lastSlashIndex);
			concatPath2[lastSlashIndex] = '\0';

			if (!(absNew = realpath(concatPath2, NULL )) || strlen(absNew) + strlen(concatPath + lastSlashIndex) >= absFilenameLen) {
				return NULL ;
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


char *cwdAbsoluteFilename(long pid, char *relFilename, char *absFilename, int absFilenameLen, int mustExist) {
	char cwd[PATH_MAX];

	if (!getCwd(pid, cwd, sizeof(cwd))) {
		return NULL;
	}

	getAbsoluteFilename(cwd, relFilename, absFilename, absFilenameLen, mustExist);

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
void ucDataFlowSemantics_write(struct tcb *tcp) {
	if (tcp->u_arg[0] <= 0) {
		// if return value is 0, nothing was written.
		return;
	}

	ucPIP_copyData(getIdentifierPID(tcp->pid, identifier, sizeof(identifier)), getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier2, sizeof(identifier2)));

	printf("write(): %s --> %s\n", identifier, identifier2);
}

// todo: write into aliases
void ucDataFlowSemantics_read(struct tcb *tcp) {
	if (tcp->u_arg[0] <= 0) {
		// if return value is 0, nothing was written.
		return;
	}

	ucPIP_copyData(getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)), getIdentifierPID(tcp->pid, identifier2, sizeof(identifier2)));

	printf("read(): %d <-- %s\n", tcp->pid, identifier);
}

void ucDataFlowSemantics_exit(struct tcb *tcp) {
	int count;
	int *openfds;

	getIdentifierPID(tcp->pid, identifier, sizeof(identifier));

	ucPIP_removeAllAliasesFrom(identifier);
	ucPIP_removeAllAliasesTo(identifier);

	ucPIP_removeIdentifier(identifier);

	// delete all of the processes' open file descriptors
	if ((openfds = getListOfOpenFileDescriptors(tcp->pid, &count))) {
		while (count-- > 0) {
			getIdentifierFD(tcp->pid, openfds[count], identifier, sizeof(identifier));
			ucPIP_removeIdentifier(identifier);
		}
		free(openfds);
	}


	removeProcMem(tcp->pid);

	printf("exit(): %d\n", tcp->pid);
}

void ucDataFlowSemantics_execve(struct tcb *tcp) {
	if (initialProcess) {
		printf("execve() initial\n");
		addProcMem(tcp->pid);

		getIdentifierPID(tcp->pid, identifier, sizeof(identifier));

		ucPIP_addIdentifier(identifier, NULL);

		initialProcess = 0;
	}
	// TODO: man 2 execve
	// Remember that execve returns 3 times!
	// Also consider man 2 open and fcntl: some file descriptors close automatically on exeve()
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
}



// done
void ucDataFlowSemantics_close(struct tcb *tcp) {
	if (tcp->u_arg[0] < 0) {
		return;
	}

	ucPIP_removeIdentifier(getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)));

	printf("close(): %s\n", identifier);
}


void ucDataFlowSemantics_open(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX];
	char *trunkstr = "";

	if (tcp->u_rval < 0) {
		return;
	}

	// retrieve the filename
	getString(tcp, tcp->u_arg[0], relFilename, sizeof(relFilename));

	cwdAbsoluteFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename), 1);
	getIdentifierFD(tcp->pid, tcp->u_rval, identifier, sizeof(identifier));

	// handle truncation flag
	if (IS_O_TRUNC(tcp->u_arg[1]) && (IS_O_RDWR(tcp->u_arg[1]) || IS_O_WRONLY(tcp->u_arg[1]))) {
		ucPIP_removeIdentifier(absFilename);
		trunkstr = "(truncated)";
	}

	ucPIP_addIdentifier(absFilename, identifier);

	printf("open(): %s %d: %s --> %s\n", trunkstr, tcp->pid, absFilename, identifier);
}

void ucDataFlowSemantics_openat(struct tcb *tcp) {
	// TODO. man 2 openat
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
}

void ucDataFlowSemantics_socket(struct tcb *tcp) {
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 socket
}

void ucDataFlowSemantics_socketpair(struct tcb *tcp) {
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 socketpair
}

void ucDataFlowSemantics_fcntl(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	if (tcp->u_arg[1] == F_DUPFD
			|| tcp->u_arg[1] == F_DUPFD_CLOEXEC) {
		getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier));
		getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2));

		ucPIP_addIdentifier(identifier, identifier2);

		printf("fcntl(): %s --> %s\n", identifier, identifier2);
	}
}

void ucDataFlowSemantics_shutdown(struct tcb *tcp) {
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 shutdown
}

void ucDataFlowSemantics_eventfd(struct tcb *tcp) {
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 eventfd
}

void ucDataFlowSemantics_mmap(struct tcb *tcp) {
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 mmap
}

void ucDataFlowSemantics_kill(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	ucPIP_copyData(getIdentifierPID(tcp->pid, identifier, sizeof(identifier)), getIdentifierPID(tcp->u_arg[0], identifier2, sizeof(identifier2)));

	printf("kill(): %s --> %s\n", identifier, identifier2);
}

void ucDataFlowSemantics_accept(struct tcb *tcp) {
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 accept
}

void ucDataFlowSemantics_connect(struct tcb *tcp) {
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 connect
}

void ucDataFlowSemantics_rename(struct tcb *tcp) {
	char oldRelFilename[FILENAME_MAX];
	char newRelFilename[FILENAME_MAX];
	char oldAbsFilename[FILENAME_MAX];
	char newAbsFilename[FILENAME_MAX];

	if (tcp->u_rval < 0) {
		return;
	}

	getString(tcp,tcp->u_arg[0],oldRelFilename, sizeof(oldRelFilename));
	getString(tcp,tcp->u_arg[1],newRelFilename, sizeof(newRelFilename));

	cwdAbsoluteFilename(tcp->pid, oldRelFilename, oldAbsFilename, sizeof(oldAbsFilename), 0);
	cwdAbsoluteFilename(tcp->pid, newRelFilename, newAbsFilename, sizeof(newAbsFilename), 1);

	ucPIP_removeContainer(newAbsFilename);
	ucPIP_addIdentifier(oldAbsFilename, newAbsFilename);
	ucPIP_removeIdentifier(oldAbsFilename);

	printf("rename(): %s --> %s\n", oldAbsFilename, newAbsFilename);
}

void ucDataFlowSemantics_clone(struct tcb *tcp) {
	int count;
	int *fds;

	if (tcp->u_rval < 0) {
		return;
	}

	addProcMem(tcp->u_rval);

	getIdentifierPID(tcp->pid, identifier, sizeof(identifier));
	getIdentifierPID(tcp->u_rval, identifier2, sizeof(identifier2));

	ucPIP_addIdentifier(identifier2, NULL);

	// copy all the data to the new process
	ucPIP_copyData(identifier, identifier2);

	// clone all the aliases
	ucPIP_copyAliases(identifier, identifier2);
	ucPIP_alsoAlias(identifier, identifier2);

	// copy all file descriptors
	fds = getListOfOpenFileDescriptors(tcp->pid, &count);
	while (--count >= 0) {
		getIdentifierFD(tcp->pid, fds[count], identifier, sizeof(identifier));
		getIdentifierFD(tcp->u_rval, fds[count], identifier2, sizeof(identifier2));

		if (ucpIP_existsContainer(identifier)) {
			ucPIP_addIdentifier(identifier, identifier2);
		}
	}
	free(fds);


	printf("clone(): %d : %ld\n", tcp->pid, tcp->u_rval);
}

void ucDataFlowSemantics_ftruncate(struct tcb *tcp) {
	// TODO. man 2 ftruncate; do sth if length == 0
}

void ucDataFlowSemantics_unlink(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	getString(tcp, tcp->u_arg[0], identifier, sizeof(identifier));

	ucPIP_removeIdentifier(identifier);

	printf("unlink(): %s\n", identifier);
}

void ucDataFlowSemantics_splice(struct tcb *tcp) {
	if (tcp->u_rval <= 0) {
		return;
	}



	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 splice
}

void ucDataFlowSemantics_munmap(struct tcb *tcp) {
	// TODO. man 2 munmap
	// is it possible to do something useful here?
	fprintf(stderr, "missing semantics for %s\n", tcp->s_ent->sys_name);
}

void ucDataFlowSemantics_pipe(struct tcb *tcp) {
	int fds[2];

	if (tcp->u_rval < 0) {
		return;
	}

	if (umoven(tcp, tcp->u_arg[0], sizeof(fds), (char *) fds) < 0) {
		return;
	}

	ucPIP_addIdentifier(getIdentifierFD(tcp->pid, fds[0], identifier, sizeof(identifier)), getIdentifierFD(tcp->pid, fds[1], identifier2, sizeof(identifier2)));

	printf("pipe(): %s %s\n", identifier, identifier2);
}

void ucDataFlowSemantics_dup(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier));
	getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2));

	ucPIP_addIdentifier(identifier, identifier2);

	printf("xdup, %s: %s --> %s\n", tcp->s_ent->sys_name, identifier, identifier2);
}

void ucDataFlowSemantics_dup2(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	// dup2() does nothing if oldfd == newfd; dup3() would have failed anyway
	if (tcp->u_arg[0] == tcp->u_rval) {
		return;
	}

	getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier));
	getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2));

	// close the new fd first, if necessary
	ucPIP_removeIdentifier(identifier2);

	ucPIP_addIdentifier(identifier, identifier2);

	printf("%s(): %s --> %s\n", tcp->s_ent->sys_name, identifier, identifier2);
}

//void ucPIPupdate(struct tcb *tcp) {
//	// pointer to the function to execute in order to update the PIP
//	void (*ucDataFlowSemanticsFunc)(struct tcb *tcp) = NULL;
//
//	// Note: Do not(!) compare function pointer. This will not work out,
//	// e.g. for dup() which is mapped to the internal sys_open()!
//	if (streq(tcp->s_ent->sys_name, "write")
//			|| streq(tcp->s_ent->sys_name, "writev")
//			|| streq(tcp->s_ent->sys_name, "pwrite")
//			|| streq(tcp->s_ent->sys_name, "pwritev")
//			|| streq(tcp->s_ent->sys_name, "pwrite64")
//			|| streq(tcp->s_ent->sys_name, "send")
//			|| streq(tcp->s_ent->sys_name, "sendto")
//			|| streq(tcp->s_ent->sys_name, "sendmsg")
//			|| streq(tcp->s_ent->sys_name, "sendmmsg")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_write;
//	}
//	else if (streq(tcp->s_ent->sys_name, "read")
//			|| streq(tcp->s_ent->sys_name, "readv")
//			|| streq(tcp->s_ent->sys_name, "pread")
//			|| streq(tcp->s_ent->sys_name, "pread64")
//			|| streq(tcp->s_ent->sys_name, "preadv")
//			|| streq(tcp->s_ent->sys_name, "recv")
//			|| streq(tcp->s_ent->sys_name, "recvfrom")
//			|| streq(tcp->s_ent->sys_name, "recvmsg")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_read;
//	}
//	else if (streq(tcp->s_ent->sys_name, "exit")
//			|| streq(tcp->s_ent->sys_name, "_exit")
//			|| streq(tcp->s_ent->sys_name, "exit_group")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_exit;
//	}
//	else if (streq(tcp->s_ent->sys_name, "execve")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_execve;
//	}
//	else if (streq(tcp->s_ent->sys_name, "close")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_close;
//	}
//	else if (streq(tcp->s_ent->sys_name, "open")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_open;
//	}
//	else if (streq(tcp->s_ent->sys_name, "openat")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_openat;
//	}
//	else if (streq(tcp->s_ent->sys_name, "socket")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_socket;
//	}
//	else if (streq(tcp->s_ent->sys_name, "socketpair")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_socketpair;
//	}
//	else if (streq(tcp->s_ent->sys_name, "accept")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_accept;
//	}
//	else if (streq(tcp->s_ent->sys_name, "connect")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_connect;
//	}
//	else if (streq(tcp->s_ent->sys_name, "fcntl64")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_fcntl;
//	}
//	else if (streq(tcp->s_ent->sys_name, "shutdown")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_shutdown;
//	}
//	else if (streq(tcp->s_ent->sys_name, "splice")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_splice;
//	}
//	else if (streq(tcp->s_ent->sys_name, "rename")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_rename;
//	}
//	else if (streq(tcp->s_ent->sys_name, "eventfd2")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_eventfd;
//	}
//	else if (streq(tcp->s_ent->sys_name, "unlink")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_unlink;
//	}
//	else if (streq(tcp->s_ent->sys_name, "kill")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_kill;
//	}
//	else if (streq(tcp->s_ent->sys_name, "munmap")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_munmap;
//	}
//	else if (streq(tcp->s_ent->sys_name, "clone")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_clone;
//	}
//	else if (streq(tcp->s_ent->sys_name, "ftruncate")
//			|| streq(tcp->s_ent->sys_name, "ftruncate64")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_ftruncate;
//	}
//	else if (streq(tcp->s_ent->sys_name, "mmap")
//			|| streq(tcp->s_ent->sys_name, "mmap2")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_mmap;
//	}
//	else if (streq(tcp->s_ent->sys_name, "pipe")
//			|| streq(tcp->s_ent->sys_name, "pipe2")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_pipe;
//	}
//	else if (streq(tcp->s_ent->sys_name, "dup")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_dup;
//	}
//	else if (streq(tcp->s_ent->sys_name, "dup2")
//			|| streq(tcp->s_ent->sys_name, "dup3")) {
//		ucDataFlowSemanticsFunc = ucDataFlowSemantics_dup2;
//	}
//	else {
//		ucDataFlowSemanticsFunc = NULL;
//	}
//
//	// TODO: sys_fstatfs may be useful to find out information about mounted file systems
//
//	// finally, update the PIP
//	if (ucDataFlowSemanticsFunc) {
//		(*ucDataFlowSemanticsFunc)(tcp);
//	}
//	else {
//		// this calls have been checked to not influence data flow.
//		// they can be ignored
//		if (!streq(tcp->s_ent->sys_name, "brk")
//
//			// stat family & file meta operations
//			&& !streq(tcp->s_ent->sys_name, "stat64")
//			&& !streq(tcp->s_ent->sys_name, "statfs64")
//			&& !streq(tcp->s_ent->sys_name, "statfs")
//			&& !streq(tcp->s_ent->sys_name, "fstatfs")
//			&& !streq(tcp->s_ent->sys_name, "fstat64")
//			&& !streq(tcp->s_ent->sys_name, "lstat64")
//			&& !streq(tcp->s_ent->sys_name, "chmod")
//			&& !streq(tcp->s_ent->sys_name, "chown32")
//			&& !streq(tcp->s_ent->sys_name, "umask")
//			&& !streq(tcp->s_ent->sys_name, "getxattr")
//			&& !streq(tcp->s_ent->sys_name, "lgetxattr")
//			&& !streq(tcp->s_ent->sys_name, "fgetxattr")
//			&& !streq(tcp->s_ent->sys_name, "readlink")
//			&& !streq(tcp->s_ent->sys_name, "lseek")
//			&& !streq(tcp->s_ent->sys_name, "fsync")
//			&& !streq(tcp->s_ent->sys_name, "utime")
//			&& !streq(tcp->s_ent->sys_name, "flock")
//			&& !streq(tcp->s_ent->sys_name, "symlink")
//			&& !streq(tcp->s_ent->sys_name, "faccessat")
//
//			// directory meta operations
//			&& !streq(tcp->s_ent->sys_name, "chdir")
//			&& !streq(tcp->s_ent->sys_name, "mkdir")
//			&& !streq(tcp->s_ent->sys_name, "rmdir")
//			&& !streq(tcp->s_ent->sys_name, "getcwd")
//			&& !streq(tcp->s_ent->sys_name, "getdents")
//			&& !streq(tcp->s_ent->sys_name, "getdents64")
//
//			// user IDs, group IDs, ...
//			&& !streq(tcp->s_ent->sys_name, "getuid32")
//			&& !streq(tcp->s_ent->sys_name, "getgid32")
//			&& !streq(tcp->s_ent->sys_name, "geteuid32")
//			&& !streq(tcp->s_ent->sys_name, "getegid32")
//			&& !streq(tcp->s_ent->sys_name, "getresuid32")
//			&& !streq(tcp->s_ent->sys_name, "getresgid32")
//			&& !streq(tcp->s_ent->sys_name, "setresuid32")
//			&& !streq(tcp->s_ent->sys_name, "setresgid32")
//			&& !streq(tcp->s_ent->sys_name, "setuid32")
//			&& !streq(tcp->s_ent->sys_name, "setgid32")
//
//			// time
//			&& !streq(tcp->s_ent->sys_name, "clock_getres")
//			&& !streq(tcp->s_ent->sys_name, "clock_gettime")
//			&& !streq(tcp->s_ent->sys_name, "clock_settime")
//			&& !streq(tcp->s_ent->sys_name, "gettimeofday")
//			&& !streq(tcp->s_ent->sys_name, "time")
//			&& !streq(tcp->s_ent->sys_name, "times")
//
//			// socket operations & information
//			&& !streq(tcp->s_ent->sys_name, "bind")
//			&& !streq(tcp->s_ent->sys_name, "listen")
//			&& !streq(tcp->s_ent->sys_name, "setsockopt")
//			&& !streq(tcp->s_ent->sys_name, "getsockopt")
//			&& !streq(tcp->s_ent->sys_name, "getpeername")
//			&& !streq(tcp->s_ent->sys_name, "getsockname")
//
//			// system limits
//			&& !streq(tcp->s_ent->sys_name, "getrlimit")
//			&& !streq(tcp->s_ent->sys_name, "setrlimit")
//			&& !streq(tcp->s_ent->sys_name, "getprlimit64")
//
//			// signal handling
//			&& !streq(tcp->s_ent->sys_name, "alarm")
//			&& !streq(tcp->s_ent->sys_name, "sigreturn")
//			&& !streq(tcp->s_ent->sys_name, "sigaltstack")
//			&& !streq(tcp->s_ent->sys_name, "rt_sigreturn")
//			&& !streq(tcp->s_ent->sys_name, "rt_sigaction")
//			&& !streq(tcp->s_ent->sys_name, "rt_sigprocmask")	// handling this call may slightly reduce overapproximations, because less signals get delivered
//
//			// kernel advises
//			&& !streq(tcp->s_ent->sys_name, "fadvise64")
//			&& !streq(tcp->s_ent->sys_name, "madvise")
//
//			// wait
//			&& !streq(tcp->s_ent->sys_name, "wait4")
//			&& // TODO: Actually, upon wait we get the return value of the child. Therefore,
//			!streq(tcp->s_ent->sys_name, "waitpid")
//			&& // we should handle these calls. But they will most likely kill data flow tracking due to overapproximations
//			!streq(tcp->s_ent->sys_name, "select") && !streq(tcp->s_ent->sys_name, "poll")
//
//
//			// thread management
//			&& !streq(tcp->s_ent->sys_name, "gettid")
//			&& !streq(tcp->s_ent->sys_name, "capset")
//			&& !streq(tcp->s_ent->sys_name, "capget")
//			&& !streq(tcp->s_ent->sys_name, "get_robust_list")
//			&& !streq(tcp->s_ent->sys_name, "set_robust_list")
//			&& !streq(tcp->s_ent->sys_name, "set_thread_area")
//			&& !streq(tcp->s_ent->sys_name, "set_tid_address")
//
//			// process & system operations
//			&& !streq(tcp->s_ent->sys_name, "getpid")
//			&& !streq(tcp->s_ent->sys_name, "getppid")
//			&& !streq(tcp->s_ent->sys_name, "getpgid")
//			&& !streq(tcp->s_ent->sys_name, "getpgrp")
//			&& !streq(tcp->s_ent->sys_name, "getrusage")
//			&& !streq(tcp->s_ent->sys_name, "prctl")
//			&& !streq(tcp->s_ent->sys_name, "getpriority")
//			&& !streq(tcp->s_ent->sys_name, "setpriority")
//			&& !streq(tcp->s_ent->sys_name, "sysinfo")
//			&& !streq(tcp->s_ent->sys_name, "setgroups")
//			&& !streq(tcp->s_ent->sys_name, "getgroups")
//			&& !streq(tcp->s_ent->sys_name, "getgroups32")
//			&& !streq(tcp->s_ent->sys_name, "setgroups32")
//			&& !streq(tcp->s_ent->sys_name, "getsid")
//			&& !streq(tcp->s_ent->sys_name, "setsid")
//			&& !streq(tcp->s_ent->sys_name, "sched_getaffinity")
//			&& !streq(tcp->s_ent->sys_name, "sched_setaffinity")
//
//			// shared memory
//			&& !streq(tcp->s_ent->sys_name, "shmdt")  // TODO: we should handle them!
//			&& !streq(tcp->s_ent->sys_name, "shmat")
//			&& !streq(tcp->s_ent->sys_name, "shmctl")
//			&& !streq(tcp->s_ent->sys_name, "shmget")
//
//			// inotify(7)
//			&& !streq(tcp->s_ent->sys_name, "inotify_init")
//			&& !streq(tcp->s_ent->sys_name, "inotify_init1")
//			&& !streq(tcp->s_ent->sys_name, "inotify_rm_watch")
//			&& !streq(tcp->s_ent->sys_name, "inotify_add_watch")
//
//			// epoll(7)
//			&& !streq(tcp->s_ent->sys_name, "epoll_create")
//			&& !streq(tcp->s_ent->sys_name, "epoll_create1")
//			&& !streq(tcp->s_ent->sys_name, "epoll_ctl")
//			&& !streq(tcp->s_ent->sys_name, "epoll_wait")
//
//			// misc.
//			&& !streq(tcp->s_ent->sys_name, "quotactl")
//			&& !streq(tcp->s_ent->sys_name, "readahead")
//			&& !streq(tcp->s_ent->sys_name, "access")
//			&& !streq(tcp->s_ent->sys_name, "fallocate")
//			&& !streq(tcp->s_ent->sys_name, "mprotect")
//			&& !streq(tcp->s_ent->sys_name, "futex")
//			&& !streq(tcp->s_ent->sys_name, "uname")
//			&& !streq(tcp->s_ent->sys_name, "_llseek")
//			&& !streq(tcp->s_ent->sys_name, "ioctl")
//			&& !streq(tcp->s_ent->sys_name, "nanosleep")
//			&& !streq(tcp->s_ent->sys_name, "restart_syscall")) {
//			printf("unhandled %s\n", tcp->s_ent->sys_name);
//		}
//
//	}
//
//}

void ucDataFlowSemantics__init() {
	int pid_max;
	int i;
	FILE *f;

	if (!(f = fopen("/proc/sys/kernel/pid_max", "r"))) {
		ucSemantics_errorExit("Unable to open /proc/sys/kernel/pid_max");
	}

	if (fscanf(f, "%d", &pid_max) != 1 || pid_max <= 1) {
		ucSemantics_errorExit("Unable to read proper value for pid_max");
	}

	// random max value here... (should not really matter)
	if (pid_max > 1048576) {
		ucSemantics_errorExit(
				"Value for pid_max too large ( > 1048576). Change that value in file '/proc/sys/kernel/pid_max' or change the code to allow for larger values.");
	}

	// important fact: this memory is initialized to zero!
	procMem = calloc(pid_max, sizeof(int));
}


