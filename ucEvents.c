#include "ucEvents.h"
#include "xlat/socketlayers.h"
#include <string.h>

char pid[PID_LEN];
char fd1[FD_LEN];
char fd2[FD_LEN];

#define ucSemantics_errorExit(msg) { \
			fprintf(stderr, "%s\n", msg); \
			fprintf(stderr, "Happened in: %s:%d\n", __FILE__, __LINE__); \
			fprintf(stderr, "Exiting.\n"); \
			exit (1); \
		}



bool ignoreFilename(char *fn) {
	int fnlen = strlen(fn);
	int ignlen;

	int i = 0;
	const char *ign = ignoredFiles[0];

	while (ign != NULL) {
		ignlen = strlen(ign);
		if (ignlen <= fnlen && strncmp(fn, ign, ignlen) == 0) {
			return true;
		}
		ign = ignoredFiles[++i];
	}

	return false;
}


// get the processes' current working directory
bool getCwd(char *cwd, int cwdlen, int pid) {
	char procfsPath[PATH_MAX];
	int read;

	snprintf(procfsPath, sizeof(procfsPath), "/proc/%d/cwd", pid);
	if ((read = readlink(procfsPath, cwd,  cwdlen - 1)) == -1) {
		return false;
	}
	cwd[read] = '\0';
	return true;
}

// get the processes' command Line parameters
int getCmdline(char *cmdline, int len, int pid) {
	char procfsPath[PATH_MAX];
	int read;

	snprintf(procfsPath, sizeof(procfsPath), "/proc/%d/cmdline", pid);

	FILE *f = fopen(procfsPath, "r");
	if (f == NULL) {
		return false;
	}
	ssize_t s = 0;
	int res=0;
	char* line=NULL;
// use a while loop for multiple lines.
// for on line it works smoothly. never found a command with more than one line.
// not sure if it works in case command line is longer than one line.
// not even sure if that is possible at all.

	cmdline[0] = '\0';
	while ((res=getline(&line, &s, f)) > 0){
		int i=0;
		for (i=0;i<res-1;i++){
			if ((line[i]=='\0')||
				(line[i]=='\n')||
				(line[i]=='\r')){
				line[i]=' ';
			}
		}
		strcat(cmdline,line);
//		printf ("line = [%s], cmdline = [%s], res=%d\n",line, cmdline, res);		
		line=NULL;
		s=0;
	}

	fclose(f);

	return res;
}

char *toAbsFilename(long pid, char *relFilename, char *absFilename, int absFilenameLen, bool tryIfNonexistent) {
	ssize_t read;
	char *absNew;
	char procfsPath[PATH_MAX];
	char cwd[PATH_MAX];
	char concatPath[2 * PATH_MAX];

	// we are done!
	if (isAbsolutePath(relFilename)) {
		strncpy(absFilename, relFilename, absFilenameLen - 1);
		absFilename[absFilenameLen - 1] = '\0';
		return (absFilename);
	}

	if (absFilenameLen < 1) {
		return NULL ;
	}

	if (!getCwd(cwd, sizeof(cwd), pid)) {
		return NULL;
	}

	// concatenate cwd and relative filename and convert it to an absolute filename
	snprintf(concatPath, sizeof(concatPath), "%s/%s", cwd, relFilename);

	// was resolving successful and is the provided buffer large enough?
	if ((absNew = realpath(concatPath, NULL)) == NULL || strlen(absNew) >= absFilenameLen) {
		if (tryIfNonexistent) {
			/*
			 * For rename(), realpath() fails because one of the arguments does (very likely)
			 * not exist. Thus, resolve the path independently and append the filename. This
			 * is what happens within in this block.
			 */
			char *file = strrchr(concatPath, '/') + 1;

			char relPath[sizeof(concatPath)];
			char *absPath;

			memcpy(relPath, concatPath, strlen(concatPath) - strlen(file));

			if ((absPath = realpath(relPath, NULL)) == NULL || strlen(absPath) + strlen(file) >= absFilenameLen) {
				return NULL ;
			}

			absNew = calloc(absFilenameLen, sizeof(char));
			snprintf(absNew, absFilenameLen, "%s/%s", absPath, file);

			free(absPath);
		}
	}



	// as we have tested the size before, an additional null byte is written by strncpy()
	strncpy(absFilename, absNew, absFilenameLen);

	free(absNew);

	return absFilename;
}


int *getIntDirEntries(long pid, int *count, char *procSubPath) {
	char procfsPath[PATH_MAX];
	int ret;
	DIR *dir;
	struct dirent *ent;
	int *fds;
	int size = 8; // do not init to 0!
	*count = 0;

	ret = snprintf(procfsPath, sizeof(procfsPath), PROCFS_MNT"/%ld/%s", pid, procSubPath);

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
		closedir(dir);
	}
	else {
		perror("opendir");
		ucSemantics_errorExit("Failed to open procfs directory");
	}

	return (fds);
}


bool getIPsAndPorts(int pid, int inode, char *localIP, char *localPort, char *remoteIP, char *remotePort) {
	char conFile[sizeof(PROCFS_MNT"/%u/net/tcp") + 6];
	snprintf(conFile, sizeof(conFile), PROCFS_MNT"/%u/net/tcp", pid);

	FILE *cons = fopen(conFile, "r");
	if (cons == NULL) {
		return false;
	}

	char *line;
	ssize_t s = 0;
	int grbg_x;
	char grbg[6][32];
	int inodeFound;
	bool first = true;
	bool found = false;

	for (line = NULL; !found && getline(&line, &s, cons) >= 0; line = NULL, s = 0){
		if (first) {
			first = false;
			continue;
		}

		sscanf(line, " %d: %8s:%4s %8s:%4s %31s %31s %31s %31s %31s %31s %d",
				&grbg_x, localIP, localPort, remoteIP, remotePort,
				grbg[0], grbg[1], grbg[2], grbg[3], grbg[4], grbg[5],
				&inodeFound);

		// Have we found the correct entry?
		// If so, adjust results and break.
		if (inode == inodeFound) {
			long lport = strtol(localPort, NULL, 16);
			long rport = strtol(remotePort, NULL, 16);
			snprintf(localPort, 6, "%u", (unsigned int) lport);
			snprintf(remotePort, 6, "%u", (unsigned int) rport);

			int a[4];
			sscanf(localIP, "%02x%02x%02x%02x", &a[0], &a[1], &a[2], &a[3]);
			snprintf(localIP, 16, "%d.%d.%d.%d", a[3], a[2], a[1], a[0]);

			sscanf(remoteIP, "%02x%02x%02x%02x", &a[0], &a[1], &a[2], &a[3]);
			snprintf(remoteIP, 16, "%d.%d.%d.%d", a[3], a[2], a[1], a[0]);

			found = true;
		}
	}

	fclose(cons);

	return found;
}


int getParentPid(int pid) {
	char statusFile[sizeof(PROCFS_MNT"/%u/status") + 6];
	snprintf(statusFile, sizeof(statusFile), PROCFS_MNT"/%u/status", pid);

	FILE *stat = fopen(statusFile, "r");
	if (stat == NULL) {
		return -1;
	}

	char *line;
	ssize_t s = 0;
	int ppid = -1;
	bool found = false;

	for (line = NULL; !found && getline(&line, &s, stat) >= 0; line = NULL, s = 0){
		if (sscanf(line, "PPid: %d", &ppid) == 1) {
			found = true;
		}
	}

	fclose(stat);
	return ppid;
}



int *getProcessTasks(long pid, int *count) {
	return getIntDirEntries(pid, count, "task");
}

