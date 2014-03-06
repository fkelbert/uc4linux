#include "ucEvents.h"

char pid[PID_LEN];
char fd1[FD_LEN];

event *ucSemantics_open(struct tcb *tcp) { return NULL;
//	char relFilename[FILENAME_MAX];
//	char absFilename[FILENAME_MAX];
//
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	// retrieve the filename
//	getString(tcp, tcp->u_arg[0], relFilename, sizeof(relFilename));
//	cwdAbsoluteFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename), 1);
//
//	ucSemantics_do_open(tcp->pid, tcp->u_rval, absFilename, tcp->u_arg[1]);
}

event *ucSemantics_unlink(struct tcb *tcp) { return NULL;
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	getString(tcp, tcp->u_arg[0], identifier1, sizeof(identifier1));
//
//	ucPIP_removeIdentifier(identifier1);
//
//	ucSemantics_log("%5d: unlink(): %s\n", tcp->pid, identifier1);
}

event *ucSemantics_splice(struct tcb *tcp) { return NULL;
//	if (tcp->u_rval <= 0) {
//		return;
//	}
//
//
//
//	ucSemantics_log("%5d: missing semantics for %s (%d)\n", tcp->pid, tcp->s_ent->sys_name, tcp->pid);
//	// TODO. man 2 splice
}



