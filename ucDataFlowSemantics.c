/*
 * ucSemantics.c
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#include "ucDataFlowSemantics.h"


char identifier[IDENTIFIER_MAX_LEN];
char identifier2[IDENTIFIER_MAX_LEN];


int *procMem;
GHashTable **procFDs;
int initialProcess = 1;


void addProcMem(int pid) {
	if (procMem[pid] == 0) {
		char foo[512];
		snprintf(foo, 512, "%s/%d/mem", PROCFS_MNT, pid);
		procMem[pid] = open(foo, O_RDONLY);
	}
}

void removeProcMem(int pid) {
	close(procMem[pid]);
	procMem[pid] = 0;
}

int ucSemantics_initProcess(int pid) {
	char *line = NULL;
	char *lineCp;
	size_t len = 0;
	int ppid = -1;

	char procpath[FILENAME_MAX];
	snprintf(procpath, sizeof(procpath), "%s/%d/status", PROCFS_MNT, pid);
	FILE *procfs = fopen(procpath, "r");

	if (!procfs) {
		ucSemantics_errorExit("Unable to read procfs");
	}

	while ((getline(&line, &len, procfs)) != -1 && ppid == -1) {
		if (strneq("PPid:",line,5)) {
			lineCp = line + strlen("PPid:");
			while(*lineCp++ == ' ');
			sscanf(lineCp, "%d\n", &ppid);
		}
	}

	if (line) {
		free(line);
	}

	if (ppid == -1) {
		ucSemantics_errorExit("No mummy found! Unknown parent process");
	}

	addProcMem(pid);

	if (procFDs[pid]) {
		ucSemantics_errorExit("procFDs not unset. This should not happen. This variable should have been set to NULL upon process exit.");
	}

	procFDs[pid] = g_hash_table_new_full(g_int_hash, g_int_equal, free, NULL);

	return (ppid);
}

void ucSemantics_destroyProcess(int pid) {
	g_hash_table_destroy(procFDs[pid]);
	procFDs[pid] = NULL;

	removeProcMem(pid);
}

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
		ucSemantics_log("getString fallback (reading using PTRACE_PEEKDATA)\n");
		if (!umovestr(tcp, addr, len_buf, dataStr)) {
			dataStr[len_buf - 1] = '\0';
		}
	}
	else {
		ucSemantics_log("getString usual (reading from /proc)\n");
	}

	// safety
	dataStr[len_buf - 1] = '\0';

	return (dataStr);
}

int *getIntDirEntries(long pid, int *count, char *procSubPath) {
	char procfsPath[PATH_MAX];
	int ret;
	DIR *dir;
	struct dirent *ent;
	int *fds;
	int size = 8; // do not init to 0!
	*count = 0;

	ret = snprintf(procfsPath, sizeof(procfsPath), "%s/%ld/%s", PROCFS_MNT, pid, procSubPath);

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


int *getProcessTasks(long pid, int *count) {
	return (getIntDirEntries(pid, count, "task"));
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

	snprintf(tmp, sizeof(tmp), "%s/%ld/cwd", PROCFS_MNT, pid);
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

	if (!procFDs[pid]) {
		procFDs[pid] = g_hash_table_new_full(g_int_hash, g_int_equal, free, NULL);
	}

	int *fdCp;

	if (!(fdCp = calloc(1, sizeof(int)))) {
		ucSemantics_errorExit("Unable to allocate enough memory");
	}
	*fdCp = fd;

	g_hash_table_insert(procFDs[pid], fdCp, NULL);

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
void ucSemantics_write(struct tcb *tcp) {
	if (tcp->u_arg[0] <= 0) {
		// if return value is 0, nothing was written.
		return;
	}

	getIdentifierPID(tcp->pid, identifier, sizeof(identifier));
	getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier2, sizeof(identifier2));

	// PID -> FD
	ucPIP_copyData(identifier, identifier2);

	ucSemantics_log("write(): %s --> %s\n", identifier, identifier2);
}

// todo: write into aliases
void ucSemantics_read(struct tcb *tcp) {
	if (tcp->u_arg[0] <= 0) {
		// if return value is 0, nothing was written.
		return;
	}

	getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier));
	getIdentifierPID(tcp->pid, identifier2, sizeof(identifier2));

	// FD -> PID
	ucPIP_copyData(identifier, identifier2);

	ucSemantics_log("read(): %d <-- %s\n", tcp->pid, identifier);
}




void ucSemantics_do_process_exit(int pid) {
	int count;
	int *openfds;

	getIdentifierPID(pid, identifier, sizeof(identifier));

	ucPIP_removeAllAliasesFrom(identifier);
	ucPIP_removeAllAliasesTo(identifier);

	ucPIP_removeIdentifier(identifier);

	// delete all of the processes' open file descriptors
	if (procFDs[pid]) {
		GHashTableIter iter;
		gpointer fd;
		g_hash_table_iter_init(&iter, procFDs[pid]);
		while (g_hash_table_iter_next (&iter, &fd, NULL)) {
			getIdentifierFD(pid, * (int*)fd, identifier, sizeof(identifier));
			ucPIP_removeIdentifier(identifier);
		}
	}
	else {
		ucSemantics_errorExit("procFDs not set. This should not happen.");
	}

	ucSemantics_destroyProcess(pid);

	ucSemantics_log("exit(): %d\n", pid);
}

void ucSemantics_exit(struct tcb *tcp) {
	ucSemantics_do_process_exit(tcp->pid);
}



void ucSemantics_exit_group(struct tcb *tcp) {
	int count;
	int *tasks;

	// terminate all of the processes' tasks
	if ((tasks = getProcessTasks(tcp->pid, &count))) {
		while (count-- > 0) {
			ucSemantics_do_process_exit(tasks[count]);
		}
		free(tasks);
	}

	ucSemantics_log("exit_group(): %d\n", tcp->pid);
}

void ucSemantics_execve(struct tcb *tcp) {
	if (initialProcess) {
		addProcMem(tcp->pid);

		getIdentifierPID(tcp->pid, identifier, sizeof(identifier));

		ucPIP_addIdentifier(identifier, NULL);

		initialProcess = 0;
	}
	// TODO: man 2 execve
	// Remember that execve returns 3 times!
	// Also consider man 2 open and fcntl: some file descriptors close automatically on exeve()
	ucSemantics_log("execve missing semantics for %s (pid: %d)\n", tcp->s_ent->sys_name, tcp->pid);
}



// done
void ucSemantics_close(struct tcb *tcp) {
	if (tcp->u_arg[0] < 0) {
		return;
	}

	ucPIP_removeIdentifier(getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier)));

	ucSemantics_log("close(): %s\n", identifier);
}


void ucSemantics_open(struct tcb *tcp) {
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

	ucSemantics_log("open(): %s %d: %s --> %s\n", trunkstr, tcp->pid, absFilename, identifier);
}

void ucSemantics_openat(struct tcb *tcp) {
	// TODO. man 2 openat
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
}

void ucSemantics_socket(struct tcb *tcp) {
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 socket
}

void ucSemantics_socketpair(struct tcb *tcp) {
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 socketpair
}

void ucSemantics_fcntl(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	if (tcp->u_arg[1] == F_DUPFD
			|| tcp->u_arg[1] == F_DUPFD_CLOEXEC) {
		getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier));
		getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2));

		ucPIP_addIdentifier(identifier, identifier2);

		ucSemantics_log("fcntl(): %s --> %s\n", identifier, identifier2);
	}
}

void ucSemantics_shutdown(struct tcb *tcp) {
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 shutdown
}

void ucSemantics_eventfd(struct tcb *tcp) {
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 eventfd
}

void ucSemantics_mmap(struct tcb *tcp) {
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 mmap
}

void ucSemantics_kill(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	getIdentifierPID(tcp->pid, identifier, sizeof(identifier));
	getIdentifierPID(tcp->u_arg[0], identifier2, sizeof(identifier2));

	// PID -> PID
	ucPIP_copyData(identifier, identifier2);

	ucSemantics_log("kill(): %s --> %s\n", identifier, identifier2);
}

void ucSemantics_accept(struct tcb *tcp) {
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 accept
}

void ucSemantics_connect(struct tcb *tcp) {
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 connect
}

void ucSemantics_rename(struct tcb *tcp) {
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

	ucSemantics_log("rename(): %s --> %s\n", oldAbsFilename, newAbsFilename);
}



void ucSemantics_cloneFirstAction(struct tcb *tcp) {
	int pid = tcp->pid;

	int ppid = ucSemantics_initProcess(pid);

	getIdentifierPID(ppid, identifier, sizeof(identifier));
	getIdentifierPID(pid, identifier2, sizeof(identifier2));

	ucPIP_addIdentifier(identifier2, NULL);

	ucSemantics_log("clone(): %s %s\n", identifier, identifier2);

	// PID -> PID
	ucPIP_copyData(identifier, identifier2);

	// clone all the aliases
	ucPIP_copyAliases(identifier, identifier2);
	ucPIP_alsoAlias(identifier, identifier2);

	// copy all file descriptors
	if (procFDs[ppid]) {
		GHashTableIter iter;
		gpointer fd;
		g_hash_table_iter_init(&iter, procFDs[ppid]);
		while (g_hash_table_iter_next (&iter, &fd, NULL)) {
			getIdentifierFD(ppid, * (int*)fd, identifier, sizeof(identifier));
			getIdentifierFD(pid, * (int*)fd, identifier2, sizeof(identifier2));

			if (ucpIP_existsContainer(identifier)) {
				ucPIP_addIdentifier(identifier, identifier2);
			}
		}
	}
	else {
		ucSemantics_errorExit("procFDs not set. This should not happen.");
	}
}


void ucSemantics_ftruncate(struct tcb *tcp) {
	// TODO. man 2 ftruncate; do sth if length == 0
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
}

void ucSemantics_unlink(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	getString(tcp, tcp->u_arg[0], identifier, sizeof(identifier));

	ucPIP_removeIdentifier(identifier);

	ucSemantics_log("unlink(): %s\n", identifier);
}

void ucSemantics_splice(struct tcb *tcp) {
	if (tcp->u_rval <= 0) {
		return;
	}



	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
	// TODO. man 2 splice
}

void ucSemantics_munmap(struct tcb *tcp) {
	// TODO. man 2 munmap
	// is it possible to do something useful here?
	ucSemantics_log("missing semantics for %s\n", tcp->s_ent->sys_name);
}

void ucSemantics_pipe(struct tcb *tcp) {
	int fds[2];

	if (tcp->u_rval < 0) {
		return;
	}

	if (umoven(tcp, tcp->u_arg[0], sizeof(fds), (char *) fds) < 0) {
		return;
	}

	ucPIP_addIdentifier(getIdentifierFD(tcp->pid, fds[0], identifier, sizeof(identifier)), getIdentifierFD(tcp->pid, fds[1], identifier2, sizeof(identifier2)));

	ucSemantics_log("pipe(): %s %s\n", identifier, identifier2);
}

void ucSemantics_dup(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier));
	getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2));

	ucPIP_addIdentifier(identifier, identifier2);

	ucSemantics_log("%s(): %s --> %s\n", tcp->s_ent->sys_name, identifier, identifier2);
}

void ucSemantics_dup2(struct tcb *tcp) {
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

	ucSemantics_log("%s(): %s --> %s\n", tcp->s_ent->sys_name, identifier, identifier2);
}

void ucSemantics__init() {
	int pid_max;
	int i;
	FILE *f;
	char procfs_pidmax[FILENAME_MAX];

	if (snprintf(procfs_pidmax, sizeof(procfs_pidmax), "%s/sys/kernel/pid_max", PROCFS_MNT) < 0) {
		ucSemantics_errorExit("Unable to open /proc/sys/kernel/pid_max");
	}

	if (!(f = fopen(procfs_pidmax, "r"))) {
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
	procFDs = calloc(pid_max, sizeof(GHashTable *));

	if (!procMem || !procFDs) {
		ucSemantics_errorExit("Unable to allocate enoug memory");
	}
}


void ucSemantics_IGNORE_impl(struct tcb *tcp) {
	ucSemantics_log("Intentionally ignoring %s (%d)\n",tcp->s_ent->sys_name, tcp->pid);
}

void ucSemantics_log_impl(const char* format, ...) {
	va_list argptr;
	va_start(argptr, format);
	vfprintf(stdout, format, argptr);
	va_end(argptr);
}