event *ucSemantics_unlink(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX];

	// TODO: we signal the desired unlink,
	// because we cannot (easily) retrieve the filename
	// after execution of unlink.
	// As unlink might fail, this implementation might be wrong
	// in this case. 
	// A correct implementation would save the filename upon syscall
	// enter and reuse this saved filename upon exiting to in fact
	// signal the actual unlink. Yet, it might be the case that 
	// multiple/threads processes unlink different files concurrently,
	// which is why we cannot use a single variable, but we would need
	// a synchronized map or something similar... oh dear.
/*
	if (tcp->u_rval < 0) {
		return NULL;
	}
*/
	toString(relFilename, tcp, tcp->u_arg[0]);
	if (!(toAbsFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename), false))
			|| ignoreFilename(absFilename)) {
		return NULL;
	}

	return do_unlink(absFilename);
}

event *ucSemantics_unlinkat(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX * 2];
	char dir[FILENAME_MAX] = "";

	// TODO: we signal the desired unlink,
	// because we cannot (easily) retrieve the filename
	// after execution of unlink.
	// As unlink might fail, this implementation might be wrong
	// in this case.
	// A correct implementation would save the filename upon syscall
	// enter and reuse this saved filename upon exiting to in fact
	// signal the actual unlink. Yet, it might be the case that
	// multiple/threads processes unlink different files concurrently,
	// which is why we cannot use a single variable, but we would need
	// a synchronized map or something similar... oh dear.
/*
	if (tcp->u_rval < 0) {
		return NULL;
	}
*/
	toString(relFilename, tcp, tcp->u_arg[1]);

	if (!isAbsolutePath(relFilename)) {
		if (tcp->u_arg[0] == AT_FDCWD) {
			getCwd(dir, sizeof(dir), tcp->pid);
		}
		else {
			getfdpath(tcp, tcp->u_arg[0], dir, sizeof(dir));
		}
	}

	if (strlen(dir) > 0) {
		snprintf(absFilename, sizeof(absFilename), "%s/%s", dir, relFilename);
		return do_unlink(absFilename);
	}
	else {
		return do_unlink(relFilename);
	}
}

event *do_unlink(char *filename) {
	event *ev = createEventWithStdParams(EVENT_NAME_UNLINK, 1);
	if (addParam(ev, createParam("filename", filename))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_splice(struct tcb *tcp) {
	char filename[FILENAME_MAX];

	if (tcp->u_rval <= 0) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);
	toFd(fd2, tcp->u_arg[2]);

	getfdpath(tcp, tcp->u_arg[2], filename, sizeof(filename));
	if (ignoreFilename(filename)) {
		return NULL;
	}

	event *ev = createEventWithStdParams(EVENT_NAME_SPLICE, 4);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("srcfd", fd1))
		&& addParam(ev, createParam("dstfd", fd2))
		&& addParam(ev, createParam("dstfilename", filename))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_tee(struct tcb *tcp) {
	char filename[FILENAME_MAX];

	if (tcp->u_rval <= 0) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);
	toFd(fd2, tcp->u_arg[1]);

	getfdpath(tcp, tcp->u_arg[1], filename, sizeof(filename));
	if (ignoreFilename(filename)) {
		return NULL;
	}

	event *ev = createEventWithStdParams(EVENT_NAME_TEE, 4);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("srcfd", fd1))
		&& addParam(ev, createParam("dstfd", fd2))
		&& addParam(ev, createParam("dstfilename", filename))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_shutdown(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);

	char *how = "";

	switch (tcp->u_arg[1]) {
	case SHUT_RD:
		how = "RD";
		break;
	case SHUT_WR:
		how = "WR";
		break;
	case SHUT_RDWR:
		how = "RDWR";
		break;
	}

	event *ev = createEventWithStdParams(EVENT_NAME_SHUTDOWN, 3);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))
		&& addParam(ev, createParam("how", how))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_write(struct tcb *tcp) {
	char filename[FILENAME_MAX];

	// We are interested in desired events only
	if (is_actual(tcp)) {
		return NULL;
	}

	getfdpath(tcp, tcp->u_arg[0], filename, sizeof(filename));
	if (ignoreFilename(filename)) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);

	event *ev = createEventWithStdParams(EVENT_NAME_WRITE, 4);

	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))
		&& addParam(ev, createParam("filename", filename))
		&& addParam(ev, createParam("allowImpliesActual", "true"))) {
		return ev;
	}

	return NULL;
}


/*
 * It seems we do not need to handle cross-process
 * file descriptor passing via sendmsg.
 * This has been solved by resolving socketnames
 * (e.g. socket:[234251]) once they are used _after_ they
 * have been passed.
 */
//event *ucSemantics_sendmsg(struct tcb *tcp) {
//	/*
//	 * sendmsg() allows to pass file descriptors between
//	 * processes (control messages) if
//	 * cmsg_level = SOL_SOCKET.
//	 *
//	 * Most code here has been heavily inspired from
//	 * strace::net.c::do_msghdr
//	 * and
//	 * strace::net.c::printcmsghdr
//	 */
//
//	char socketname[FILENAME_MAX];
//
//	if (tcp->u_arg < 0) {
//		return NULL;
//	}
//
//	getfdpath(tcp, tcp->u_arg[0], socketname, sizeof(socketname));
//	if (ignoreFilename(socketname)) {
//		return NULL;
//	}
//
//	toPid(pid, tcp->pid);
//	toFd(fd1, tcp->u_arg[0]);
//
//	struct msghdr msg;
//
//	if ((umove(tcp, tcp->u_arg[1], &msg) < 0) || !(msg.msg_controllen)) {
//		// fallback to standard write semantics if
//		// this is not a control message
//		return ucSemantics_write(tcp);
//	}
//
//	// allocate memory for control message
//	struct cmsghdr *cmsg = msg.msg_controllen < sizeof(struct cmsghdr) ? NULL : malloc(msg.msg_controllen);
//	if (cmsg == NULL) {
//		return ucSemantics_write(tcp);
//	}
//
//
//	// convert message into control message
//	if (umoven(tcp, (unsigned long) msg.msg_control, msg.msg_controllen, (char *) cmsg) < 0
//		|| cmsg->cmsg_level != SOL_SOCKET) {
//
//		// fallback to standard write semantics in
//		// case of error or if not SOL_SOCKET
//		free(cmsg);
//		return ucSemantics_write(tcp);
//	}
//
//	unsigned long cmsg_len = (msg.msg_controllen < cmsg->cmsg_len) ? msg.msg_controllen : cmsg->cmsg_len;
//
//	if (cmsg->cmsg_type != SCM_RIGHTS || CMSG_LEN(sizeof(int)) > cmsg_len) {
//		free(cmsg);
//		return ucSemantics_write(tcp);
//	}
//
//	// gather all passed file descriptors
//
//	int *fds = (int *) CMSG_DATA(cmsg);
//
//	char allFds[PATH_MAX];
//	int ptr = 0;
//	int written = 0;
//
//	while ((char *) fds < ((char *) cmsg + cmsg_len)) {
//		written = snprintf(allFds+ptr, PATH_MAX - ptr - 1, "%u:", *fds++);
//		if (written > 0) {
//			ptr += written;
//		}
//	}
//
//	free(cmsg);
//
//	event *ev = createEventWithStdParams(EVENT_NAME_SENDMSG, 3);
//
//	if (addParam(ev, createParam("pid", pid))
//		&& addParam(ev, createParam("fd", fd1))
//		&& addParam(ev, createParam("socketname", fd1))
//		&& addParam(ev, createParam("shared_fds", allFds))) {
//		return ev;
//	}
//
//	return NULL;
//}


