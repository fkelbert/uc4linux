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

	// FD -> PID
	ucPIP_copyData(identifier, identifier2, NULL);

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
	ucPIP_copyData(identifier, identifier2, NULL);

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






void (*ucSemanticsFunct[])(struct tcb *tcp) = {
	[SYS_accept] = ucSemantics_accept,
	[SYS_access] = NULL,
	[SYS_acct] = NULL,
	[SYS_add_key] = NULL,
	[SYS_adjtimex] = NULL,
	[SYS_afs_syscall] = NULL,
	[SYS_alarm] = NULL,
	[SYS_bdflush] = NULL,
	[SYS_bind] = NULL,
	[SYS_break] = NULL,
	[SYS_brk] = NULL,
	[SYS_capget] = NULL,
	[SYS_capset] = NULL,
	[SYS_chdir] = NULL,
	[SYS_chmod] = NULL,
	[SYS_chown32] = NULL,
	[SYS_chown] = NULL,
	[SYS_chroot] = NULL,
	[SYS_clock_adjtime] = NULL,
	[SYS_clock_getres] = NULL,
	[SYS_clock_gettime] = NULL,
	[SYS_clock_nanosleep] = NULL,
	[SYS_clock_settime] = NULL,
	[SYS_clone] = NULL,								// we handle clone differently...
	[SYS_close] = ucSemantics_close,
	[SYS_connect] = ucSemantics_connect,
	[SYS_create_module] = NULL,
	[SYS_creat] = NULL,
	[SYS_delete_module] = NULL,
	[SYS_dup2] = ucSemantics_dup2,
	[SYS_dup3] = ucSemantics_dup2,
	[SYS_dup] = ucSemantics_dup,
	[SYS_epoll_create1] = NULL,
	[SYS_epoll_create] = NULL,
	[SYS_epoll_ctl] = NULL,
	[SYS_epoll_pwait] = NULL,
	[SYS_epoll_wait] = NULL,
	[SYS_eventfd2] = ucSemantics_eventfd,
	[SYS_eventfd] = ucSemantics_eventfd,
	[SYS_execve] = ucSemantics_execve,
	[SYS_exit_group] = ucSemantics_exit_group,
	[SYS_exit] = ucSemantics_exit,
	[SYS_faccessat] = NULL,
	[SYS_fadvise64_64] = NULL,
	[SYS_fadvise64] = NULL,
	[SYS_fallocate] = NULL,
	[SYS_fanotify_init] = NULL,
	[SYS_fanotify_mark] = NULL,
	[SYS_fchdir] = NULL,
	[SYS_fchmodat] = NULL,
	[SYS_fchmod] = NULL,
	[SYS_fchown32] = NULL,
	[SYS_fchownat] = NULL,
	[SYS_fchown] = NULL,
	[SYS_fcntl64] = ucSemantics_fcntl,
	[SYS_fcntl] = ucSemantics_fcntl,
	[SYS_fdatasync] = NULL,
	[SYS_fgetxattr] = NULL,
	[SYS_flistxattr] = NULL,
	[SYS_flock] = NULL,
	[SYS_fork] = ucSemantics_IGNORE,
	[SYS_fremovexattr] = NULL,
	[SYS_fsetxattr] = NULL,
	[SYS_fstat64] = NULL,
	[SYS_fstatat64] = NULL,
	[SYS_fstatfs64] = NULL,
	[SYS_fstatfs] = NULL,
	[SYS_fstat] = NULL,
	[SYS_fsync] = NULL,
	[SYS_ftime] = NULL,
	[SYS_ftruncate64] = ucSemantics_ftruncate,
	[SYS_ftruncate] = ucSemantics_ftruncate,
	[SYS_futex] = NULL,
	[SYS_futimesat] = NULL,
	[SYS_getcpu] = NULL,
	[SYS_getcwd] = NULL,
	[SYS_getdents64] = NULL,
	[SYS_getdents] = NULL,
	[SYS_getegid32] = NULL,
	[SYS_getegid] = NULL,
	[SYS_geteuid32] = NULL,
	[SYS_geteuid] = NULL,
	[SYS_getgid32] = NULL,
	[SYS_getgid] = NULL,
	[SYS_getgroups32] = NULL,
	[SYS_getgroups] = NULL,
	[SYS_getitimer] = NULL,
	[SYS_get_kernel_syms] = NULL,
	[SYS_get_mempolicy] = NULL,
	[SYS_getpeername] = NULL,
	[SYS_getpgid] = NULL,
	[SYS_getpgrp] = NULL,
	[SYS_getpid] = NULL,
	[SYS_getpmsg] = NULL,
	[SYS_getppid] = NULL,
	[SYS_getpriority] = NULL,
	[SYS_getresgid32] = NULL,
	[SYS_getresgid] = NULL,
	[SYS_getresuid32] = NULL,
	[SYS_getresuid] = NULL,
	[SYS_getrlimit] = NULL,
	[SYS_get_robust_list] = NULL,
	[SYS_getrusage] = NULL,
	[SYS_getsid] = NULL,
	[SYS_getsockname] = NULL,
	[SYS_getsockopt] = NULL,
	[SYS_get_thread_area] = NULL,
	[SYS_gettid] = NULL,
	[SYS_gettimeofday] = ucSemantics_IGNORE,
	[SYS_getuid32] = NULL,
	[SYS_getuid] = NULL,
	[SYS_getxattr] = NULL,
	[SYS_gtty] = NULL,
	[SYS_idle] = NULL,
	[SYS_init_module] = NULL,
	[SYS_inotify_add_watch] = NULL,
	[SYS_inotify_init1] = NULL,
	[SYS_inotify_init] = NULL,
	[SYS_inotify_rm_watch] = NULL,
	[SYS_io_cancel] = NULL,
	[SYS_ioctl] = NULL,
	[SYS_io_destroy] = NULL,
	[SYS_io_getevents] = NULL,
	[SYS_ioperm] = NULL,
	[SYS_iopl] = NULL,
	[SYS_ioprio_get] = NULL,
	[SYS_ioprio_set] = NULL,
	[SYS_io_setup] = NULL,
	[SYS_io_submit] = NULL,
	[SYS_ipc] = NULL,
	[SYS_kexec_load] = NULL,
	[SYS_keyctl] = NULL,
	[SYS_kill] = ucSemantics_kill,
	[SYS_lchown32] = NULL,
	[SYS_lchown] = NULL,
	[SYS_lgetxattr] = NULL,
	[SYS_linkat] = NULL,
	[SYS_link] = NULL,
	[SYS_listen] = NULL,
	[SYS_listxattr] = NULL,
	[SYS_llistxattr] = NULL,
	[SYS__llseek] = NULL,
	[SYS__llseek] = NULL,
	[SYS_lock] = NULL,
	[SYS_lookup_dcookie] = NULL,
	[SYS_lremovexattr] = NULL,
	[SYS_lseek] = NULL,
	[SYS_lsetxattr] = NULL,
	[SYS_lstat64] = NULL,
	[SYS_lstat] = NULL,
	[SYS_madvise] = NULL,
	[SYS_mbind] = NULL,
	[SYS_migrate_pages] = NULL,
	[SYS_mincore] = NULL,
	[SYS_mkdirat] = NULL,
	[SYS_mkdir] = NULL,
	[SYS_mknodat] = NULL,
	[SYS_mknod] = NULL,
	[SYS_mlockall] = NULL,
	[SYS_mlock] = NULL,
	[SYS_mmap2] = ucSemantics_mmap,
	[SYS_mmap] = ucSemantics_mmap,
	[SYS_modify_ldt] = NULL,
	[SYS_mount] = NULL,
	[SYS_move_pages] = NULL,
	[SYS_mprotect] = NULL,
	[SYS_mpx] = NULL,
	[SYS_mq_getsetattr] = NULL,
	[SYS_mq_notify] = NULL,
	[SYS_mq_open ] = NULL,
	[SYS_mq_timedreceive] = NULL,
	[SYS_mq_timedsend] = NULL,
	[SYS_mq_unlink] = ucSemantics_unlink,
	[SYS_mremap] = NULL,
	[SYS_msync] = NULL,
	[SYS_munlockall] = NULL,
	[SYS_munlock] = NULL,
	[SYS_munmap] = ucSemantics_munmap,
	[SYS_name_to_handle_at] = NULL,
	[SYS_nanosleep] = NULL,
	[SYS__newselect] = NULL,
	[SYS_nfsservctl] = NULL,
	[SYS_nice] = NULL,
	[SYS_oldfstat] = NULL,
	[SYS_oldlstat] = NULL,
	[SYS_oldolduname] = NULL,
	[SYS_oldstat] = NULL,
	[SYS_olduname] = NULL,
	[SYS_openat] = ucSemantics_openat,
	[SYS_open_by_handle_at] = NULL,
	[SYS_open] = ucSemantics_open,
	[SYS_pause] = NULL,
	[SYS_perf_event_open] = NULL,
	[SYS_personality] = NULL,
	[SYS_pipe2] = ucSemantics_pipe,
	[SYS_pipe] = ucSemantics_pipe,
	[SYS_pivot_root] = NULL,
	[SYS_poll] = ucSemantics_IGNORE,
	[SYS_ppoll] = NULL,
	[SYS_prctl] = NULL,
	[SYS_pread64] = ucSemantics_read,
	[SYS_preadv] = ucSemantics_read,
	[SYS_prlimit64] = NULL,
	[SYS_process_vm_readv] = NULL,
	[SYS_process_vm_writev] = NULL,
	[SYS_profil] = NULL,
	[SYS_prof] = NULL,
	[SYS_pselect6] = NULL,
	[SYS_ptrace] = NULL,
	[SYS_putpmsg] = NULL,
	[SYS_pwrite64] = ucSemantics_write,
	[SYS_pwritev] = ucSemantics_write,
	[SYS_query_module] = NULL,
	[SYS_quotactl] = NULL,
	[SYS_readahead] = NULL,
	[SYS_readdir] = NULL,
	[SYS_readlinkat] = NULL,
	[SYS_readlink] = NULL,
	[SYS_read] = ucSemantics_read,
	[SYS_readv] = ucSemantics_read,
	[SYS_reboot] = NULL,
	[SYS_recvfrom] = ucSemantics_read,
	[SYS_recvmmsg] = ucSemantics_read,
	[SYS_recvmsg] = ucSemantics_read,
	[SYS_recv] = ucSemantics_read,
	[SYS_remap_file_pages] = NULL,
	[SYS_removexattr] = NULL,
	[SYS_renameat] = NULL,
	[SYS_rename] = ucSemantics_rename,
	[SYS_request_key] = NULL,
	[SYS_restart_syscall] = NULL,
	[SYS_rmdir] = NULL,
	[SYS_rt_sigaction] = NULL,
	[SYS_rt_sigaction] = NULL,
	[SYS_rt_sigpending] = NULL,
	[SYS_rt_sigprocmask] = NULL,
	[SYS_rt_sigqueueinfo] = NULL,
	[SYS_rt_sigreturn] = NULL,
	[SYS_rt_sigsuspend] = NULL,
	[SYS_rt_sigtimedwait] = NULL,
	[SYS_rt_tgsigqueueinfo] = NULL,
	[SYS_sched_getaffinity] = NULL,
	[SYS_sched_getparam] = NULL,
	[SYS_sched_get_priority_max] = NULL,
	[SYS_sched_get_priority_min] = NULL,
	[SYS_sched_getscheduler] = NULL,
	[SYS_sched_rr_get_interval] = NULL,
	[SYS_sched_setaffinity] = NULL,
	[SYS_sched_setparam] = NULL,
	[SYS_sched_setscheduler] = NULL,
	[SYS_sched_yield] = NULL,
	[SYS_select] = NULL,
	[SYS_sendfile64] = NULL,
	[SYS_sendfile] = NULL,
	[SYS_sendmmsg] = ucSemantics_write,
	[SYS_sendmsg] = ucSemantics_write,
	[SYS_send] = ucSemantics_write,
	[SYS_sendto] = ucSemantics_write,
	[SYS_setdomainname] = NULL,
	[SYS_setfsgid32] = NULL,
	[SYS_setfsgid] = NULL,
	[SYS_setfsuid32] = NULL,
	[SYS_setfsuid] = NULL,
	[SYS_setgid32] = NULL,
	[SYS_setgid] = NULL,
	[SYS_setgroups32] = NULL,
	[SYS_setgroups] = NULL,
	[SYS_sethostname] = NULL,
	[SYS_setitimer] = NULL,
	[SYS_set_mempolicy] = NULL,
	[SYS_setns] = NULL,
	[SYS_setpgid] = NULL,
	[SYS_setpriority] = NULL,
	[SYS_setregid32] = NULL,
	[SYS_setregid] = NULL,
	[SYS_setresgid32] = NULL,
	[SYS_setresgid] = NULL,
	[SYS_setresuid32] = NULL,
	[SYS_setresuid] = NULL,
	[SYS_setreuid32] = NULL,
	[SYS_setreuid] = NULL,
	[SYS_setrlimit] = NULL,
	[SYS_set_robust_list] = NULL,
	[SYS_setsid] = NULL,
	[SYS_setsockopt] = NULL,
	[SYS_set_thread_area] = NULL,
	[SYS_set_thread_area] = NULL,
	[SYS_set_tid_address] = NULL,
	[SYS_settimeofday] = NULL,
	[SYS_setuid32] = NULL,
	[SYS_setuid] = NULL,
	[SYS_setxattr] = NULL,
	[SYS_sgetmask] = NULL,
	[SYS_shutdown] = ucSemantics_shutdown,
	[SYS_sigaction] = NULL,
	[SYS_sigaltstack] = NULL,
	[SYS_signalfd4] = NULL,
	[SYS_signalfd] = NULL,
	[SYS_signal] = NULL,
	[SYS_sigpending] = NULL,
	[SYS_sigprocmask] = NULL,
	[SYS_sigreturn] = NULL,
	[SYS_sigsuspend] = NULL,
	[SYS_socketcall] = NULL,
	[SYS_socketpair] = ucSemantics_socketpair,
	[SYS_socket_subcall + 1] = ucSemantics_socket,
	[SYS_splice] = ucSemantics_splice,
	[SYS_ssetmask] = NULL,
	[SYS_stat64] = NULL,
	[SYS_statfs64] = NULL,
	[SYS_statfs] = NULL,
	[SYS_stat] = NULL,
	[SYS_stime] = NULL,
	[SYS_stty] = NULL,
	[SYS_swapoff] = NULL,
	[SYS_swapon] = NULL,
	[SYS_symlinkat] = NULL,
	[SYS_symlink] = NULL,
	[SYS_sync_file_range] = NULL,
	[SYS_syncfs] = NULL,
	[SYS_sync] = NULL,
	[SYS__sysctl] = NULL,
	[SYS_sysfs] = NULL,
	[SYS_sysinfo] = NULL,
	[SYS_syslog] = NULL,
	[SYS_tee] = NULL,
	[SYS_tgkill] = NULL,
	[SYS_timer_create] = NULL,
	[SYS_timer_delete] = NULL,
	[SYS_timerfd_create] = NULL,
	[SYS_timerfd_gettime] = NULL,
	[SYS_timerfd_settime] = NULL,
	[SYS_timer_getoverrun] = NULL,
	[SYS_timer_gettime] = NULL,
	[SYS_timer_settime] = NULL,
	[SYS_times] = NULL,
	[SYS_time] = NULL,
	[SYS_tkill] = NULL,
	[SYS_truncate64] = NULL,
	[SYS_truncate] = NULL,
	[SYS_ugetrlimit] = NULL,
	[SYS_ulimit] = NULL,
	[SYS_umask] = NULL,
	[SYS_umount2] = NULL,
	[SYS_umount] = NULL,
	[SYS_uname] = NULL,
	[SYS_unlinkat] = NULL,
	[SYS_unlink] = NULL,
	[SYS_unshare] = NULL,
	[SYS_uselib] = NULL,
	[SYS_ustat] = NULL,
	[SYS_utimensat] = NULL,
	[SYS_utimes] = NULL,
	[SYS_utime] = NULL,
	[SYS_vfork] = ucSemantics_IGNORE,
	[SYS_vhangup] = NULL,
	[SYS_vm86old] = NULL,
	[SYS_vm86] = NULL,
	[SYS_vmsplice] = NULL,
	[SYS_vserver] = NULL,
	[SYS_wait4] = ucSemantics_IGNORE,
	[SYS_waitid] = ucSemantics_IGNORE,
	[SYS_waitpid] = ucSemantics_IGNORE,
	[SYS_write] = ucSemantics_write,
	[SYS_writev] = ucSemantics_write,


	[SYS_semop] = NULL,
	[SYS_semget] = NULL,
	[SYS_semctl] = NULL,
	[SYS_semtimedop] = NULL,
	[SYS_ipc_subcall] = NULL,
	[SYS_msgsnd] = NULL,
	[SYS_msgrcv] = NULL,
	[SYS_msgget] = NULL,
	[SYS_msgctl] = NULL,
	[SYS_shmat] = NULL,
	[SYS_shmdt] = NULL,
	[SYS_shmget] = NULL,
	[SYS_shmctl] = NULL,

	[SYS_cloneFirstAction] = ucSemantics_cloneFirstAction

};