event *ucSemantics_socketpair(struct tcb *tcp) { return NULL;
//	int sockets[2];
//	char sockname1[FILENAME_MAX];
//	char sockname2[FILENAME_MAX];
//
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	if (umoven(tcp, tcp->u_arg[3], sizeof(sockets), (char *) sockets) < 0) {
//		return;
//	}
//
//	if (!getSpecialFilename(tcp->pid, sockets[0], sockname1, sizeof(sockname1))) {
//		ucSemantics_errorExit("Unable to get socket name.");
//	}
//
//	if (!getSpecialFilename(tcp->pid, sockets[1], sockname2, sizeof(sockname2))) {
//		ucSemantics_errorExit("Unable to get socket name.");
//	}
//
//	getIdentifierFD(tcp->pid, sockets[0], identifier1, sizeof(identifier1), sockname1);
//	getIdentifierFD(tcp->pid, sockets[1], identifier2, sizeof(identifier2), sockname2);
//
//	ucPIP_addIdentifier(identifier1, NULL);
//	ucPIP_addIdentifier(identifier2, NULL);
//
//	ucPIP_addAlias(identifier1, identifier2);
//	ucPIP_addAlias(identifier2, identifier1);
//
//	ucSemantics_log("%5d: %s(): %s, %s\n", tcp->pid, tcp->s_ent->sys_name, identifier1, identifier2);
}

event *ucSemantics_shutdown(struct tcb *tcp) { return NULL;
//	ucSemantics_log("%5d: missing semantics for %s (%d)\n", tcp->pid, tcp->s_ent->sys_name, tcp->pid);
	// TODO. man 2 shutdown
}



event *ucSemantics_rename(struct tcb *tcp) { return NULL;
//	char oldRelFilename[FILENAME_MAX];
//	char newRelFilename[FILENAME_MAX];
//	char oldAbsFilename[FILENAME_MAX];
//	char newAbsFilename[FILENAME_MAX];
//
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	getString(tcp,tcp->u_arg[0],oldRelFilename, sizeof(oldRelFilename));
//	getString(tcp,tcp->u_arg[1],newRelFilename, sizeof(newRelFilename));
//
//	cwdAbsoluteFilename(tcp->pid, oldRelFilename, oldAbsFilename, sizeof(oldAbsFilename), 0);
//	cwdAbsoluteFilename(tcp->pid, newRelFilename, newAbsFilename, sizeof(newAbsFilename), 1);
//
//	ucPIP_removeContainer(newAbsFilename);
//
//	if (!ignoreFile(newAbsFilename)) {
//		ucPIP_addIdentifier(oldAbsFilename, newAbsFilename);
//	}
//
//	ucPIP_removeIdentifier(oldAbsFilename);
//
//	ucSemantics_log("%5d: rename(): %s --> %s\n", tcp->pid, oldAbsFilename, newAbsFilename);
}

event *ucSemantics_write(struct tcb *tcp) { return NULL;
//	if (tcp->u_arg[0] <= 0) {
//		// if return value is 0, nothing was written.
//		return;
//	}
//
//	getIdentifierPID(tcp->pid, identifier1, sizeof(identifier1));
//	char *fn = getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier2, sizeof(identifier2), NULL);
//
//	if (g_hash_table_lookup_extended(ignoreFDs, identifier2, NULL, NULL)) {
//		return;
//	}
//
//	if (strstarts(fn, "socket:")) {
//		// For sockets, we won't be able to read the same data again using the same container.
//		// Thus, copy data to all aliases but not to the container into which we are writing itself.
//		ucPIP_copyData(identifier1, identifier2, UC_COPY_INTO_ALL_ALIASES, NULL);
//	}
//	else {
//		ucPIP_copyData(identifier1, identifier2, UC_COPY_INTO_ALL, NULL);
//	}
//
//	ucSemantics_log("%5d: write(): %s --> %s\n", tcp->pid, identifier1, identifier2);
}

event *ucSemantics_socket(struct tcb *tcp) { return NULL;
//	char sockname[FILENAME_MAX];
//	int sfd = tcp->u_rval;
//
//	if (sfd < 0) {
//		return;
//	}
//
//	if (!getSpecialFilename(tcp->pid, sfd, sockname, sizeof(sockname))) {
//		ucSemantics_errorExit("Unable to get socket name.");
//	}
//
//	getIdentifierFD(tcp->pid, sfd, identifier1, sizeof(identifier1), sockname);
//
//	ucPIP_addIdentifier(identifier1, NULL);
//
//	ucSemantics_log("%5d: %s(): %s\n", tcp->pid, tcp->s_ent->sys_name, identifier1);
}

event *ucSemantics_pipe(struct tcb *tcp) { return NULL;
//	int fds[2];
//	char pipename[FILENAME_MAX];
//
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	if (umoven(tcp, tcp->u_arg[0], sizeof(fds), (char *) fds) < 0) {
//		return;
//	}
//
//	if (!getSpecialFilename(tcp->pid, fds[0], pipename, sizeof(pipename))) {
//		strncpy(pipename, "<undef>", sizeof(pipename));
//	}
//
//	getIdentifierFD(tcp->pid, fds[0], identifier1, sizeof(identifier1), pipename);
//	getIdentifierFD(tcp->pid, fds[1], identifier2, sizeof(identifier2), pipename);
//
//	if (g_hash_table_lookup_extended(ignoreFDs, identifier1, NULL, NULL)) {
//		g_hash_table_insert(ignoreFDs, strdup(identifier2), NULL);
//	}
//	else {
//		ucPIP_addIdentifier(identifier1, identifier2);
//		ucSemantics_log("%5d: pipe(): %s %s\n", tcp->pid, identifier1, identifier2);
//	}
}

char *x() {
	return "x";
}

event *ucSemantics_read(struct tcb *tcp) {
	if (tcp->u_arg[0] <= 0) {
		// if return value is 0, nothing was written.
		return NULL;
	}

	toPid(pid, PID_LEN, tcp->pid);
	toFd(fd1, FD_LEN, tcp->u_arg[0]);

	event *ev = createEventWithStdParams(EVENT_NAME_READ, 2);

	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_openat(struct tcb *tcp) { return NULL;
//	char path[FILENAME_MAX];
//	char relFilename[FILENAME_MAX];
//	char absFilename[FILENAME_MAX];
//
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	getString(tcp, tcp->u_arg[1], relFilename, sizeof(relFilename));
//
//	if (isAbsolute(relFilename)) {
//		ucSemantics_log("%5d: %s(%s) resulting in:\n", tcp->pid, tcp->s_ent->sys_name, relFilename);
//		ucSemantics_do_open(tcp->pid, tcp->u_rval, relFilename, tcp->u_arg[2]);
//	}
//	else {
//		char *dir;
//		if (tcp->u_arg[0] == AT_FDCWD) {
//			cwdAbsoluteFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename), 1);
//		}
//		else {
//			dir = getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier1, sizeof(identifier1), NULL);
//			getAbsoluteFilename(dir, relFilename, absFilename, sizeof(absFilename), 1);
//		}
//		ucSemantics_log("%5d: %s(%s, %s) resulting in:\n", tcp->pid, tcp->s_ent->sys_name, dir, relFilename);
//		ucSemantics_do_open(tcp->pid, tcp->u_rval, absFilename, tcp->u_arg[2]);
//	}
}

event *ucSemantics_munmap(struct tcb *tcp) { return NULL;
//	// TODO. man 2 munmap
//	// is it possible to do something useful here?
//	ucSemantics_log("%5d: missing semantics for %s (%d)\n", tcp->pid, tcp->s_ent->sys_name, tcp->pid);
}

event *ucSemantics_mmap(struct tcb *tcp) { return NULL;
//	ucSemantics_log("%5d: missing semantics for %s (%d)\n", tcp->pid, tcp->s_ent->sys_name, tcp->pid);
//	// TODO. man 2 mmap
}

event *ucSemantics_kill(struct tcb *tcp) { return NULL;
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	getIdentifierPID(tcp->pid, identifier1, sizeof(identifier1));
//	getIdentifierPID(tcp->u_arg[0], identifier2, sizeof(identifier2));
//
//	// PID -> PID
//	ucPIP_copyData(identifier1, identifier2, UC_COPY_INTO_ALL, NULL);
//
//	ucSemantics_log("%5d: kill(): %s --> %s\n", tcp->pid, identifier1, identifier2);
//
//	// todo: kill may imply exit (do CTRL+F4 while the monitored gnome-terminal is booting up)
}

event *ucSemantics_fcntl(struct tcb *tcp) { return NULL;
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	if (tcp->u_arg[1] == F_DUPFD
//			|| tcp->u_arg[1] == F_DUPFD_CLOEXEC) {
//		char * fn = getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier1, sizeof(identifier1), NULL);
//		getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2), fn);
//
//		if (g_hash_table_lookup_extended(ignoreFDs, identifier1, NULL, NULL)) {
//			g_hash_table_insert(ignoreFDs, strdup(identifier2), NULL);
//		}
//		else {
//			ucPIP_addIdentifier(identifier1, identifier2);
//			ucSemantics_log("%5d: fcntl(): %s --> %s\n", tcp->pid, identifier1, identifier2);
//		}
//	}
}


event *ucSemantics_ftruncate(struct tcb *tcp) { return NULL;
//	// TODO. man 2 ftruncate; do sth if length == 0
//	ucSemantics_log("%5d: missing semantics for %s (%d)\n", tcp->pid, tcp->s_ent->sys_name, tcp->pid);
}

event *ucSemantics_exit(struct tcb *tcp) { return NULL;
//	ucSemantics_do_process_exit(tcp->pid);
}

event *ucSemantics_execve(struct tcb *tcp) { return NULL;
//	if (initialProcess) {
//		addProcMem(tcp->pid);
//
//		getIdentifierPID(tcp->pid, identifier1, sizeof(identifier1));
//
//		ucPIP_addIdentifier(identifier1, NULL);
//
//		initialProcess = 0;
//	}
//	// TODO: man 2 execve
//	// Remember that execve returns 3 times!
//	// Also consider man 2 open and fcntl: some file descriptors close automatically on exeve()
//	ucSemantics_log("%5d: missing semantics for %s (%d)\n", tcp->pid, tcp->s_ent->sys_name, tcp->pid);
}


event *ucSemantics_eventfd(struct tcb *tcp) { return NULL;
//	ucSemantics_log("%5d: missing semantics for %s (%d)\n", tcp->pid, tcp->s_ent->sys_name, tcp->pid);
//	// TODO. man 2 eventfd
}


event *ucSemantics_dup(struct tcb *tcp) { return NULL;
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	char *fn = getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier1, sizeof(identifier1), NULL);
//	getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2), fn);
//
//	if (g_hash_table_lookup_extended(ignoreFDs, identifier1, NULL, NULL)) {
//		g_hash_table_insert(ignoreFDs, strdup(identifier2), NULL);
//	}
//	else {
//		ucPIP_addIdentifier(identifier1, identifier2);
//		ucSemantics_log("%5d: %s(): %s --> %s\n", tcp->pid, tcp->s_ent->sys_name, identifier1, identifier2);
//	}
}


event *ucSemantics_close(struct tcb *tcp) { return NULL;
//	if (tcp->u_arg[0] < 0) {
//		return;
//	}
//
//	// will set identifier as a side effect
//	ucSemantics_do_close(tcp->pid, tcp->u_arg[0], NULL);
//
//	ucSemantics_log("%5d: close(): %s\n", tcp->pid, identifier1);
}

event *ucSemantics_connect(struct tcb *tcp) { return NULL;
//	ucSemantics_log("%5d: missing semantics for %s (%d)\n", tcp->pid, tcp->s_ent->sys_name, tcp->pid);
//	// TODO. man 2 connect
}

event *ucSemantics_accept(struct tcb *tcp) { return NULL;
//	char sockname1[FILENAME_MAX];
//	int sfd = tcp->u_rval;
//
//	if (sfd < 0) {
//		return;
//	}
//
//	if (!getSpecialFilename(tcp->pid, sfd, sockname1, sizeof(sockname1))) {
//		ucSemantics_errorExit("Unable to get socket name.");
//	}
//
//	getIdentifierFD(tcp->pid, sfd, identifier1, sizeof(identifier1), sockname1);
////	getIdentifierSocket(tcp, tcp->u_arg[1], tcp->u_arg[2], identifier2, sizeof(identifier2), sockname1);
//
//	ucPIP_addIdentifier(identifier1, identifier2);
//
//	ucSemantics_log("%5d: %s(): %s\n", tcp->pid, tcp->s_ent->sys_name, identifier1);
}


event *ucSemantics_sendfile(struct tcb *tcp) { return NULL;
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	getIdentifierFD(tcp->pid, tcp->u_arg[1], identifier1, sizeof(identifier1), NULL);
//	getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier2, sizeof(identifier2), NULL);
//
//	ucPIP_copyData(identifier1, identifier2, UC_COPY_INTO_ALL, NULL);
//
//	ucSemantics_log("%5d: %s(): %s --> %s\n", tcp->pid, tcp->s_ent->sys_name, identifier1, identifier2);
}


event *ucSemantics_dup2(struct tcb *tcp) { return NULL;
//	if (tcp->u_rval < 0) {
//		return;
//	}
//
//	// dup2() does nothing if oldfd == newfd; dup3() would have failed anyway
//	if (tcp->u_arg[0] == tcp->u_rval) {
//		return;
//	}
//
//	// close the new fd first, if necessary
//	ucSemantics_do_close(tcp->pid, tcp->u_rval, NULL);
//
//	char *fn = getIdentifierFD(tcp->pid, tcp->u_arg[0], identifier1, sizeof(identifier1), NULL);
//	getIdentifierFD(tcp->pid, tcp->u_rval, identifier2, sizeof(identifier2), fn);
//
//	if (g_hash_table_lookup_extended(ignoreFDs, identifier1, NULL, NULL)) {
//		g_hash_table_insert(ignoreFDs, strdup(identifier2), NULL);
//	}
//	else {
//		ucPIP_addIdentifier(identifier1, identifier2);
//		ucSemantics_log("%5d: %s(): %s --> %s\n", tcp->pid, tcp->s_ent->sys_name, identifier1, identifier2);
//	}

}


event *ucSemantics_exit_group(struct tcb *tcp) { return NULL;
//	int count;
//	int *tasks;
//
//	// terminate all of the processes' tasks
//	if ((tasks = getProcessTasks(tcp->pid, &count))) {
//		while (count-- > 0) {
//			ucSemantics_do_process_exit(tasks[count]);
//		}
//		free(tasks);
//	}
//
//	ucSemantics_log("%5d: exit_group(): %d\n", tcp->pid, tcp->pid);
}



event *(*ucSemanticsFunct[])(struct tcb *tcp) = {
	[SYS_accept] = ucSemantics_accept,
	[SYS_accept4] = ucSemantics_accept,
	[SYS_access] = ucSemantics_IGNORE,
	[SYS_acct] = ucSemantics_IGNORE,
	[SYS_add_key] = ucSemantics_IGNORE,
	[SYS_adjtimex] = ucSemantics_IGNORE,
	[SYS_afs_syscall] = ucSemantics_IGNORE,
	[SYS_alarm] = ucSemantics_IGNORE,
//	[SYS_bdflush] = ucSemantics_IGNORE,
	[SYS_bind] = ucSemantics_IGNORE,
//	[SYS_break] = ucSemantics_IGNORE,
	[SYS_brk] = ucSemantics_IGNORE,
	[SYS_capget] = ucSemantics_IGNORE,
	[SYS_capset] = ucSemantics_IGNORE,
	[SYS_chdir] = ucSemantics_IGNORE,
	[SYS_chmod] = ucSemantics_IGNORE,
//	[SYS_chown32] = ucSemantics_IGNORE,
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
//	[SYS_fadvise64_64] = ucSemantics_IGNORE,
	[SYS_fadvise64] = ucSemantics_IGNORE,
	[SYS_fallocate] = ucSemantics_IGNORE,
	[SYS_fanotify_init] = ucSemantics_IGNORE,
	[SYS_fanotify_mark] = ucSemantics_IGNORE,
	[SYS_fchdir] = ucSemantics_IGNORE,
	[SYS_fchmodat] = ucSemantics_IGNORE,
	[SYS_fchmod] = ucSemantics_IGNORE,
//	[SYS_fchown32] = ucSemantics_IGNORE,
	[SYS_fchownat] = ucSemantics_IGNORE,
	[SYS_fchown] = ucSemantics_IGNORE,
//	[SYS_fcntl64] = ucSemantics_fcntl,
	[SYS_fcntl] = ucSemantics_fcntl,
	[SYS_fdatasync] = ucSemantics_IGNORE,
	[SYS_fgetxattr] = ucSemantics_IGNORE,
	[SYS_flistxattr] = ucSemantics_IGNORE,
	[SYS_flock] = ucSemantics_IGNORE,
	[SYS_fork] = ucSemantics_IGNORE,
	[SYS_fremovexattr] = ucSemantics_IGNORE,
	[SYS_fsetxattr] = ucSemantics_IGNORE,
//	[SYS_fstat64] = ucSemantics_IGNORE,
//	[SYS_fstatat64] = ucSemantics_IGNORE,
//	[SYS_fstatfs64] = ucSemantics_IGNORE,
	[SYS_fstatfs] = ucSemantics_IGNORE,
	[SYS_fstat] = ucSemantics_IGNORE,
	[SYS_fsync] = ucSemantics_IGNORE,
//	[SYS_ftime] = ucSemantics_IGNORE,
//	[SYS_ftruncate64] = ucSemantics_ftruncate,
	[SYS_ftruncate] = ucSemantics_ftruncate,
	[SYS_futex] = ucSemantics_IGNORE,
	[SYS_futimesat] = ucSemantics_IGNORE,
	[SYS_getcpu] = ucSemantics_IGNORE,
	[SYS_getcwd] = ucSemantics_IGNORE,
	[SYS_getdents64] = ucSemantics_IGNORE,
	[SYS_getdents] = ucSemantics_IGNORE,
//	[SYS_getegid32] = ucSemantics_IGNORE,
	[SYS_getegid] = ucSemantics_IGNORE,
//	[SYS_geteuid32] = ucSemantics_IGNORE,
	[SYS_geteuid] = ucSemantics_IGNORE,
//	[SYS_getgid32] = ucSemantics_IGNORE,
	[SYS_getgid] = ucSemantics_IGNORE,
//	[SYS_getgroups32] = ucSemantics_IGNORE,
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
//	[SYS_getresgid32] = ucSemantics_IGNORE,
	[SYS_getresgid] = ucSemantics_IGNORE,
//	[SYS_getresuid32] = ucSemantics_IGNORE,
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
//	[SYS_getuid32] = ucSemantics_IGNORE,
	[SYS_getuid] = ucSemantics_IGNORE,
	[SYS_getxattr] = ucSemantics_IGNORE,
//	[SYS_gtty] = ucSemantics_IGNORE,
//	[SYS_idle] = ucSemantics_IGNORE,
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
//	[SYS_ipc] = ucSemantics_IGNORE,
	[SYS_kexec_load] = ucSemantics_IGNORE,
	[SYS_keyctl] = ucSemantics_IGNORE,
	[SYS_kill] = ucSemantics_kill,
//	[SYS_lchown32] = ucSemantics_IGNORE,
	[SYS_lchown] = ucSemantics_IGNORE,
	[SYS_lgetxattr] = ucSemantics_IGNORE,
	[SYS_linkat] = ucSemantics_IGNORE,
	[SYS_link] = ucSemantics_IGNORE,
	[SYS_listen] = ucSemantics_IGNORE,
	[SYS_listxattr] = ucSemantics_IGNORE,
	[SYS_llistxattr] = ucSemantics_IGNORE,
//	[SYS__llseek] = ucSemantics_IGNORE,
//	[SYS__llseek] = ucSemantics_IGNORE,
//	[SYS_lock] = ucSemantics_IGNORE,
	[SYS_lookup_dcookie] = ucSemantics_IGNORE,
	[SYS_lremovexattr] = ucSemantics_IGNORE,
	[SYS_lseek] = ucSemantics_IGNORE,
	[SYS_lsetxattr] = ucSemantics_IGNORE,
//	[SYS_lstat64] = ucSemantics_IGNORE,
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
//	[SYS_mmap2] = ucSemantics_mmap,
	[SYS_mmap] = ucSemantics_mmap,
	[SYS_modify_ldt] = ucSemantics_IGNORE,
	[SYS_mount] = ucSemantics_IGNORE,
	[SYS_move_pages] = ucSemantics_IGNORE,
	[SYS_mprotect] = ucSemantics_IGNORE,
//	[SYS_mpx] = ucSemantics_IGNORE,
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
//	[SYS__newselect] = ucSemantics_IGNORE,
	[SYS_nfsservctl] = ucSemantics_IGNORE,
//	[SYS_nice] = ucSemantics_IGNORE,
//	[SYS_oldfstat] = ucSemantics_IGNORE,
//	[SYS_oldlstat] = ucSemantics_IGNORE,
//	[SYS_oldolduname] = ucSemantics_IGNORE,
//	[SYS_oldstat] = ucSemantics_IGNORE,
//	[SYS_olduname] = ucSemantics_IGNORE,
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
//	[SYS_profil] = ucSemantics_IGNORE,
//	[SYS_prof] = ucSemantics_IGNORE,
	[SYS_pselect6] = ucSemantics_IGNORE,
	[SYS_ptrace] = ucSemantics_IGNORE,
	[SYS_putpmsg] = ucSemantics_IGNORE,
	[SYS_pwrite64] = ucSemantics_write,
	[SYS_pwritev] = ucSemantics_write,
	[SYS_query_module] = ucSemantics_IGNORE,
	[SYS_quotactl] = ucSemantics_IGNORE,
	[SYS_readahead] = ucSemantics_IGNORE,
//	[SYS_readdir] = ucSemantics_IGNORE,
	[SYS_readlinkat] = ucSemantics_IGNORE,
	[SYS_readlink] = ucSemantics_IGNORE,
	[SYS_read] = ucSemantics_read,
	[SYS_readv] = ucSemantics_read,
	[SYS_reboot] = ucSemantics_IGNORE,
	[SYS_recvfrom] = ucSemantics_read,
	[SYS_recvmmsg] = ucSemantics_read,
	[SYS_recvmsg] = ucSemantics_read,
//	[SYS_recv] = ucSemantics_read,
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
//	[SYS_sendfile64] = ucSemantics_sendfile,
	[SYS_sendfile] = ucSemantics_sendfile,
	[SYS_sendmmsg] = ucSemantics_write,
	[SYS_sendmsg] = ucSemantics_write,
//	[SYS_send] = ucSemantics_write,
	[SYS_sendto] = ucSemantics_write,
	[SYS_setdomainname] = ucSemantics_IGNORE,
//	[SYS_setfsgid32] = ucSemantics_IGNORE,
	[SYS_setfsgid] = ucSemantics_IGNORE,
//	[SYS_setfsuid32] = ucSemantics_IGNORE,
	[SYS_setfsuid] = ucSemantics_IGNORE,
//	[SYS_setgid32] = ucSemantics_IGNORE,
	[SYS_setgid] = ucSemantics_IGNORE,
//	[SYS_setgroups32] = ucSemantics_IGNORE,
	[SYS_setgroups] = ucSemantics_IGNORE,
	[SYS_sethostname] = ucSemantics_IGNORE,
	[SYS_setitimer] = ucSemantics_IGNORE,
	[SYS_set_mempolicy] = ucSemantics_IGNORE,
	[SYS_setns] = ucSemantics_IGNORE,
	[SYS_setpgid] = ucSemantics_IGNORE,
	[SYS_setpriority] = ucSemantics_IGNORE,
//	[SYS_setregid32] = ucSemantics_IGNORE,
	[SYS_setregid] = ucSemantics_IGNORE,
//	[SYS_setresgid32] = ucSemantics_IGNORE,
	[SYS_setresgid] = ucSemantics_IGNORE,
//	[SYS_setresuid32] = ucSemantics_IGNORE,
	[SYS_setresuid] = ucSemantics_IGNORE,
//	[SYS_setreuid32] = ucSemantics_IGNORE,
	[SYS_setreuid] = ucSemantics_IGNORE,
	[SYS_setrlimit] = ucSemantics_IGNORE,
	[SYS_set_robust_list] = ucSemantics_IGNORE,
	[SYS_setsid] = ucSemantics_IGNORE,
	[SYS_setsockopt] = ucSemantics_IGNORE,
	[SYS_set_thread_area] = ucSemantics_IGNORE,
	[SYS_set_thread_area] = ucSemantics_IGNORE,
	[SYS_set_tid_address] = ucSemantics_IGNORE,
	[SYS_settimeofday] = ucSemantics_IGNORE,
//	[SYS_setuid32] = ucSemantics_IGNORE,
	[SYS_setuid] = ucSemantics_IGNORE,
	[SYS_setxattr] = ucSemantics_IGNORE,
//	[SYS_sgetmask] = ucSemantics_IGNORE,
	[SYS_shutdown] = ucSemantics_shutdown,
//	[SYS_sigaction] = ucSemantics_IGNORE,
	[SYS_sigaltstack] = ucSemantics_IGNORE,
	[SYS_signalfd4] = ucSemantics_IGNORE,
	[SYS_signalfd] = ucSemantics_IGNORE,
//	[SYS_signal] = ucSemantics_IGNORE,
//	[SYS_sigpending] = ucSemantics_IGNORE,
//	[SYS_sigprocmask] = ucSemantics_IGNORE,
//	[SYS_sigreturn] = ucSemantics_IGNORE,
//	[SYS_sigsuspend] = ucSemantics_IGNORE,
//	[SYS_socketcall] = ucSemantics_IGNORE,
	[SYS_socketpair] = ucSemantics_socketpair,
//	[SYS_socket_subcall + 1] = ucSemantics_socket,
	[SYS_splice] = ucSemantics_splice,
//	[SYS_ssetmask] = ucSemantics_IGNORE,
//	[SYS_stat64] = ucSemantics_IGNORE,
//	[SYS_statfs64] = ucSemantics_IGNORE,
	[SYS_statfs] = ucSemantics_IGNORE,
	[SYS_stat] = ucSemantics_IGNORE,
//	[SYS_stime] = ucSemantics_IGNORE,
//	[SYS_stty] = ucSemantics_IGNORE,
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
//	[SYS_truncate64] = ucSemantics_IGNORE,
	[SYS_truncate] = ucSemantics_IGNORE,
//	[SYS_ugetrlimit] = ucSemantics_IGNORE,
//	[SYS_ulimit] = ucSemantics_IGNORE,
	[SYS_umask] = ucSemantics_IGNORE,
	[SYS_umount2] = ucSemantics_IGNORE,
//	[SYS_umount] = ucSemantics_IGNORE,
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
//	[SYS_vm86old] = ucSemantics_IGNORE,
//	[SYS_vm86] = ucSemantics_IGNORE,
	[SYS_vmsplice] = ucSemantics_IGNORE,
	[SYS_vserver] = ucSemantics_IGNORE,
	[SYS_wait4] = ucSemantics_IGNORE,
	[SYS_waitid] = ucSemantics_IGNORE,
//	[SYS_waitpid] = ucSemantics_IGNORE,
	[SYS_write] = ucSemantics_write,
	[SYS_writev] = ucSemantics_write,


	[SYS_semop] = ucSemantics_IGNORE,
	[SYS_semget] = ucSemantics_IGNORE,
	[SYS_semctl] = ucSemantics_IGNORE,
	[SYS_semtimedop] = ucSemantics_IGNORE,
//	[SYS_ipc_subcall] = ucSemantics_IGNORE,
	[SYS_msgsnd] = ucSemantics_IGNORE,
	[SYS_msgrcv] = ucSemantics_IGNORE,
	[SYS_msgget] = ucSemantics_IGNORE,
	[SYS_msgctl] = ucSemantics_IGNORE,
	[SYS_shmat] = ucSemantics_IGNORE,
	[SYS_shmdt] = ucSemantics_IGNORE,
	[SYS_shmget] = ucSemantics_IGNORE,
	[SYS_shmctl] = ucSemantics_IGNORE,

//	[SYS_cloneFirstAction] = ucSemantics_cloneFirstAction
};