event *ucSemantics_socket(struct tcb *tcp) {
	char socketname[PATH_MAX];

	if (tcp->u_rval < 0) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_rval);

	getfdpath(tcp, tcp->u_rval, socketname, sizeof(socketname));

	// cf. strace:net.c
	char *domain = (char*) xlookup(domains, tcp->u_arg[0]);

	// cf. strace:net.c: SOCK_TYPE_MASK == 0xf
	char *type = (char*) xlookup(socktypes, tcp->u_arg[1] & 0xf);

	event *ev = createEventWithStdParams(EVENT_NAME_SOCKET, 5);

	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))
		&& addParam(ev, createParam("domain", domain))
		&& addParam(ev, createParam("type", type))
		&& addParam(ev, createParam("socketname", socketname))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_socketpair(struct tcb *tcp) {
	char socketname1[PATH_MAX];
	char socketname2[PATH_MAX];
	int fds[2];

	if (tcp->u_rval < 0) {
		return NULL;
	}

	if (umoven(tcp, tcp->u_arg[3], sizeof fds, (char *) fds) < 0) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, fds[0]);
	toFd(fd2, fds[1]);

	getfdpath(tcp, fds[0], socketname1, sizeof(socketname1));
	getfdpath(tcp, fds[1], socketname2, sizeof(socketname2));

	// cf. strace:net.c
	char *domain = (char*) xlookup(domains, tcp->u_arg[0]);

	// cf. strace:net.c: SOCK_TYPE_MASK == 0xf
	char *type = (char*) xlookup(socktypes, tcp->u_arg[1] & 0xf);

	event *ev = createEventWithStdParams(EVENT_NAME_SOCKETPAIR, 7);

	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd1", fd1))
		&& addParam(ev, createParam("fd2", fd2))
		&& addParam(ev, createParam("socketname1", socketname1))
		&& addParam(ev, createParam("socketname2", socketname2))
		&& addParam(ev, createParam("domain", domain))
		&& addParam(ev, createParam("type", type))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_pipe(struct tcb *tcp) {
	int fds[2];
	char fd1[FD_LEN];
	char fd2[FD_LEN];
	char pipename[FILENAME_MAX];

	if (tcp->u_rval < 0) {
		return NULL;
	}

	if (umoven(tcp, tcp->u_arg[0], sizeof(fds), (char *) fds) < 0) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1,fds[0]);
	toFd(fd2,fds[1]);

	getfdpath(tcp, fds[0], pipename, sizeof(pipename));

	event *ev = createEventWithStdParams(EVENT_NAME_PIPE, 4);

	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd1", fd1))
		&& addParam(ev, createParam("fd2", fd2))
		&& addParam(ev, createParam("pipename", pipename))) {
		return ev;
	}

	return NULL;
}


event *do_open(struct tcb *tcp, str evName, char *absFilename, int flags) {
	char *trunc = "false";

	if (ignoreFilename(absFilename)) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_rval);

	if (IS_FLAG_SET(flags, O_TRUNC) && (IS_FLAG_SET(flags, O_RDWR) || IS_FLAG_SET(flags, O_WRONLY))) {
		trunc = "true";
	}

	event *ev = createEventWithStdParams(evName, 4);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))
		&& addParam(ev, createParam("filename", absFilename))
		&& addParam(ev, createParam("trunc", trunc))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_openat(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX * 2];
	char dir[FILENAME_MAX] = "";

	if (tcp->u_rval < 0) {
		return NULL;
	}

	toString(relFilename, tcp, tcp->u_arg[1]);

	if (!isAbsolutePath(relFilename)) {
		if (tcp->u_arg[0] == AT_FDCWD) {
			getCwd(dir, sizeof(dir), tcp->pid);
		}
		else {
			getfdpath(tcp, tcp->u_arg[0], dir, sizeof(dir));
		}
	}

	if (strlen(dir) > 0) {
		snprintf(absFilename, sizeof(absFilename), "%s/%s", dir, relFilename);
		return do_open(tcp, EVENT_NAME_OPEN, absFilename, tcp->u_arg[2]);
	}
	else {
		return do_open(tcp, EVENT_NAME_OPEN, relFilename, tcp->u_arg[2]);
	}
}


event *ucSemantics_open(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX];

	if (tcp->u_rval < 0) {
		return NULL;
	}

	toString(relFilename, tcp, tcp->u_arg[0]);

	if (!(toAbsFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename), false))
			|| ignoreFilename(absFilename)) {
		return NULL;
	}

	return do_open(tcp, EVENT_NAME_OPEN, absFilename, tcp->u_arg[1]);
}



event *ucSemantics_creat(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX];

	if (tcp->u_rval < 0) {
		return NULL;
	}

	toString(relFilename, tcp, tcp->u_arg[0]);

	if (!(toAbsFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename), false))
			|| ignoreFilename(absFilename)) {
		return NULL;
	}

	return do_open(tcp, EVENT_NAME_CREAT, absFilename, O_CREAT|O_WRONLY|O_TRUNC);
}


event *ucSemantics_read(struct tcb *tcp) {
	char filename[FILENAME_MAX];

	if (tcp->u_rval <= 0) {
		// if return value is 0, nothing was read.
		return NULL;
	}

	getfdpath(tcp, tcp->u_arg[0], filename, sizeof(filename));
	if (ignoreFilename(filename)) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);

	event *ev = createEventWithStdParams(EVENT_NAME_READ, 3);

	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))
		&& addParam(ev, createParam("filename", filename))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_rename(struct tcb *tcp) {
	char oldRelFilename[FILENAME_MAX];
	char oldAbsFilename[FILENAME_MAX];
	char newRelFilename[FILENAME_MAX];
	char newAbsFilename[FILENAME_MAX];

	if (tcp->u_rval < 0) {
		return NULL;
	}

	toString(oldRelFilename, tcp, tcp->u_arg[0]);
	if (!(toAbsFilename(tcp->pid, oldRelFilename, oldAbsFilename, sizeof(oldAbsFilename), true))
			|| ignoreFilename(oldAbsFilename)) {
		return NULL;
	}

	toString(newRelFilename, tcp, tcp->u_arg[1]);
	if (!(toAbsFilename(tcp->pid, newRelFilename, newAbsFilename, sizeof(newAbsFilename), true))
				|| ignoreFilename(newAbsFilename)) {
		return NULL;
	}

	toPid(pid, tcp->pid);

	event *ev = createEventWithStdParams(EVENT_NAME_RENAME, 2);
	if (addParam(ev, createParam("old", oldAbsFilename))
		&& addParam(ev, createParam("new", newAbsFilename))) {
		return ev;
	}

	return NULL;
}



event *ucSemantics_munmap(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return NULL;
	}

	toPid(pid, tcp->pid);

	char addr[12];
	snprintf(addr, sizeof(addr), "%#lx", tcp->u_arg[0]);

	event *ev = createEventWithStdParams(EVENT_NAME_MUNMAP, 2);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("addr", addr))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_mmap(struct tcb *tcp) {
	char filename[FILENAME_MAX];
	char flags[32];
	char mapaddr[12];
	int nul_seen;
	int written = 0;

	// MAP_ANONYMOUS flag is not interesting. Return.
	if (IS_FLAG_SET(tcp->u_arg[3], MAP_ANONYMOUS)) {
		return NULL;
	}

	if (tcp->u_rval < 0 || tcp->u_arg[4] < 0) {
		return NULL;
	}

	getfdpath(tcp, tcp->u_arg[4], filename, sizeof(filename));
	if (ignoreFilename(filename)) {
		return NULL;
	}

	// check READ/WRITE flags
	if (IS_FLAG_SET(tcp->u_arg[2], PROT_READ)) {
		written += snprintf(flags + written, sizeof(flags) - written, "r");
	}
	if (IS_FLAG_SET(tcp->u_arg[2], PROT_WRITE)) {
		written += snprintf(flags + written, sizeof(flags) - written, "w");
	}

	// if we have not written PROT_READ or PROT_WRITE at this point
	// the call is not interesting and we return
	if (written == 0) {
		return NULL;
	}

	if (IS_FLAG_SET(tcp->u_arg[2], MAP_SHARED)) {
		written += snprintf(flags + written, sizeof(flags) - written, "s");
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[4]);
	snprintf(mapaddr, sizeof(mapaddr), "%#lx", tcp->u_rval);

	event *ev = createEventWithStdParams(EVENT_NAME_MMAP, 4);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))
		&& addParam(ev, createParam("addr", mapaddr))
		&& addParam(ev, createParam("flags", flags))) {
		return ev;
	}

	return NULL;
}

