#include "ucTypes.h"

#if UC_JNI
void ucTypesSetJniEnv(JNIEnv *mainJniEnv) {
	jniEnv = mainJniEnv;
}
#endif

char str_addr[12];
char str_cmdline[FILENAME_MAX];
char str_cpid[PID_LEN];
char str_cwd[FILENAME_MAX];
char str_dir[FILENAME_MAX];
char str_domain[128];
char str_dstfd[FD_LEN];
char str_dstfilename[FILENAME_MAX];
char str_dstpid[PID_LEN];
char str_fd[FD_LEN];
char str_fd1[FD_LEN];
char str_fd2[FD_LEN];
char str_filename[FILENAME_MAX];
char str_flags[256];
char str_how[128];
char str_infd[FD_LEN];
char str_localip[16];
char str_localport[6];
char str_new[FILENAME_MAX];
char str_newfd[FD_LEN];
char str_old[FILENAME_MAX];
char str_oldfd[FD_LEN];
char str_operation[128];
char str_outfd[FD_LEN];
char str_outfilename[FILENAME_MAX];
char str_pid[PID_LEN];
char str_pids[4096];
char str_pipename[FILENAME_MAX];
char str_ppid[PID_LEN];
char str_remoteip[16];
char str_remoteport[6];
char str_socketname[PATH_MAX + 1];
char str_socketname1[PATH_MAX + 1];
char str_socketname2[PATH_MAX + 1];
char str_srcfd[FD_LEN];
char str_srcpid[PID_LEN];
char str_trunc[6];
char str_type[128];

param *createParam(char *key, char *val) {
	param *p = (param *) malloc(sizeof(param));
	p->key = createString(key);
	p->val = createString(val);
	return p;
}

inline bool addParam(event *ev, param *p) {
	if (ev->iterParams < MAX_PARAMS) {
		ev->params[ev->iterParams] = p;
		ev->iterParams++;
		return true;
	}
	return false;
}

inline bool addParamAddr(char *addr) {
	snprintf(str_addr, 12, "%s", addr);
	return addParam(theEvent, paramAddr);
}

inline bool addParamAllowImpliesActual() {
	return addParam(theEvent, paramAllowImpliesActual);
}

inline bool addParamCmdline(char *cmdline) {
	snprintf(str_cmdline, FILENAME_MAX, "%s", cmdline);
	return addParam(theEvent, paramCmdline);
}

inline bool addParamCpid(int pid) {
	toPid(str_cpid, pid);
	return addParam(theEvent, paramCpid);
}

inline bool addParamCwd(char *cwd) {
	snprintf(str_cwd, FILENAME_MAX, "%s", cwd);
	return addParam(theEvent, paramCwd);
}

inline bool addParamDir(char *dir) {
	snprintf(str_dir, FILENAME_MAX, "%s", dir);
	return addParam(theEvent, paramDir);
}

inline bool addParamDomain(char *domain) {
	snprintf(str_domain, 128, "%s", domain);
	return addParam(theEvent, paramDomain);
}

inline bool addParamDstfd(int fd) {
	toFd(str_dstfd, fd);
	return addParam(theEvent, paramDstfd);
}

inline bool addParamDstfilename(char *filename) {
	snprintf(str_dstfilename, FILENAME_MAX, "%s", filename);
	return addParam(theEvent, paramDstfilename);
}

inline bool addParamDstpid(int pid) {
	toPid(str_dstpid, pid);
	return addParam(theEvent, paramDstpid);
}

inline bool addParamFd(int fd) {
	toFd(str_fd, fd);
	return addParam(theEvent, paramFd);
}

inline bool addParamFd1(int fd) {
	toFd(str_fd1, fd);
	return addParam(theEvent, paramFd1);
}

inline bool addParamFd2(int fd) {
	toFd(str_fd2, fd);
	return addParam(theEvent, paramFd2);
}

inline bool addParamFilename(char *filename) {
	snprintf(str_filename, FILENAME_MAX, "%s", filename);
	return addParam(theEvent, paramFilename);
}

inline bool addParamFlags(char *flags) {
	snprintf(str_flags, 256, "%s", flags);
	return addParam(theEvent, paramFlags);
}

inline bool addParamHow(char *how) {
	snprintf(str_how, 128, "%s", how);
	return addParam(theEvent, paramHow);
}

inline bool addParamInfd(int fd) {
	toFd(str_infd, fd);
	return addParam(theEvent, paramInfd);
}

inline bool addParamLocalip(unsigned long ip) {
	snprintf(str_localip, 16, "%d.%d.%d.%d", (unsigned char) ip & 0xFF, (unsigned char) (ip >> 8) & 0xFF, (unsigned char) (ip >> 16) & 0xFF, (unsigned char) (ip >> 24) & 0xFF);
	return addParam(theEvent, paramLocalip);
}

