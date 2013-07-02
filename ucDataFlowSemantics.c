/*
 * ucSemantics.c
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#include "ucDataFlowSemantics.h"


char identifier[IDENTIFIER_MAX_LEN];
char identifier2[IDENTIFIER_MAX_LEN];

int initialProcess = 1;
int *procMem;
GHashTable **procFDs;
GHashTable *ignoreFDs;


int ignoreFile(char *absFilename) {
	if (strstr(absFilename, "/etc/") == absFilename
		|| strstr(absFilename, "/dev/") == absFilename
		|| strstr(absFilename, "/usr/") == absFilename
		|| strstr(absFilename, "/sys/") == absFilename
		|| strstr(absFilename, "/proc/") == absFilename
		|| strstr(absFilename, "/lib/") == absFilename
		|| strstr(absFilename, "/var/") == absFilename
		|| strstr(absFilename, ".viminfo") != NULL
		) {
		return (1);
	}

	return (0);
}

void addProcMem(int pid) {
	if (procMem[pid] == 0) {
		char procfsMem[512];
		snprintf(procfsMem, 512, "%s/%d/mem", PROCFS_MNT, pid);
		procMem[pid] = open(procfsMem, O_RDONLY);
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
		ucSemantics_errorExit("Unknown parent process");
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
	if ((read = readlink(tmp, cwd, len - 1)) == -1) {
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

	if (isAbsolute(relFilename)) {
		strncpy(absFilename, relFilename, absFilenameLen - 1);
		absFilename[absFilenameLen - 1] = '\0';
		return (absFilename);
	}

	if (!getCwd(pid, cwd, sizeof(cwd))) {
		return NULL;
	}

	getAbsoluteFilename(cwd, relFilename, absFilename, absFilenameLen, mustExist);

	return (absFilename);
}


void freeConditional(gpointer g) {
	if (g) free(g);
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

	if (g_hash_table_lookup_extended(ignoreFDs, identifier2, NULL, NULL)) {
		return;
	}

	ucPIP_copyData(identifier, identifier2, NULL);

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

	if (g_hash_table_lookup_extended(ignoreFDs, identifier, NULL, NULL)) {
		return;
	}

	ucSemantics_log("%s(): %d <-- %s\n", tcp->s_ent->sys_name, tcp->pid, identifier);

	// FD -> PID
#if defined(UC_DECLASS_ENABLED) && UC_DECLASS_ENABLED
	ucDataSet copied;

	dataSetNew(copied);
	ucPIP_copyData(identifier, identifier2, copied);
	ucDeclass_splus_add(tcp->pid, copied);
#else
	ucPIP_copyData(identifier, identifier2, NULL);
#endif
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
			ucSemantics_do_fd_close(pid, * (int*)fd);
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


void ucSemantics_do_fd_close(pid_t pid, int fd) {
	getIdentifierFD(pid, fd, identifier, sizeof(identifier));

	ucPIP_removeIdentifier(identifier);
	g_hash_table_remove(ignoreFDs, identifier);
}


// done
void ucSemantics_close(struct tcb *tcp) {
	if (tcp->u_arg[0] < 0) {
		return;
	}

	ucSemantics_do_fd_close(tcp->pid, tcp->u_arg[0]);

	ucSemantics_log("close(): %dx%d\n", tcp->pid, tcp->u_arg[0]);
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

	if (ignoreFile(absFilename)) {
		g_hash_table_insert(ignoreFDs, strdup(identifier), NULL);
		printf("%s(): ignoring %s (%s)\n", tcp->s_ent->sys_name, identifier, absFilename);
	}
	else {
		ucPIP_addIdentifier(absFilename, identifier);
		ucSemantics_log("%s(): %s %d: %s --> %s\n", tcp->s_ent->sys_name, trunkstr, tcp->pid, absFilename, identifier);
	}

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

		if (g_hash_table_lookup_extended(ignoreFDs, identifier, NULL, NULL)) {
			g_hash_table_insert(ignoreFDs, strdup(identifier2), NULL);
			printf("%s(): ignoring %s (%s)\n", tcp->s_ent->sys_name, identifier2, identifier);
		}
		else {
			ucPIP_addIdentifier(identifier, identifier2);
			ucSemantics_log("fcntl(): %s --> %s\n", identifier, identifier2);
		}
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
	ucPIP_copyData(identifier, identifier2, NULL);

	ucSemantics_log("kill(): %s --> %s\n", identifier, identifier2);

	// todo: kill may imply exit (do CTRL+F4 while the monitored gnome-terminal is booting up)
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

	if (!ignoreFile(newAbsFilename)) {
		ucPIP_addIdentifier(oldAbsFilename, newAbsFilename);
	}

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
	ucPIP_copyData(identifier, identifier2, NULL);

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

			if (g_hash_table_lookup_extended(ignoreFDs, identifier, NULL, NULL)) {
				g_hash_table_insert(ignoreFDs, strdup(identifier2), NULL);
				printf("clone(): ignoring %s (%s)\n", identifier2, identifier);
			}
			else if (VALID_CONTID(ucPIP_getContainer(identifier, 0))) {
				ucPIP_addIdentifier(identifier, identifier2);
			}
		}
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

	getIdentifierFD(tcp->pid, fds[0], identifier, sizeof(identifier));
	getIdentifierFD(tcp->pid, fds[1], identifier2, sizeof(identifier2));

	if (g_hash_table_lookup_extended(ignoreFDs, identifier, NULL, NULL)) {
		g_hash_table_insert(ignoreFDs, strdup(identifier2), NULL);
		printf("%s(): ignoring %s (%s)\n", tcp->s_ent->sys_name, identifier2, identifier);
	}
	else {
		ucPIP_addIdentifier(identifier, identifier2);
		ucSemantics_log("pipe(): %s %s\n", identifier, identifier2);
	}



}

void ucSemantics_dup(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier));
	getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2));

	if (g_hash_table_lookup_extended(ignoreFDs, identifier, NULL, NULL)) {
		g_hash_table_insert(ignoreFDs, strdup(identifier2), NULL);
		printf("%s(): ignoring %s (%s)\n", tcp->s_ent->sys_name, identifier2, identifier);
	}
	else {
		ucPIP_addIdentifier(identifier, identifier2);
		ucSemantics_log("%s(): %s --> %s\n", tcp->s_ent->sys_name, identifier, identifier2);
	}

}

void ucSemantics_dup2(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return;
	}

	// dup2() does nothing if oldfd == newfd; dup3() would have failed anyway
	if (tcp->u_arg[0] == tcp->u_rval) {
		return;
	}

	// close the new fd first, if necessary
	ucSemantics_do_fd_close(tcp->pid, tcp->u_rval);

	getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier, sizeof(identifier));
	getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2));

	if (g_hash_table_lookup_extended(ignoreFDs, identifier, NULL, NULL)) {
		g_hash_table_insert(ignoreFDs, strdup(identifier2), NULL);
		printf("%s(): ignoring %s (%s)\n", tcp->s_ent->sys_name, identifier2, identifier);
	}
	else {
		ucPIP_addIdentifier(identifier, identifier2);
		ucSemantics_log("%s(): %s --> %s\n", tcp->s_ent->sys_name, identifier, identifier2);
	}

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

	ignoreFDs = g_hash_table_new_full(g_str_hash, g_str_equal, free, NULL);
	printf("init\n");fflush(stdout);

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






void (*ucSemanticsFunct[])(struct tcb *tcp) = {
	[SYS_accept] = ucSemantics_accept,
	[SYS_access] = ucSemantics_IGNORE,
	[SYS_acct] = ucSemantics_IGNORE,
	[SYS_add_key] = ucSemantics_IGNORE,
	[SYS_adjtimex] = ucSemantics_IGNORE,
	[SYS_afs_syscall] = ucSemantics_IGNORE,
	[SYS_alarm] = ucSemantics_IGNORE,
	[SYS_bdflush] = ucSemantics_IGNORE,
	[SYS_bind] = ucSemantics_IGNORE,
	[SYS_break] = ucSemantics_IGNORE,
	[SYS_brk] = ucSemantics_IGNORE,
	[SYS_capget] = ucSemantics_IGNORE,
	[SYS_capset] = ucSemantics_IGNORE,
	[SYS_chdir] = ucSemantics_IGNORE,
	[SYS_chmod] = ucSemantics_IGNORE,
	[SYS_chown32] = ucSemantics_IGNORE,
	[SYS_chown] = ucSemantics_IGNORE,
	[SYS_chroot] = ucSemantics_IGNORE,
	[SYS_clock_adjtime] = ucSemantics_IGNORE,
	[SYS_clock_getres] = ucSemantics_IGNORE,
	[SYS_clock_gettime] = ucSemantics_IGNORE,
	[SYS_clock_nanosleep] = ucSemantics_IGNORE,
	[SYS_clock_settime] = ucSemantics_IGNORE,
	[SYS_clone] = ucSemantics_IGNORE,								// we handle clone differently...
	[SYS_close] = ucSemantics_close,
	[SYS_connect] = ucSemantics_connect,
	[SYS_create_module] = ucSemantics_IGNORE,
	[SYS_creat] = ucSemantics_IGNORE,
	[SYS_delete_module] = ucSemantics_IGNORE,
	[SYS_dup2] = ucSemantics_dup2,
	[SYS_dup3] = ucSemantics_dup2,
	[SYS_dup] = ucSemantics_dup,
	[SYS_epoll_create1] = ucSemantics_IGNORE,
	[SYS_epoll_create] = ucSemantics_IGNORE,
	[SYS_epoll_ctl] = ucSemantics_IGNORE,
	[SYS_epoll_pwait] = ucSemantics_IGNORE,
	[SYS_epoll_wait] = ucSemantics_IGNORE,
	[SYS_eventfd2] = ucSemantics_eventfd,
	[SYS_eventfd] = ucSemantics_eventfd,
	[SYS_execve] = ucSemantics_execve,
	[SYS_exit_group] = ucSemantics_exit_group,
	[SYS_exit] = ucSemantics_exit,
	[SYS_faccessat] = ucSemantics_IGNORE,
	[SYS_fadvise64_64] = ucSemantics_IGNORE,
	[SYS_fadvise64] = ucSemantics_IGNORE,
	[SYS_fallocate] = ucSemantics_IGNORE,
	[SYS_fanotify_init] = ucSemantics_IGNORE,
	[SYS_fanotify_mark] = ucSemantics_IGNORE,
	[SYS_fchdir] = ucSemantics_IGNORE,
	[SYS_fchmodat] = ucSemantics_IGNORE,
	[SYS_fchmod] = ucSemantics_IGNORE,
	[SYS_fchown32] = ucSemantics_IGNORE,
	[SYS_fchownat] = ucSemantics_IGNORE,
	[SYS_fchown] = ucSemantics_IGNORE,
	[SYS_fcntl64] = ucSemantics_fcntl,
	[SYS_fcntl] = ucSemantics_fcntl,
	[SYS_fdatasync] = ucSemantics_IGNORE,
	[SYS_fgetxattr] = ucSemantics_IGNORE,
	[SYS_flistxattr] = ucSemantics_IGNORE,
	[SYS_flock] = ucSemantics_IGNORE,
	[SYS_fork] = ucSemantics_IGNORE,
	[SYS_fremovexattr] = ucSemantics_IGNORE,
	[SYS_fsetxattr] = ucSemantics_IGNORE,
	[SYS_fstat64] = ucSemantics_IGNORE,
	[SYS_fstatat64] = ucSemantics_IGNORE,
	[SYS_fstatfs64] = ucSemantics_IGNORE,
	[SYS_fstatfs] = ucSemantics_IGNORE,
	[SYS_fstat] = ucSemantics_IGNORE,
	[SYS_fsync] = ucSemantics_IGNORE,
	[SYS_ftime] = ucSemantics_IGNORE,
	[SYS_ftruncate64] = ucSemantics_ftruncate,
	[SYS_ftruncate] = ucSemantics_ftruncate,
	[SYS_futex] = ucSemantics_IGNORE,
	[SYS_futimesat] = ucSemantics_IGNORE,
	[SYS_getcpu] = ucSemantics_IGNORE,
	[SYS_getcwd] = ucSemantics_IGNORE,
	[SYS_getdents64] = ucSemantics_IGNORE,
	[SYS_getdents] = ucSemantics_IGNORE,
	[SYS_getegid32] = ucSemantics_IGNORE,
	[SYS_getegid] = ucSemantics_IGNORE,
	[SYS_geteuid32] = ucSemantics_IGNORE,
	[SYS_geteuid] = ucSemantics_IGNORE,
	[SYS_getgid32] = ucSemantics_IGNORE,
	[SYS_getgid] = ucSemantics_IGNORE,
	[SYS_getgroups32] = ucSemantics_IGNORE,
	[SYS_getgroups] = ucSemantics_IGNORE,
	[SYS_getitimer] = ucSemantics_IGNORE,
	[SYS_get_kernel_syms] = ucSemantics_IGNORE,
	[SYS_get_mempolicy] = ucSemantics_IGNORE,
	[SYS_getpeername] = ucSemantics_IGNORE,
	[SYS_getpgid] = ucSemantics_IGNORE,
	[SYS_getpgrp] = ucSemantics_IGNORE,
	[SYS_getpid] = ucSemantics_IGNORE,
	[SYS_getpmsg] = ucSemantics_IGNORE,
	[SYS_getppid] = ucSemantics_IGNORE,
	[SYS_getpriority] = ucSemantics_IGNORE,
	[SYS_getresgid32] = ucSemantics_IGNORE,
	[SYS_getresgid] = ucSemantics_IGNORE,
	[SYS_getresuid32] = ucSemantics_IGNORE,
	[SYS_getresuid] = ucSemantics_IGNORE,
	[SYS_getrlimit] = ucSemantics_IGNORE,
	[SYS_get_robust_list] = ucSemantics_IGNORE,
	[SYS_getrusage] = ucSemantics_IGNORE,
	[SYS_getsid] = ucSemantics_IGNORE,
	[SYS_getsockname] = ucSemantics_IGNORE,
	[SYS_getsockopt] = ucSemantics_IGNORE,
	[SYS_get_thread_area] = ucSemantics_IGNORE,
	[SYS_gettid] = ucSemantics_IGNORE,
	[SYS_gettimeofday] = ucSemantics_IGNORE,
	[SYS_getuid32] = ucSemantics_IGNORE,
	[SYS_getuid] = ucSemantics_IGNORE,
	[SYS_getxattr] = ucSemantics_IGNORE,
	[SYS_gtty] = ucSemantics_IGNORE,
	[SYS_idle] = ucSemantics_IGNORE,
	[SYS_init_module] = ucSemantics_IGNORE,
	[SYS_inotify_add_watch] = ucSemantics_IGNORE,
	[SYS_inotify_init1] = ucSemantics_IGNORE,
	[SYS_inotify_init] = ucSemantics_IGNORE,
	[SYS_inotify_rm_watch] = ucSemantics_IGNORE,
	[SYS_io_cancel] = ucSemantics_IGNORE,
	[SYS_ioctl] = ucSemantics_IGNORE,
	[SYS_io_destroy] = ucSemantics_IGNORE,
	[SYS_io_getevents] = ucSemantics_IGNORE,
	[SYS_ioperm] = ucSemantics_IGNORE,
	[SYS_iopl] = ucSemantics_IGNORE,
	[SYS_ioprio_get] = ucSemantics_IGNORE,
	[SYS_ioprio_set] = ucSemantics_IGNORE,
	[SYS_io_setup] = ucSemantics_IGNORE,
	[SYS_io_submit] = ucSemantics_IGNORE,
	[SYS_ipc] = ucSemantics_IGNORE,
	[SYS_kexec_load] = ucSemantics_IGNORE,
	[SYS_keyctl] = ucSemantics_IGNORE,
	[SYS_kill] = ucSemantics_kill,
	[SYS_lchown32] = ucSemantics_IGNORE,
	[SYS_lchown] = ucSemantics_IGNORE,
	[SYS_lgetxattr] = ucSemantics_IGNORE,
	[SYS_linkat] = ucSemantics_IGNORE,
	[SYS_link] = ucSemantics_IGNORE,
	[SYS_listen] = ucSemantics_IGNORE,
	[SYS_listxattr] = ucSemantics_IGNORE,
	[SYS_llistxattr] = ucSemantics_IGNORE,
	[SYS__llseek] = ucSemantics_IGNORE,
	[SYS__llseek] = ucSemantics_IGNORE,
	[SYS_lock] = ucSemantics_IGNORE,
	[SYS_lookup_dcookie] = ucSemantics_IGNORE,
	[SYS_lremovexattr] = ucSemantics_IGNORE,
	[SYS_lseek] = ucSemantics_IGNORE,
	[SYS_lsetxattr] = ucSemantics_IGNORE,
	[SYS_lstat64] = ucSemantics_IGNORE,
	[SYS_lstat] = ucSemantics_IGNORE,
	[SYS_madvise] = ucSemantics_IGNORE,
	[SYS_mbind] = ucSemantics_IGNORE,
	[SYS_migrate_pages] = ucSemantics_IGNORE,
	[SYS_mincore] = ucSemantics_IGNORE,
	[SYS_mkdirat] = ucSemantics_IGNORE,
	[SYS_mkdir] = ucSemantics_IGNORE,
	[SYS_mknodat] = ucSemantics_IGNORE,
	[SYS_mknod] = ucSemantics_IGNORE,
	[SYS_mlockall] = ucSemantics_IGNORE,
	[SYS_mlock] = ucSemantics_IGNORE,
	[SYS_mmap2] = ucSemantics_mmap,
	[SYS_mmap] = ucSemantics_mmap,
	[SYS_modify_ldt] = ucSemantics_IGNORE,
	[SYS_mount] = ucSemantics_IGNORE,
	[SYS_move_pages] = ucSemantics_IGNORE,
	[SYS_mprotect] = ucSemantics_IGNORE,
	[SYS_mpx] = ucSemantics_IGNORE,
	[SYS_mq_getsetattr] = ucSemantics_IGNORE,
	[SYS_mq_notify] = ucSemantics_IGNORE,
	[SYS_mq_open ] = ucSemantics_IGNORE,
	[SYS_mq_timedreceive] = ucSemantics_IGNORE,
	[SYS_mq_timedsend] = ucSemantics_IGNORE,
	[SYS_mq_unlink] = ucSemantics_IGNORE,
	[SYS_mremap] = ucSemantics_IGNORE,
	[SYS_msync] = ucSemantics_IGNORE,
	[SYS_munlockall] = ucSemantics_IGNORE,
	[SYS_munlock] = ucSemantics_IGNORE,
	[SYS_munmap] = ucSemantics_munmap,
	[SYS_name_to_handle_at] = ucSemantics_IGNORE,
	[SYS_nanosleep] = ucSemantics_IGNORE,
	[SYS__newselect] = ucSemantics_IGNORE,
	[SYS_nfsservctl] = ucSemantics_IGNORE,
	[SYS_nice] = ucSemantics_IGNORE,
	[SYS_oldfstat] = ucSemantics_IGNORE,
	[SYS_oldlstat] = ucSemantics_IGNORE,
	[SYS_oldolduname] = ucSemantics_IGNORE,
	[SYS_oldstat] = ucSemantics_IGNORE,
	[SYS_olduname] = ucSemantics_IGNORE,
	[SYS_openat] = ucSemantics_openat,
	[SYS_open_by_handle_at] = ucSemantics_IGNORE,
	[SYS_open] = ucSemantics_open,
	[SYS_pause] = ucSemantics_IGNORE,
	[SYS_perf_event_open] = ucSemantics_IGNORE,
	[SYS_personality] = ucSemantics_IGNORE,
	[SYS_pipe2] = ucSemantics_pipe,
	[SYS_pipe] = ucSemantics_pipe,
	[SYS_pivot_root] = ucSemantics_IGNORE,
	[SYS_poll] = ucSemantics_IGNORE,
	[SYS_ppoll] = ucSemantics_IGNORE,
	[SYS_prctl] = ucSemantics_IGNORE,
	[SYS_pread64] = ucSemantics_read,
	[SYS_preadv] = ucSemantics_read,
	[SYS_prlimit64] = ucSemantics_IGNORE,
	[SYS_process_vm_readv] = ucSemantics_IGNORE,
	[SYS_process_vm_writev] = ucSemantics_IGNORE,
	[SYS_profil] = ucSemantics_IGNORE,
	[SYS_prof] = ucSemantics_IGNORE,
	[SYS_pselect6] = ucSemantics_IGNORE,
	[SYS_ptrace] = ucSemantics_IGNORE,
	[SYS_putpmsg] = ucSemantics_IGNORE,
	[SYS_pwrite64] = ucSemantics_write,
	[SYS_pwritev] = ucSemantics_write,
	[SYS_query_module] = ucSemantics_IGNORE,
	[SYS_quotactl] = ucSemantics_IGNORE,
	[SYS_readahead] = ucSemantics_IGNORE,
	[SYS_readdir] = ucSemantics_IGNORE,
	[SYS_readlinkat] = ucSemantics_IGNORE,
	[SYS_readlink] = ucSemantics_IGNORE,
	[SYS_read] = ucSemantics_read,
	[SYS_readv] = ucSemantics_read,
	[SYS_reboot] = ucSemantics_IGNORE,
	[SYS_recvfrom] = ucSemantics_read,
	[SYS_recvmmsg] = ucSemantics_read,
	[SYS_recvmsg] = ucSemantics_read,
	[SYS_recv] = ucSemantics_read,
	[SYS_remap_file_pages] = ucSemantics_IGNORE,
	[SYS_removexattr] = ucSemantics_IGNORE,
	[SYS_renameat] = ucSemantics_IGNORE,
	[SYS_rename] = ucSemantics_rename,
	[SYS_request_key] = ucSemantics_IGNORE,
	[SYS_restart_syscall] = ucSemantics_IGNORE,
	[SYS_rmdir] = ucSemantics_IGNORE,
	[SYS_rt_sigaction] = ucSemantics_IGNORE,
	[SYS_rt_sigaction] = ucSemantics_IGNORE,
	[SYS_rt_sigpending] = ucSemantics_IGNORE,
	[SYS_rt_sigprocmask] = ucSemantics_IGNORE,
	[SYS_rt_sigqueueinfo] = ucSemantics_IGNORE,
	[SYS_rt_sigreturn] = ucSemantics_IGNORE,
	[SYS_rt_sigsuspend] = ucSemantics_IGNORE,
	[SYS_rt_sigtimedwait] = ucSemantics_IGNORE,
	[SYS_rt_tgsigqueueinfo] = ucSemantics_IGNORE,
	[SYS_sched_getaffinity] = ucSemantics_IGNORE,
	[SYS_sched_getparam] = ucSemantics_IGNORE,
	[SYS_sched_get_priority_max] = ucSemantics_IGNORE,
	[SYS_sched_get_priority_min] = ucSemantics_IGNORE,
	[SYS_sched_getscheduler] = ucSemantics_IGNORE,
	[SYS_sched_rr_get_interval] = ucSemantics_IGNORE,
	[SYS_sched_setaffinity] = ucSemantics_IGNORE,
	[SYS_sched_setparam] = ucSemantics_IGNORE,
	[SYS_sched_setscheduler] = ucSemantics_IGNORE,
	[SYS_sched_yield] = ucSemantics_IGNORE,
	[SYS_select] = ucSemantics_IGNORE,
	[SYS_sendfile64] = ucSemantics_IGNORE,
	[SYS_sendfile] = ucSemantics_IGNORE,
	[SYS_sendmmsg] = ucSemantics_write,
	[SYS_sendmsg] = ucSemantics_write,
	[SYS_send] = ucSemantics_write,
	[SYS_sendto] = ucSemantics_write,
	[SYS_setdomainname] = ucSemantics_IGNORE,
	[SYS_setfsgid32] = ucSemantics_IGNORE,
	[SYS_setfsgid] = ucSemantics_IGNORE,
	[SYS_setfsuid32] = ucSemantics_IGNORE,
	[SYS_setfsuid] = ucSemantics_IGNORE,
	[SYS_setgid32] = ucSemantics_IGNORE,
	[SYS_setgid] = ucSemantics_IGNORE,
	[SYS_setgroups32] = ucSemantics_IGNORE,
	[SYS_setgroups] = ucSemantics_IGNORE,
	[SYS_sethostname] = ucSemantics_IGNORE,
	[SYS_setitimer] = ucSemantics_IGNORE,
	[SYS_set_mempolicy] = ucSemantics_IGNORE,
	[SYS_setns] = ucSemantics_IGNORE,
	[SYS_setpgid] = ucSemantics_IGNORE,
	[SYS_setpriority] = ucSemantics_IGNORE,
	[SYS_setregid32] = ucSemantics_IGNORE,
	[SYS_setregid] = ucSemantics_IGNORE,
	[SYS_setresgid32] = ucSemantics_IGNORE,
	[SYS_setresgid] = ucSemantics_IGNORE,
	[SYS_setresuid32] = ucSemantics_IGNORE,
	[SYS_setresuid] = ucSemantics_IGNORE,
	[SYS_setreuid32] = ucSemantics_IGNORE,
	[SYS_setreuid] = ucSemantics_IGNORE,
	[SYS_setrlimit] = ucSemantics_IGNORE,
	[SYS_set_robust_list] = ucSemantics_IGNORE,
	[SYS_setsid] = ucSemantics_IGNORE,
	[SYS_setsockopt] = ucSemantics_IGNORE,
	[SYS_set_thread_area] = ucSemantics_IGNORE,
	[SYS_set_thread_area] = ucSemantics_IGNORE,
	[SYS_set_tid_address] = ucSemantics_IGNORE,
	[SYS_settimeofday] = ucSemantics_IGNORE,
	[SYS_setuid32] = ucSemantics_IGNORE,
	[SYS_setuid] = ucSemantics_IGNORE,
	[SYS_setxattr] = ucSemantics_IGNORE,
	[SYS_sgetmask] = ucSemantics_IGNORE,
	[SYS_shutdown] = ucSemantics_shutdown,
	[SYS_sigaction] = ucSemantics_IGNORE,
	[SYS_sigaltstack] = ucSemantics_IGNORE,
	[SYS_signalfd4] = ucSemantics_IGNORE,
	[SYS_signalfd] = ucSemantics_IGNORE,
	[SYS_signal] = ucSemantics_IGNORE,
	[SYS_sigpending] = ucSemantics_IGNORE,
	[SYS_sigprocmask] = ucSemantics_IGNORE,
	[SYS_sigreturn] = ucSemantics_IGNORE,
	[SYS_sigsuspend] = ucSemantics_IGNORE,
	[SYS_socketcall] = ucSemantics_IGNORE,
	[SYS_socketpair] = ucSemantics_socketpair,
	[SYS_socket_subcall + 1] = ucSemantics_socket,
	[SYS_splice] = ucSemantics_splice,
	[SYS_ssetmask] = ucSemantics_IGNORE,
	[SYS_stat64] = ucSemantics_IGNORE,
	[SYS_statfs64] = ucSemantics_IGNORE,
	[SYS_statfs] = ucSemantics_IGNORE,
	[SYS_stat] = ucSemantics_IGNORE,
	[SYS_stime] = ucSemantics_IGNORE,
	[SYS_stty] = ucSemantics_IGNORE,
	[SYS_swapoff] = ucSemantics_IGNORE,
	[SYS_swapon] = ucSemantics_IGNORE,
	[SYS_symlinkat] = ucSemantics_IGNORE,
	[SYS_symlink] = ucSemantics_IGNORE,
	[SYS_sync_file_range] = ucSemantics_IGNORE,
	[SYS_syncfs] = ucSemantics_IGNORE,
	[SYS_sync] = ucSemantics_IGNORE,
	[SYS__sysctl] = ucSemantics_IGNORE,
	[SYS_sysfs] = ucSemantics_IGNORE,
	[SYS_sysinfo] = ucSemantics_IGNORE,
	[SYS_syslog] = ucSemantics_IGNORE,
	[SYS_tee] = ucSemantics_IGNORE,
	[SYS_tgkill] = ucSemantics_IGNORE,
	[SYS_timer_create] = ucSemantics_IGNORE,
	[SYS_timer_delete] = ucSemantics_IGNORE,
	[SYS_timerfd_create] = ucSemantics_IGNORE,
	[SYS_timerfd_gettime] = ucSemantics_IGNORE,
	[SYS_timerfd_settime] = ucSemantics_IGNORE,
	[SYS_timer_getoverrun] = ucSemantics_IGNORE,
	[SYS_timer_gettime] = ucSemantics_IGNORE,
	[SYS_timer_settime] = ucSemantics_IGNORE,
	[SYS_times] = ucSemantics_IGNORE,
	[SYS_time] = ucSemantics_IGNORE,
	[SYS_tkill] = ucSemantics_IGNORE,
	[SYS_truncate64] = ucSemantics_IGNORE,
	[SYS_truncate] = ucSemantics_IGNORE,
	[SYS_ugetrlimit] = ucSemantics_IGNORE,
	[SYS_ulimit] = ucSemantics_IGNORE,
	[SYS_umask] = ucSemantics_IGNORE,
	[SYS_umount2] = ucSemantics_IGNORE,
	[SYS_umount] = ucSemantics_IGNORE,
	[SYS_uname] = ucSemantics_IGNORE,
	[SYS_unlinkat] = ucSemantics_IGNORE,
	[SYS_unlink] = ucSemantics_unlink,
	[SYS_unshare] = ucSemantics_IGNORE,
	[SYS_uselib] = ucSemantics_IGNORE,
	[SYS_ustat] = ucSemantics_IGNORE,
	[SYS_utimensat] = ucSemantics_IGNORE,
	[SYS_utimes] = ucSemantics_IGNORE,
	[SYS_utime] = ucSemantics_IGNORE,
	[SYS_vfork] = ucSemantics_IGNORE,
	[SYS_vhangup] = ucSemantics_IGNORE,
	[SYS_vm86old] = ucSemantics_IGNORE,
	[SYS_vm86] = ucSemantics_IGNORE,
	[SYS_vmsplice] = ucSemantics_IGNORE,
	[SYS_vserver] = ucSemantics_IGNORE,
	[SYS_wait4] = ucSemantics_IGNORE,
	[SYS_waitid] = ucSemantics_IGNORE,
	[SYS_waitpid] = ucSemantics_IGNORE,
	[SYS_write] = ucSemantics_write,
	[SYS_writev] = ucSemantics_write,


	[SYS_semop] = ucSemantics_IGNORE,
	[SYS_semget] = ucSemantics_IGNORE,
	[SYS_semctl] = ucSemantics_IGNORE,
	[SYS_semtimedop] = ucSemantics_IGNORE,
	[SYS_ipc_subcall] = ucSemantics_IGNORE,
	[SYS_msgsnd] = ucSemantics_IGNORE,
	[SYS_msgrcv] = ucSemantics_IGNORE,
	[SYS_msgget] = ucSemantics_IGNORE,
	[SYS_msgctl] = ucSemantics_IGNORE,
	[SYS_shmat] = ucSemantics_IGNORE,
	[SYS_shmdt] = ucSemantics_IGNORE,
	[SYS_shmget] = ucSemantics_IGNORE,
	[SYS_shmctl] = ucSemantics_IGNORE,

	[SYS_cloneFirstAction] = ucSemantics_cloneFirstAction

};