// todo: kill may imply exit (do CTRL+F4 while the monitored gnome-terminal is booting up)
event *ucSemantics_kill(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return NULL;
	}

	char pid2[PID_LEN];

	toPid(pid, tcp->pid);
	toPid(pid2, tcp->u_arg[0]);

	event *ev = createEventWithStdParams(EVENT_NAME_KILL, 2);
	if (addParam(ev, createParam("srcPid", pid))
		&& addParam(ev, createParam("dstPid", pid2))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_fcntl(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return NULL;
	}

	if (tcp->u_arg[1] != F_DUPFD && tcp->u_arg[1] != F_DUPFD_CLOEXEC) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);
	toFd(fd2, tcp->u_rval);

	event *ev = createEventWithStdParams(EVENT_NAME_FCNTL, 4);
	if (addParam(ev, createParam("operation", "dupfd"))
		&& addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("oldfd", fd1))
		&& addParam(ev, createParam("newfd", fd2))) {
		return ev;
	}

	return NULL;
}


event *ucSemantics_ftruncate(struct tcb *tcp) {
	if (tcp->u_rval < 0 || tcp->u_arg[1] != 0) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);

	event *ev = createEventWithStdParams(EVENT_NAME_FTRUNCATE, 2);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_truncate(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX];

	if (tcp->u_rval < 0 || tcp->u_arg[1] == 0) {
		return NULL;
	}

	toString(relFilename, tcp, tcp->u_arg[0]);
	if (!(toAbsFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename), false))
			|| ignoreFilename(absFilename)) {
		return NULL;
	}

	toPid(pid, tcp->pid);

	event *ev = createEventWithStdParams(EVENT_NAME_TRUNCATE, 1);
	if (addParam(ev, createParam("filename", absFilename))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_exit(struct tcb *tcp) {
	toPid(pid, tcp->pid);

	event *ev = createEventWithStdParams(EVENT_NAME_EXIT, 1);

	if (addParam(ev, createParam("pid", pid))) {
		return ev;
	}

	return NULL;
}


event *ucSemantics_exit_group(struct tcb *tcp) {
	int count;
	int *tasks;

	// terminate all of the processes' tasks
	if ((tasks = getProcessTasks(tcp->pid, &count))) {
		char pids[count * sizeof(char) * PID_LEN];
		int written = 0;
		while (count-- > 0) {
			written += snprintf(pids + written, sizeof(pids) - written, "%d:", tasks[count]);
		}
		free(tasks);

		event *ev = createEventWithStdParams(EVENT_NAME_EXITGROUP, 1);
		if (addParam(ev, createParam("pids", pids))) {
			return ev;
		}
	}

	return NULL;
}

event *ucSemantics_execve(struct tcb *tcp) {
	char relFilename[FILENAME_MAX];
	char absFilename[FILENAME_MAX];
	char cmdline[FILENAME_MAX];

	cmdline[0]='\0';

	if (is_actual(tcp)) {
		return NULL;
	}

	// Note: Do not ignore filename.

	toString(relFilename, tcp, tcp->u_arg[0]);
	if (!(toAbsFilename(tcp->pid, relFilename, absFilename, sizeof(absFilename), false))) {
		return NULL;
	}


	toPid(pid, tcp->pid);

	int l=getCmdline(cmdline, FILENAME_MAX, tcp->pid);

	//it shouldn't be needed, but just in case	
	cmdline[l]='\0';

	event *ev = createEventWithStdParams(EVENT_NAME_EXECVE, 3);
	
	bool bpid=addParam(ev, createParam("pid", pid));	
	bool bfile=addParam(ev, createParam("filename", absFilename));
	bool bcmd=addParam(ev, createParam("cmdline", cmdline));
	
	if (bpid && bfile && bcmd) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_dup(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);
	toFd(fd2, tcp->u_rval);

	event *ev = createEventWithStdParams(EVENT_NAME_DUP, 3);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("oldfd", fd1))
		&& addParam(ev, createParam("newfd", fd2))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_dup2(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return NULL;
	}

	// dup2() does nothing if oldfd == newfd; dup3() would have failed anyway
	if (tcp->u_arg[0] == tcp->u_rval) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);
	toFd(fd2, tcp->u_rval);

	event *ev = createEventWithStdParams(EVENT_NAME_DUP2, 3);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("oldfd", fd1))
		&& addParam(ev, createParam("newfd", fd2))) {
		return ev;
	}

	return NULL;
}

// define copied from strace:process.c, cf. comment there
#define CLONE_FILES     0x00000400

event *ucSemantics_clone(struct tcb *tcp) {
	char childPid[PID_LEN];
	char parentPid[PID_LEN];
	char flags[256];

	/*
	 * The following else-ifs get the parent and child pids.
	 */
	if (tcp->u_rval < 0) {
		return NULL;
	}
	else if (tcp->u_rval == 0) {
		// the child

		int ppid = getParentPid(tcp->pid);
		if (ppid < 0) {
			return NULL;
		}

		toPid(childPid, tcp->pid);
		toPid(parentPid, ppid);
	}
	else { // tcp->u_rval > 0
		// the parent

		toPid(childPid, tcp->u_rval);
		toPid(parentPid, tcp->pid);
	}


	int written = 0;
	flags[0] = '\0';
	// process flags
	if (tcp->scno == SYS_clone) {
		if (IS_FLAG_SET(tcp->u_arg[0], CLONE_FILES)) {
			written += snprintf(flags + written, sizeof(flags) - written, "%s|", "CLONE_FILES");
		}
	}
	else if (tcp->scno == SYS_fork) {
	}
	else if (tcp->scno == SYS_fork) {
	}

	event *ev = createEventWithStdParams(EVENT_NAME_CLONE, 3);
	if (addParam(ev, createParam("cpid", childPid))
		&& addParam(ev, createParam("ppid", parentPid))
		&& addParam(ev, createParam("flags", flags))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_close(struct tcb *tcp) {
	char filename[FILENAME_MAX];

	if (tcp->u_rval < 0) {
		return NULL;
	}

	getfdpath(tcp, tcp->u_arg[0], filename, sizeof(filename));
	if (ignoreFilename(filename)) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);

	event *ev = createEventWithStdParams(EVENT_NAME_CLOSE, 2);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_connect(struct tcb *tcp) {
	// connect might succeed if return value is -1 and errno == EINPROGRESS.
	if (tcp->u_arg[0] < 0 || (tcp->u_rval < 0 && tcp->u_error != EINPROGRESS)) {
		return NULL;
	}

	char socketname[PATH_MAX + 1];
	int socketInode;
	char localIP[16];
	char remoteIP[16];
	char localPort[6];
	char remotePort[6];

	getfdpath(tcp, tcp->u_arg[0], socketname, sizeof(socketname));
	sscanf(socketname,"socket:[%d]", &socketInode);

	if (!getIPsAndPorts(tcp->pid, socketInode, localIP, localPort, remoteIP, remotePort)) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);

	event *ev = createEventWithStdParams(EVENT_NAME_CONNECT, 7);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("fd", fd1))
		&& addParam(ev, createParam("localIP", localIP))
		&& addParam(ev, createParam("localPort", localPort))
		&& addParam(ev, createParam("remoteIP", remoteIP))
		&& addParam(ev, createParam("remotePort", remotePort))
		&& addParam(ev, createParam("socketname", socketname))) {
		return ev;
	}

	return NULL;
}