inline bool addParamLocalport(unsigned long port) {
	snprintf(str_localport, 6, "%ld", port);
	return addParam(theEvent, paramLocalport);
}

inline bool addParamNew(char *filename) {
	snprintf(str_new, FILENAME_MAX, "%s", filename);
	return addParam(theEvent, paramNew);
}

inline bool addParamNewfd(int fd) {
	toFd(str_newfd, fd);
	return addParam(theEvent, paramNewfd);
}

inline bool addParamOld(char *filename) {
	snprintf(str_old, FILENAME_MAX, "%s", filename);
	return addParam(theEvent, paramOld);
}

inline bool addParamOldfd(int fd) {
	toFd(str_oldfd, fd);
	return addParam(theEvent, paramOldfd);
}

inline bool addParamOperation(char *operation) {
	snprintf(str_operation, 128, "%s", operation);
	return addParam(theEvent, paramOperation);
}

inline bool addParamOutfd(int fd) {
	toFd(str_outfd, fd);
	return addParam(theEvent, paramOutfd);
}

inline bool addParamOutfilename(char *filename) {
	snprintf(str_outfilename, FILENAME_MAX, "%s", filename);
	return addParam(theEvent, paramOutfilename);
}

inline bool addParamPid(int pid) {
	toPid(str_pid, pid);
	return addParam(theEvent, paramPid);
}

inline bool addParamPids(char *pids) {
	snprintf(str_pids, 4096, "%s", pids);
	return addParam(theEvent, paramPids);
}

inline bool addParamPipename(char *pipename) {
	snprintf(str_pipename, FILENAME_MAX, "%s", pipename);
	return addParam(theEvent, paramPipename);
}

inline bool addParamPpid(int pid) {
	toPid(str_ppid, pid);
	return addParam(theEvent, paramPpid);
}

inline bool addParamRemoteip(unsigned long ip) {
	snprintf(str_remoteip, 16, "%d.%d.%d.%d", (unsigned char) ip & 0xFF, (unsigned char) (ip >> 8) & 0xFF, (unsigned char) (ip >> 16) & 0xFF, (unsigned char) (ip >> 24) & 0xFF);
	return addParam(theEvent, paramRemoteip);
}

inline bool addParamRemoteport(unsigned long port) {
	snprintf(str_remoteport, 6, "%ld", port);
	return addParam(theEvent, paramRemoteport);
}

inline bool addParamSocketname(char *sname) {
	snprintf(str_socketname, PATH_MAX + 1, "%s", sname);
	return addParam(theEvent, paramSocketname);
}

inline bool addParamSocketname1(char *sname) {
	snprintf(str_socketname1, PATH_MAX + 1, "%s", sname);
	return addParam(theEvent, paramSocketname1);
}

inline bool addParamSocketname2(char *sname) {
	snprintf(str_socketname2, PATH_MAX + 1, "%s", sname);
	return addParam(theEvent, paramSocketname2);
}

inline bool addParamSrcfd(int fd) {
	toFd(str_srcfd, fd);
	return addParam(theEvent, paramSrcfd);
}

inline bool addParamSrcpid(int pid) {
	toPid(str_srcpid, pid);
	return addParam(theEvent, paramSrcpid);
}

inline bool addParamTrunc(int trunc) {
	snprintf(str_trunc, 6, "%s", trunc ? "true" : "false");
	return addParam(theEvent, paramTrunc);
}

inline bool addParamType(char *type) {
	snprintf(str_type, 128, "%s", type);
	return addParam(theEvent, paramType);
}

inline event *createEventWithStdParams(str name, int cntParams) {
	theEvent->name = name;
	theEvent->isActual = true;
	theEvent->cntParams = cntParams + 2;
	theEvent->iterParams = 2;
	return theEvent;
}