event *ucSemantics_accept(struct tcb *tcp) {
	if (tcp->u_rval < 0) {
		return NULL;
	}

	char socketname[PATH_MAX + 1];
	int socketInode;
	char localIP[16];
	char remoteIP[16];
	char localPort[6];
	char remotePort[6];

	getfdpath(tcp, tcp->u_rval, socketname, sizeof(socketname));
	sscanf(socketname,"socket:[%d]", &socketInode);

	if (!getIPsAndPorts(tcp->pid, socketInode, localIP, localPort, remoteIP, remotePort)) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_rval);
	toFd(fd2, tcp->u_arg[0]);

	event *ev = createEventWithStdParams(EVENT_NAME_ACCEPT, 8);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("newfd", fd1))
		&& addParam(ev, createParam("oldfd", fd2))
		&& addParam(ev, createParam("localIP", localIP))
		&& addParam(ev, createParam("localPort", localPort))
		&& addParam(ev, createParam("remoteIP", remoteIP))
		&& addParam(ev, createParam("remotePort", remotePort))
		&& addParam(ev, createParam("socketname", socketname))) {
		return ev;
	}

	return NULL;
}


event *ucSemantics_sendfile(struct tcb *tcp) {
	char filename[FILENAME_MAX];

	// We are interested in desired events only
	if (is_actual(tcp)) {
		return NULL;
	}

	getfdpath(tcp, tcp->u_arg[0], filename, sizeof(filename));
	if (ignoreFilename(filename)) {
		return NULL;
	}

	toPid(pid, tcp->pid);
	toFd(fd1, tcp->u_arg[0]);
	toFd(fd2, tcp->u_arg[1]);

	event *ev = createEventWithStdParams(EVENT_NAME_SENDFILE, 5);
	if (addParam(ev, createParam("pid", pid))
		&& addParam(ev, createParam("outfd", fd1))
		&& addParam(ev, createParam("infd", fd2))
		&& addParam(ev, createParam("outfilename", filename))
		&& addParam(ev, createParam("allowImpliesActual", "true"))) {
		return ev;
	}

	return NULL;
}