void ucTypesInit() {
	// get the hostname using uname()
	struct utsname utsname;
	if (uname(&utsname) == -1) {
		perror("uname failed.");
		exit(1);
	}

	uc_log("Determined hostname: %s\n", utsname.nodename);

	theEvent = (event *) malloc(sizeof(event));
	theEvent->params = (param**) malloc(MAX_PARAMS * sizeof(param*));
	theEvent->iterParams = 0;

	addParam(theEvent, createParam("PEP", "Linux"));
	addParam(theEvent, createParam("host", strdup(utsname.nodename)));

	paramAddr = createParam("addr", "");
	paramAddr->val = str_addr;

	paramAllowImpliesActual = createParam("allowImpliesActual", "true");

	paramCmdline = createParam("cmdline", "");
	paramCmdline->val = str_cmdline;

	paramCpid = createParam("cpid", "");
	paramCpid->val = str_cpid;

	paramCwd = createParam("cwd", "");
	paramCwd->val = str_cwd;

	paramDir = createParam("dir", "");
	paramDir->val = str_dir;

	paramDomain = createParam("domain", "");
	paramDomain->val = str_domain;

	paramDstfd = createParam("dstfd", "");
	paramDstfd->val = str_dstfd;

	paramDstfilename = createParam("dstfilename", "");
	paramDstfilename->val = str_dstfilename;

	paramDstpid = createParam("dstpid", "");
	paramDstpid->val = str_dstpid;

	paramFd = createParam("fd", "");
	paramFd->val = str_fd;

	paramFd1 = createParam("fd1", "");
	paramFd1->val = str_fd1;

	paramFd2 = createParam("fd2", "");
	paramFd2->val = str_fd2;

	paramFilename = createParam("filename", "");
	paramFilename->val = str_filename;

	paramFlags = createParam("flags", "");
	paramFlags->val = str_flags;

	paramHow = createParam("how", "");
	paramHow->val = str_how;

	paramInfd = createParam("infd", "");
	paramInfd->val = str_infd;

	paramLocalip = createParam("localIP", "");
	paramLocalip->val = str_localip;

	paramLocalport = createParam("localPort", "");
	paramLocalport->val = str_localport;

	paramNew = createParam("new", "");
	paramNew->val = str_new;

	paramNewfd = createParam("newfd", "");
	paramNewfd->val = str_newfd;

	paramOld = createParam("old", "");
	paramOld->val = str_old;

	paramOldfd = createParam("oldfd", "");
	paramOldfd->val = str_oldfd;

	paramOperation = createParam("operation", "");
	paramOperation->val = str_operation;

	paramOutfd = createParam("outfd", "");
	paramOutfd->val = str_outfd;

	paramOutfilename = createParam("outfilename", "");
	paramOutfilename->val = str_outfilename;

	paramPid = createParam("pid", "");
	paramPid->val = str_pid;

	paramPids = createParam("pids", "");
	paramPids->val = str_pids;

	paramPipename = createParam("pipename", "");
	paramPipename->val = str_pipename;

	paramPpid = createParam("ppid", "");
	paramPpid->val = str_ppid;

	paramRemoteip = createParam("remoteIP", "");
	paramRemoteip->val = str_remoteip;

	paramRemoteport = createParam("remotePort", "");
	paramRemoteport->val = str_remoteport;

	paramSocketname = createParam("socketname", "");
	paramSocketname->val = str_socketname;

	paramSocketname1 = createParam("socketname1", "");
	paramSocketname1->val = str_socketname1;

	paramSocketname2 = createParam("socketname2", "");
	paramSocketname2->val = str_socketname2;

	paramSrcfd = createParam("srcfd", "");
	paramSrcfd->val = str_srcfd;

	paramSrcpid = createParam("srcpid", "");
	paramSrcpid->val = str_srcpid;

	paramTrunc = createParam("trunc", "");
	paramTrunc->val = str_trunc;

	paramType = createParam("type", "");
	paramType->val = str_type;


	EVENT_NAME_ACCEPT = createString("Accept");
	EVENT_NAME_CHROOT = createString("Chroot");
	EVENT_NAME_CLONE = createString("Clone");
	EVENT_NAME_CLOSE = createString("Close");
	EVENT_NAME_CONNECT = createString("Connect");
	EVENT_NAME_CREAT = createString("Creat");
	EVENT_NAME_DUP = createString("Dup");
	EVENT_NAME_DUP2 = createString("Dup2");
	EVENT_NAME_EXECVE = createString("Execve");
	EVENT_NAME_EXIT = createString("Exit");
	EVENT_NAME_EXITGROUP = createString("ExitGroup");
	EVENT_NAME_FCNTL = createString("Fcntl");
	EVENT_NAME_FTRUNCATE = createString("Ftruncate");
	EVENT_NAME_KILL = createString("Kill");
	EVENT_NAME_MMAP = createString("Mmap");
	EVENT_NAME_MUNMAP = createString("Munmap");
	EVENT_NAME_OPEN = createString("Open");
	EVENT_NAME_PIPE = createString("Pipe");
	EVENT_NAME_READ = createString("Read");
	EVENT_NAME_RENAME = createString("Rename");
	EVENT_NAME_SENDFILE = createString("Sendfile");
	EVENT_NAME_SHUTDOWN = createString("Shutdown");
	EVENT_NAME_SOCKET = createString("Socket");
	EVENT_NAME_SOCKETPAIR = createString("Socketpair");
	EVENT_NAME_SPLICE = createString("Splice");
	EVENT_NAME_TEE = createString("Tee");
	EVENT_NAME_TRUNCATE = createString("Truncate");
	EVENT_NAME_UNLINK = createString("Unlink");
	EVENT_NAME_WRITE = createString("Write");
}