event *(*ucSemanticsFunct[])(struct tcb *tcp) = {
#ifdef SYS_accept
	[SYS_accept] = ucSemantics_accept,
#endif
#ifdef SYS_accept4
	[SYS_accept4] = ucSemantics_accept,
#endif
#ifdef SYS_access
	[SYS_access] = ucSemantics_IGNORE,
#endif
#ifdef SYS_acct
	[SYS_acct] = ucSemantics_IGNORE,
#endif
#ifdef SYS_add_key
	[SYS_add_key] = ucSemantics_IGNORE,
#endif
#ifdef SYS_adjtimex
	[SYS_adjtimex] = ucSemantics_IGNORE,
#endif
#ifdef SYS_afs_syscall
	[SYS_afs_syscall] = ucSemantics_IGNORE,
#endif
#ifdef SYS_alarm
	[SYS_alarm] = ucSemantics_IGNORE,
#endif
#ifdef SYS_bdflush
	[SYS_bdflush] = ucSemantics_IGNORE,
#endif
#ifdef SYS_bind
	[SYS_bind] = ucSemantics_IGNORE,
#endif
#ifdef SYS_break
	[SYS_break] = ucSemantics_IGNORE,
#endif
#ifdef SYS_brk
	[SYS_brk] = ucSemantics_IGNORE,
#endif
#ifdef SYS_capget
	[SYS_capget] = ucSemantics_IGNORE,
#endif
#ifdef SYS_capset
	[SYS_capset] = ucSemantics_IGNORE,
#endif
#ifdef SYS_chdir
	[SYS_chdir] = ucSemantics_IGNORE,
#endif
#ifdef SYS_chmod
	[SYS_chmod] = ucSemantics_IGNORE,
#endif
#ifdef SYS_chown32
	[SYS_chown32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_chown
	[SYS_chown] = ucSemantics_IGNORE,
#endif
#ifdef SYS_chroot
	[SYS_chroot] = ucSemantics_IGNORE,
#endif
#ifdef SYS_clock_adjtime
	[SYS_clock_adjtime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_clock_getres
	[SYS_clock_getres] = ucSemantics_IGNORE,
#endif
#ifdef SYS_clock_gettime
	[SYS_clock_gettime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_clock_nanosleep
	[SYS_clock_nanosleep] = ucSemantics_IGNORE,
#endif
#ifdef SYS_clock_settime
	[SYS_clock_settime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_clone
	[SYS_clone] = ucSemantics_clone,
#endif
#ifdef SYS_close
	[SYS_close] = ucSemantics_close,
#endif
#ifdef SYS_connect
	[SYS_connect] = ucSemantics_connect,
#endif
#ifdef SYS_create_module
	[SYS_create_module] = ucSemantics_IGNORE,
#endif
#ifdef SYS_creat
	[SYS_creat] = ucSemantics_creat,
#endif
#ifdef SYS_delete_module
	[SYS_delete_module] = ucSemantics_IGNORE,
#endif
#ifdef SYS_dup2
	[SYS_dup2] = ucSemantics_dup2,
#endif
#ifdef SYS_dup3
	[SYS_dup3] = ucSemantics_dup2,
#endif
#ifdef SYS_dup
	[SYS_dup] = ucSemantics_dup,
#endif
#ifdef SYS_epoll_create1
	[SYS_epoll_create1] = ucSemantics_IGNORE,
#endif
#ifdef SYS_epoll_create
	[SYS_epoll_create] = ucSemantics_IGNORE,
#endif
#ifdef SYS_epoll_ctl
	[SYS_epoll_ctl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_epoll_pwait
	[SYS_epoll_pwait] = ucSemantics_IGNORE,
#endif
#ifdef SYS_epoll_wait
	[SYS_epoll_wait] = ucSemantics_IGNORE,
#endif
#ifdef SYS_eventfd2
	[SYS_eventfd2] = ucSemantics_IGNORE,
#endif
#ifdef SYS_eventfd
	[SYS_eventfd] = ucSemantics_IGNORE,
#endif
#ifdef SYS_execve
	[SYS_execve] = ucSemantics_execve,
#endif
#ifdef SYS_exit_group
	[SYS_exit_group] = ucSemantics_exit_group,
#endif
#ifdef SYS_exit
	[SYS_exit] = ucSemantics_exit,
#endif
#ifdef SYS_faccessat
	[SYS_faccessat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fadvise64_64
	[SYS_fadvise64_64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fadvise64
	[SYS_fadvise64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fallocate
	[SYS_fallocate] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fanotify_init
	[SYS_fanotify_init] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fanotify_mark
	[SYS_fanotify_mark] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fchdir
	[SYS_fchdir] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fchmodat
	[SYS_fchmodat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fchmod
	[SYS_fchmod] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fchown32
	[SYS_fchown32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fchownat
	[SYS_fchownat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fchown
	[SYS_fchown] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fcntl64
	[SYS_fcntl64] = ucSemantics_fcntl,
#endif
#ifdef SYS_fcntl
	[SYS_fcntl] = ucSemantics_fcntl,
#endif
#ifdef SYS_fdatasync
	[SYS_fdatasync] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fgetxattr
	[SYS_fgetxattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_flistxattr
	[SYS_flistxattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_flock
	[SYS_flock] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fork
	[SYS_fork] = ucSemantics_clone,
#endif
#ifdef SYS_fremovexattr
	[SYS_fremovexattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fsetxattr
	[SYS_fsetxattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fstat64
	[SYS_fstat64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fstatat64
	[SYS_fstatat64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fstatfs64
	[SYS_fstatfs64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fstatfs
	[SYS_fstatfs] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fstat
	[SYS_fstat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_fsync
	[SYS_fsync] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ftime
	[SYS_ftime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ftruncate64
	[SYS_ftruncate64] = ucSemantics_ftruncate,
#endif
#ifdef SYS_ftruncate
	[SYS_ftruncate] = ucSemantics_ftruncate,
#endif
#ifdef SYS_futex
	[SYS_futex] = ucSemantics_IGNORE,
#endif
#ifdef SYS_futimesat
	[SYS_futimesat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getcpu
	[SYS_getcpu] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getcwd
	[SYS_getcwd] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getdents64
	[SYS_getdents64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getdents
	[SYS_getdents] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getegid32
	[SYS_getegid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getegid
	[SYS_getegid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_geteuid32
	[SYS_geteuid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_geteuid
	[SYS_geteuid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getgid32
	[SYS_getgid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getgid
	[SYS_getgid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getgroups32
	[SYS_getgroups32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getgroups
	[SYS_getgroups] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getitimer
	[SYS_getitimer] = ucSemantics_IGNORE,
#endif
#ifdef SYS_get_kernel_syms
	[SYS_get_kernel_syms] = ucSemantics_IGNORE,
#endif
#ifdef SYS_get_mempolicy
	[SYS_get_mempolicy] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getpeername
	[SYS_getpeername] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getpgid
	[SYS_getpgid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getpgrp
	[SYS_getpgrp] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getpid
	[SYS_getpid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getpmsg
	[SYS_getpmsg] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getppid
	[SYS_getppid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getpriority
	[SYS_getpriority] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getresgid32
	[SYS_getresgid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getresgid
	[SYS_getresgid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getresuid32
	[SYS_getresuid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getresuid
	[SYS_getresuid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getrlimit
	[SYS_getrlimit] = ucSemantics_IGNORE,
#endif
#ifdef SYS_get_robust_list
	[SYS_get_robust_list] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getrusage
	[SYS_getrusage] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getsid
	[SYS_getsid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getsockname
	[SYS_getsockname] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getsockopt
	[SYS_getsockopt] = ucSemantics_IGNORE,
#endif
#ifdef SYS_get_thread_area
	[SYS_get_thread_area] = ucSemantics_IGNORE,
#endif
#ifdef SYS_gettid
	[SYS_gettid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_gettimeofday
	[SYS_gettimeofday] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getuid32
	[SYS_getuid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getuid
	[SYS_getuid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_getxattr
	[SYS_getxattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_gtty
	[SYS_gtty] = ucSemantics_IGNORE,
#endif
#ifdef SYS_idle
	[SYS_idle] = ucSemantics_IGNORE,
#endif
#ifdef SYS_init_module
	[SYS_init_module] = ucSemantics_IGNORE,
#endif
#ifdef SYS_inotify_add_watch
	[SYS_inotify_add_watch] = ucSemantics_IGNORE,
#endif
#ifdef SYS_inotify_init1
	[SYS_inotify_init1] = ucSemantics_IGNORE,
#endif
#ifdef SYS_inotify_init
	[SYS_inotify_init] = ucSemantics_IGNORE,
#endif
#ifdef SYS_inotify_rm_watch
	[SYS_inotify_rm_watch] = ucSemantics_IGNORE,
#endif
#ifdef SYS_io_cancel
	[SYS_io_cancel] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ioctl
	[SYS_ioctl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_io_destroy
	[SYS_io_destroy] = ucSemantics_IGNORE,
#endif
#ifdef SYS_io_getevents
	[SYS_io_getevents] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ioperm
	[SYS_ioperm] = ucSemantics_IGNORE,
#endif
#ifdef SYS_iopl
	[SYS_iopl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ioprio_get
	[SYS_ioprio_get] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ioprio_set
	[SYS_ioprio_set] = ucSemantics_IGNORE,
#endif
#ifdef SYS_io_setup
	[SYS_io_setup] = ucSemantics_IGNORE,
#endif
#ifdef SYS_io_submit
	[SYS_io_submit] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ipc
	[SYS_ipc] = ucSemantics_IGNORE,
#endif
#ifdef SYS_kexec_load
	[SYS_kexec_load] = ucSemantics_IGNORE,
#endif
#ifdef SYS_keyctl
	[SYS_keyctl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_kill
	[SYS_kill] = ucSemantics_kill,
#endif
#ifdef SYS_lchown32
	[SYS_lchown32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_lchown
	[SYS_lchown] = ucSemantics_IGNORE,
#endif
#ifdef SYS_lgetxattr
	[SYS_lgetxattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_linkat
	[SYS_linkat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_link
	[SYS_link] = ucSemantics_IGNORE,
#endif
#ifdef SYS_listen
	[SYS_listen] = ucSemantics_IGNORE,
#endif
#ifdef SYS_listxattr
	[SYS_listxattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_llistxattr
	[SYS_llistxattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS__llseek
	[SYS__llseek] = ucSemantics_IGNORE,
#endif
#ifdef SYS__llseek
	[SYS__llseek] = ucSemantics_IGNORE,
#endif
#ifdef SYS_lock
	[SYS_lock] = ucSemantics_IGNORE,
#endif
#ifdef SYS_lookup_dcookie
	[SYS_lookup_dcookie] = ucSemantics_IGNORE,
#endif
#ifdef SYS_lremovexattr
	[SYS_lremovexattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_lseek
	[SYS_lseek] = ucSemantics_IGNORE,
#endif
#ifdef SYS_lsetxattr
	[SYS_lsetxattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_lstat64
	[SYS_lstat64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_lstat
	[SYS_lstat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_madvise
	[SYS_madvise] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mbind
	[SYS_mbind] = ucSemantics_IGNORE,
#endif
#ifdef SYS_migrate_pages
	[SYS_migrate_pages] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mincore
	[SYS_mincore] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mkdirat
	[SYS_mkdirat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mkdir
	[SYS_mkdir] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mknodat
	[SYS_mknodat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mknod
	[SYS_mknod] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mlockall
	[SYS_mlockall] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mlock
	[SYS_mlock] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mmap2
	[SYS_mmap2] = ucSemantics_mmap,
#endif
#ifdef SYS_mmap
	[SYS_mmap] = ucSemantics_mmap,
#endif
#ifdef SYS_modify_ldt
	[SYS_modify_ldt] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mount
	[SYS_mount] = ucSemantics_IGNORE,
#endif
#ifdef SYS_move_pages
	[SYS_move_pages] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mprotect
	[SYS_mprotect] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mpx
	[SYS_mpx] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mq_getsetattr
	[SYS_mq_getsetattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mq_notify
	[SYS_mq_notify] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mq_open
	[SYS_mq_open ] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mq_timedreceive
	[SYS_mq_timedreceive] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mq_timedsend
	[SYS_mq_timedsend] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mq_unlink
	[SYS_mq_unlink] = ucSemantics_IGNORE,
#endif
#ifdef SYS_mremap
	[SYS_mremap] = ucSemantics_IGNORE,
#endif
#ifdef SYS_msync
	[SYS_msync] = ucSemantics_IGNORE,
#endif
#ifdef SYS_munlockall
	[SYS_munlockall] = ucSemantics_IGNORE,
#endif
#ifdef SYS_munlock
	[SYS_munlock] = ucSemantics_IGNORE,
#endif
#ifdef SYS_munmap
	[SYS_munmap] = ucSemantics_munmap,
#endif
#ifdef SYS_name_to_handle_at
	[SYS_name_to_handle_at] = ucSemantics_IGNORE,
#endif
#ifdef SYS_nanosleep
	[SYS_nanosleep] = ucSemantics_IGNORE,
#endif
#ifdef SYS__newselect
	[SYS__newselect] = ucSemantics_IGNORE,
#endif
#ifdef SYS_nfsservctl
	[SYS_nfsservctl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_nice
	[SYS_nice] = ucSemantics_IGNORE,
#endif
#ifdef SYS_oldfstat
	[SYS_oldfstat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_oldlstat
	[SYS_oldlstat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_oldolduname
	[SYS_oldolduname] = ucSemantics_IGNORE,
#endif
#ifdef SYS_oldstat
	[SYS_oldstat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_olduname
	[SYS_olduname] = ucSemantics_IGNORE,
#endif
#ifdef SYS_openat
	[SYS_openat] = ucSemantics_openat,
#endif
#ifdef SYS_open_by_handle_at
	[SYS_open_by_handle_at] = ucSemantics_IGNORE,
#endif
#ifdef SYS_open
	[SYS_open] = ucSemantics_open,
#endif
#ifdef SYS_pause
	[SYS_pause] = ucSemantics_IGNORE,
#endif
#ifdef SYS_perf_event_open
	[SYS_perf_event_open] = ucSemantics_IGNORE,
#endif
#ifdef SYS_personality
	[SYS_personality] = ucSemantics_IGNORE,
#endif
#ifdef SYS_pipe2
	[SYS_pipe2] = ucSemantics_pipe,
#endif
#ifdef SYS_pipe
	[SYS_pipe] = ucSemantics_pipe,
#endif
#ifdef SYS_pivot_root
	[SYS_pivot_root] = ucSemantics_IGNORE,
#endif
#ifdef SYS_poll
	[SYS_poll] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ppoll
	[SYS_ppoll] = ucSemantics_IGNORE,
#endif
#ifdef SYS_prctl
	[SYS_prctl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_pread64
	[SYS_pread64] = ucSemantics_read,
#endif
#ifdef SYS_preadv
	[SYS_preadv] = ucSemantics_read,
#endif
#ifdef SYS_prlimit64
	[SYS_prlimit64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_process_vm_readv
	[SYS_process_vm_readv] = ucSemantics_IGNORE,
#endif
#ifdef SYS_process_vm_writev
	[SYS_process_vm_writev] = ucSemantics_IGNORE,
#endif
#ifdef SYS_profil
	[SYS_profil] = ucSemantics_IGNORE,
#endif
#ifdef SYS_prof
	[SYS_prof] = ucSemantics_IGNORE,
#endif
#ifdef SYS_pselect6
	[SYS_pselect6] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ptrace
	[SYS_ptrace] = ucSemantics_IGNORE,
#endif
#ifdef SYS_putpmsg
	[SYS_putpmsg] = ucSemantics_IGNORE,
#endif
#ifdef SYS_pwrite64
	[SYS_pwrite64] = ucSemantics_write,
#endif
#ifdef SYS_pwritev
	[SYS_pwritev] = ucSemantics_write,
#endif
#ifdef SYS_query_module
	[SYS_query_module] = ucSemantics_IGNORE,
#endif
#ifdef SYS_quotactl
	[SYS_quotactl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_readahead
	[SYS_readahead] = ucSemantics_IGNORE,
#endif
#ifdef SYS_readdir
	[SYS_readdir] = ucSemantics_IGNORE,
#endif
#ifdef SYS_readlinkat
	[SYS_readlinkat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_readlink
	[SYS_readlink] = ucSemantics_IGNORE,
#endif
#ifdef SYS_read
	[SYS_read] = ucSemantics_read,
#endif
#ifdef SYS_readv
	[SYS_readv] = ucSemantics_read,
#endif
#ifdef SYS_reboot
	[SYS_reboot] = ucSemantics_IGNORE,
#endif
#ifdef SYS_recvfrom
	[SYS_recvfrom] = ucSemantics_read,
#endif
#ifdef SYS_recvmmsg
	[SYS_recvmmsg] = ucSemantics_read,
#endif
#ifdef SYS_recvmsg
	[SYS_recvmsg] = ucSemantics_read,
#endif
#ifdef SYS_recv
	[SYS_recv] = ucSemantics_read,
#endif
#ifdef SYS_remap_file_pages
	[SYS_remap_file_pages] = ucSemantics_IGNORE,
#endif
#ifdef SYS_removexattr
	[SYS_removexattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_renameat
	[SYS_renameat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rename
	[SYS_rename] = ucSemantics_rename,
#endif
#ifdef SYS_request_key
	[SYS_request_key] = ucSemantics_IGNORE,
#endif
#ifdef SYS_restart_syscall
	[SYS_restart_syscall] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rmdir
	[SYS_rmdir] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rt_sigaction
	[SYS_rt_sigaction] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rt_sigaction
	[SYS_rt_sigaction] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rt_sigpending
	[SYS_rt_sigpending] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rt_sigprocmask
	[SYS_rt_sigprocmask] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rt_sigqueueinfo
	[SYS_rt_sigqueueinfo] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rt_sigreturn
	[SYS_rt_sigreturn] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rt_sigsuspend
	[SYS_rt_sigsuspend] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rt_sigtimedwait
	[SYS_rt_sigtimedwait] = ucSemantics_IGNORE,
#endif
#ifdef SYS_rt_tgsigqueueinfo
	[SYS_rt_tgsigqueueinfo] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_getaffinity
	[SYS_sched_getaffinity] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_getparam
	[SYS_sched_getparam] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_get_priority_max
	[SYS_sched_get_priority_max] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_get_priority_min
	[SYS_sched_get_priority_min] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_getscheduler
	[SYS_sched_getscheduler] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_rr_get_interval
	[SYS_sched_rr_get_interval] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_setaffinity
	[SYS_sched_setaffinity] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_setparam
	[SYS_sched_setparam] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_setscheduler
	[SYS_sched_setscheduler] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sched_yield
	[SYS_sched_yield] = ucSemantics_IGNORE,
#endif
#ifdef SYS_select
	[SYS_select] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sendfile64
	[SYS_sendfile64] = ucSemantics_sendfile,
#endif
#ifdef SYS_sendfile
	[SYS_sendfile] = ucSemantics_sendfile,
#endif
#ifdef SYS_sendmmsg
	[SYS_sendmmsg] = ucSemantics_write,
#endif
#ifdef SYS_sendmsg
	[SYS_sendmsg] = ucSemantics_write,
#endif
#ifdef SYS_send
	[SYS_send] = ucSemantics_write,
#endif
#ifdef SYS_sendto
	[SYS_sendto] = ucSemantics_write,
#endif
#ifdef SYS_setdomainname
	[SYS_setdomainname] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setfsgid32
	[SYS_setfsgid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setfsgid
	[SYS_setfsgid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setfsuid32
	[SYS_setfsuid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setfsuid
	[SYS_setfsuid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setgid32
	[SYS_setgid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setgid
	[SYS_setgid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setgroups32
	[SYS_setgroups32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setgroups
	[SYS_setgroups] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sethostname
	[SYS_sethostname] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setitimer
	[SYS_setitimer] = ucSemantics_IGNORE,
#endif
#ifdef SYS_set_mempolicy
	[SYS_set_mempolicy] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setns
	[SYS_setns] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setpgid
	[SYS_setpgid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setpriority
	[SYS_setpriority] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setregid32
	[SYS_setregid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setregid
	[SYS_setregid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setresgid32
	[SYS_setresgid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setresgid
	[SYS_setresgid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setresuid32
	[SYS_setresuid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setresuid
	[SYS_setresuid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setreuid32
	[SYS_setreuid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setreuid
	[SYS_setreuid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setrlimit
	[SYS_setrlimit] = ucSemantics_IGNORE,
#endif
#ifdef SYS_set_robust_list
	[SYS_set_robust_list] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setsid
	[SYS_setsid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setsockopt
	[SYS_setsockopt] = ucSemantics_IGNORE,
#endif
#ifdef SYS_set_thread_area
	[SYS_set_thread_area] = ucSemantics_IGNORE,
#endif
#ifdef SYS_set_thread_area
	[SYS_set_thread_area] = ucSemantics_IGNORE,
#endif
#ifdef SYS_set_tid_address
	[SYS_set_tid_address] = ucSemantics_IGNORE,
#endif
#ifdef SYS_settimeofday
	[SYS_settimeofday] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setuid32
	[SYS_setuid32] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setuid
	[SYS_setuid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_setxattr
	[SYS_setxattr] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sgetmask
	[SYS_sgetmask] = ucSemantics_IGNORE,
#endif
#ifdef SYS_shutdown
	[SYS_shutdown] = ucSemantics_shutdown,
#endif
#ifdef SYS_sigaction
	[SYS_sigaction] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sigaltstack
	[SYS_sigaltstack] = ucSemantics_IGNORE,
#endif
#ifdef SYS_signalfd4
	[SYS_signalfd4] = ucSemantics_IGNORE,
#endif
#ifdef SYS_signalfd
	[SYS_signalfd] = ucSemantics_IGNORE,
#endif
#ifdef SYS_signal
	[SYS_signal] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sigpending
	[SYS_sigpending] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sigprocmask
	[SYS_sigprocmask] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sigreturn
	[SYS_sigreturn] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sigsuspend
	[SYS_sigsuspend] = ucSemantics_IGNORE,
#endif
#ifdef SYS_socketcall
	[SYS_socketcall] = ucSemantics_IGNORE,
#endif
#ifdef SYS_socket
	[SYS_socket] = ucSemantics_socket,
#endif
#ifdef SYS_socketpair
	[SYS_socketpair] = ucSemantics_socketpair,
#endif
#ifdef SYS_splice
	[SYS_splice] = ucSemantics_splice,
#endif
#ifdef SYS_ssetmask
	[SYS_ssetmask] = ucSemantics_IGNORE,
#endif
#ifdef SYS_stat64
	[SYS_stat64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_statfs64
	[SYS_statfs64] = ucSemantics_IGNORE,
#endif
#ifdef SYS_statfs
	[SYS_statfs] = ucSemantics_IGNORE,
#endif
#ifdef SYS_stat
	[SYS_stat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_stime
	[SYS_stime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_stty
	[SYS_stty] = ucSemantics_IGNORE,
#endif
#ifdef SYS_swapoff
	[SYS_swapoff] = ucSemantics_IGNORE,
#endif
#ifdef SYS_swapon
	[SYS_swapon] = ucSemantics_IGNORE,
#endif
#ifdef SYS_symlinkat
	[SYS_symlinkat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_symlink
	[SYS_symlink] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sync_file_range
	[SYS_sync_file_range] = ucSemantics_IGNORE,
#endif
#ifdef SYS_syncfs
	[SYS_syncfs] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sync
	[SYS_sync] = ucSemantics_IGNORE,
#endif
#ifdef SYS__sysctl
	[SYS__sysctl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sysfs
	[SYS_sysfs] = ucSemantics_IGNORE,
#endif
#ifdef SYS_sysinfo
	[SYS_sysinfo] = ucSemantics_IGNORE,
#endif
#ifdef SYS_syslog
	[SYS_syslog] = ucSemantics_IGNORE,
#endif
#ifdef SYS_tee
	[SYS_tee] = ucSemantics_tee,
#endif
#ifdef SYS_tgkill
	[SYS_tgkill] = ucSemantics_IGNORE,
#endif
#ifdef SYS_timer_create
	[SYS_timer_create] = ucSemantics_IGNORE,
#endif
#ifdef SYS_timer_delete
	[SYS_timer_delete] = ucSemantics_IGNORE,
#endif
#ifdef SYS_timerfd_create
	[SYS_timerfd_create] = ucSemantics_IGNORE,
#endif
#ifdef SYS_timerfd_gettime
	[SYS_timerfd_gettime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_timerfd_settime
	[SYS_timerfd_settime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_timer_getoverrun
	[SYS_timer_getoverrun] = ucSemantics_IGNORE,
#endif
#ifdef SYS_timer_gettime
	[SYS_timer_gettime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_timer_settime
	[SYS_timer_settime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_times
	[SYS_times] = ucSemantics_IGNORE,
#endif
#ifdef SYS_time
	[SYS_time] = ucSemantics_IGNORE,
#endif
#ifdef SYS_tkill
	[SYS_tkill] = ucSemantics_IGNORE,
#endif
#ifdef SYS_truncate64
	[SYS_truncate64] = ucSemantics_truncate,
#endif
#ifdef SYS_truncate
	[SYS_truncate] = ucSemantics_truncate,
#endif
#ifdef SYS_ugetrlimit
	[SYS_ugetrlimit] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ulimit
	[SYS_ulimit] = ucSemantics_IGNORE,
#endif
#ifdef SYS_umask
	[SYS_umask] = ucSemantics_IGNORE,
#endif
#ifdef SYS_umount2
	[SYS_umount2] = ucSemantics_IGNORE,
#endif
#ifdef SYS_umount
	[SYS_umount] = ucSemantics_IGNORE,
#endif
#ifdef SYS_uname
	[SYS_uname] = ucSemantics_IGNORE,
#endif
#ifdef SYS_unlinkat
	[SYS_unlinkat] = ucSemantics_unlinkat,
#endif
#ifdef SYS_unlink
	[SYS_unlink] = ucSemantics_unlink,
#endif
#ifdef SYS_unshare
	[SYS_unshare] = ucSemantics_IGNORE,
#endif
#ifdef SYS_uselib
	[SYS_uselib] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ustat
	[SYS_ustat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_utimensat
	[SYS_utimensat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_utimes
	[SYS_utimes] = ucSemantics_IGNORE,
#endif
#ifdef SYS_utime
	[SYS_utime] = ucSemantics_IGNORE,
#endif
#ifdef SYS_vfork
	[SYS_vfork] = ucSemantics_clone,
#endif
#ifdef SYS_vhangup
	[SYS_vhangup] = ucSemantics_IGNORE,
#endif
#ifdef SYS_vm86old
	[SYS_vm86old] = ucSemantics_IGNORE,
#endif
#ifdef SYS_vm86
	[SYS_vm86] = ucSemantics_IGNORE,
#endif
#ifdef SYS_vmsplice
	[SYS_vmsplice] = ucSemantics_IGNORE,
#endif
#ifdef SYS_vserver
	[SYS_vserver] = ucSemantics_IGNORE,
#endif
#ifdef SYS_wait4
	[SYS_wait4] = ucSemantics_IGNORE,
#endif
#ifdef SYS_waitid
	[SYS_waitid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_waitpid
	[SYS_waitpid] = ucSemantics_IGNORE,
#endif
#ifdef SYS_write
	[SYS_write] = ucSemantics_write,
#endif
#ifdef SYS_writev
	[SYS_writev] = ucSemantics_write,
#endif
#ifdef SYS_semop
	[SYS_semop] = ucSemantics_IGNORE,
#endif
#ifdef SYS_semget
	[SYS_semget] = ucSemantics_IGNORE,
#endif
#ifdef SYS_semctl
	[SYS_semctl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_semtimedop
	[SYS_semtimedop] = ucSemantics_IGNORE,
#endif
#ifdef SYS_ipc_subcall
	[SYS_ipc_subcall] = ucSemantics_IGNORE,
#endif
#ifdef SYS_msgsnd
	[SYS_msgsnd] = ucSemantics_IGNORE,
#endif
#ifdef SYS_msgrcv
	[SYS_msgrcv] = ucSemantics_IGNORE,
#endif
#ifdef SYS_msgget
	[SYS_msgget] = ucSemantics_IGNORE,
#endif
#ifdef SYS_msgctl
	[SYS_msgctl] = ucSemantics_IGNORE,
#endif
#ifdef SYS_shmat
	[SYS_shmat] = ucSemantics_IGNORE,
#endif
#ifdef SYS_shmdt
	[SYS_shmdt] = ucSemantics_IGNORE,
#endif
#ifdef SYS_shmget
	[SYS_shmget] = ucSemantics_IGNORE,
#endif
#ifdef SYS_shmctl
	[SYS_shmctl] = ucSemantics_IGNORE,
#endif
};
