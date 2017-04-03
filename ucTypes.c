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

param *createParam(char *key) {
	param *p = (param *) malloc(sizeof(param));
	p->key = strdup(key);
	return p;
}

inline bool addParam(param *p) {
	if (theEvent->iterParams < MAX_PARAMS) {
		theEvent->params[theEvent->iterParams] = p;
		theEvent->iterParams++;
		return true;
	}
	return false;
}

inline bool addParamAddr(char *addr) {
	snprintf(str_addr, 12, "%s", addr);
	return addParam(paramAddr);
}

inline bool addParamAllowImpliesActual() {
	return addParam(paramAllowImpliesActual);
}

inline bool addParamCmdline(char *cmdline) {
	snprintf(str_cmdline, FILENAME_MAX, "%s", cmdline);
	return addParam(paramCmdline);
}

inline bool addParamCpid(int pid) {
	toPid(str_cpid, pid);
	return addParam(paramCpid);
}

inline bool addParamCwd(char *cwd) {
	snprintf(str_cwd, FILENAME_MAX, "%s", cwd);
	return addParam(paramCwd);
}

inline bool addParamDir(char *dir) {
	snprintf(str_dir, FILENAME_MAX, "%s", dir);
	return addParam(paramDir);
}

inline bool addParamDomain(char *domain) {
	snprintf(str_domain, 128, "%s", domain);
	return addParam(paramDomain);
}

inline bool addParamDstfd(int fd) {
	toFd(str_dstfd, fd);
	return addParam(paramDstfd);
}

inline bool addParamDstfilename(char *filename) {
	snprintf(str_dstfilename, FILENAME_MAX, "%s", filename);
	return addParam(paramDstfilename);
}

inline bool addParamDstpid(int pid) {
	toPid(str_dstpid, pid);
	return addParam(paramDstpid);
}

inline bool addParamFd(int fd) {
	toFd(str_fd, fd);
	return addParam(paramFd);
}

inline bool addParamFd1(int fd) {
	toFd(str_fd1, fd);
	return addParam(paramFd1);
}

inline bool addParamFd2(int fd) {
	toFd(str_fd2, fd);
	return addParam(paramFd2);
}

inline bool addParamFilename(char *filename) {
	snprintf(str_filename, FILENAME_MAX, "%s", filename);
	return addParam(paramFilename);
}

inline bool addParamFlags(char *flags) {
	snprintf(str_flags, 256, "%s", flags);
	return addParam(paramFlags);
}

inline bool addParamHow(char *how) {
	snprintf(str_how, 128, "%s", how);
	return addParam(paramHow);
}

inline bool addParamInfd(int fd) {
	toFd(str_infd, fd);
	return addParam(paramInfd);
}

inline bool addParamLocalip(unsigned long ip) {
	snprintf(str_localip, 16, "%d.%d.%d.%d", (unsigned char) ip & 0xFF, (unsigned char) (ip >> 8) & 0xFF, (unsigned char) (ip >> 16) & 0xFF, (unsigned char) (ip >> 24) & 0xFF);
	return addParam(paramLocalip);
}

inline bool addParamLocalport(unsigned long port) {
	snprintf(str_localport, 6, "%ld", port);
	return addParam(paramLocalport);
}

inline bool addParamNew(char *filename) {
	snprintf(str_new, FILENAME_MAX, "%s", filename);
	return addParam(paramNew);
}

inline bool addParamNewfd(int fd) {
	toFd(str_newfd, fd);
	return addParam(paramNewfd);
}

inline bool addParamOld(char *filename) {
	snprintf(str_old, FILENAME_MAX, "%s", filename);
	return addParam(paramOld);
}

inline bool addParamOldfd(int fd) {
	toFd(str_oldfd, fd);
	return addParam(paramOldfd);
}

inline bool addParamOperation(char *operation) {
	snprintf(str_operation, 128, "%s", operation);
	return addParam(paramOperation);
}

inline bool addParamOutfd(int fd) {
	toFd(str_outfd, fd);
	return addParam(paramOutfd);
}

inline bool addParamOutfilename(char *filename) {
	snprintf(str_outfilename, FILENAME_MAX, "%s", filename);
	return addParam(paramOutfilename);
}

inline bool addParamPid(int pid) {
	toPid(str_pid, pid);
	return addParam(paramPid);
}

inline bool addParamPids(char *pids) {
	snprintf(str_pids, 4096, "%s", pids);
	return addParam(paramPids);
}

inline bool addParamPipename(char *pipename) {
	snprintf(str_pipename, FILENAME_MAX, "%s", pipename);
	return addParam(paramPipename);
}

inline bool addParamPpid(int pid) {
	toPid(str_ppid, pid);
	return addParam(paramPpid);
}

inline bool addParamRemoteip(unsigned long ip) {
	snprintf(str_remoteip, 16, "%d.%d.%d.%d", (unsigned char) ip & 0xFF, (unsigned char) (ip >> 8) & 0xFF, (unsigned char) (ip >> 16) & 0xFF, (unsigned char) (ip >> 24) & 0xFF);
	return addParam(paramRemoteip);
}

inline bool addParamRemoteport(unsigned long port) {
	snprintf(str_remoteport, 6, "%ld", port);
	return addParam(paramRemoteport);
}

inline bool addParamSocketname(char *sname) {
	snprintf(str_socketname, PATH_MAX + 1, "%s", sname);
	return addParam(paramSocketname);
}

inline bool addParamSocketname1(char *sname) {
	snprintf(str_socketname1, PATH_MAX + 1, "%s", sname);
	return addParam(paramSocketname1);
}

inline bool addParamSocketname2(char *sname) {
	snprintf(str_socketname2, PATH_MAX + 1, "%s", sname);
	return addParam(paramSocketname2);
}

inline bool addParamSrcfd(int fd) {
	toFd(str_srcfd, fd);
	return addParam(paramSrcfd);
}

inline bool addParamSrcpid(int pid) {
	toPid(str_srcpid, pid);
	return addParam(paramSrcpid);
}

inline bool addParamTrunc(int trunc) {
	snprintf(str_trunc, 6, "%s", trunc ? "true" : "false");
	return addParam(paramTrunc);
}

inline bool addParamType(char *type) {
	snprintf(str_type, 128, "%s", type);
	return addParam(paramType);
}

inline event *createEventWithStdParams(char *name, int cntParams) {
	snprintf(theEvent->name, 64, "%s", name);
	theEvent->isActual = true;
	theEvent->iterParams = 2;
	theEvent->cntParams = cntParams + theEvent->iterParams;
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

	paramPep = createParam("PEP");
	paramPep->val = strdup("Linux");
	addParam(paramPep);

	paramHost = createParam("host");
	paramHost->val = strdup(utsname.nodename);
	addParam(paramHost);

	paramAddr = createParam("addr");
	paramAddr->val = str_addr;

	paramAllowImpliesActual = createParam("allowImpliesActual");
	paramAllowImpliesActual->val = strdup("true");

	paramCmdline = createParam("cmdline");
	paramCmdline->val = str_cmdline;

	paramCpid = createParam("cpid");
	paramCpid->val = str_cpid;

	paramCwd = createParam("cwd");
	paramCwd->val = str_cwd;

	paramDir = createParam("dir");
	paramDir->val = str_dir;

	paramDomain = createParam("domain");
	paramDomain->val = str_domain;

	paramDstfd = createParam("dstfd");
	paramDstfd->val = str_dstfd;

	paramDstfilename = createParam("dstfilename");
	paramDstfilename->val = str_dstfilename;

	paramDstpid = createParam("dstpid");
	paramDstpid->val = str_dstpid;

	paramFd = createParam("fd");
	paramFd->val = str_fd;

	paramFd1 = createParam("fd1");
	paramFd1->val = str_fd1;

	paramFd2 = createParam("fd2");
	paramFd2->val = str_fd2;

	paramFilename = createParam("filename");
	paramFilename->val = str_filename;

	paramFlags = createParam("flags");
	paramFlags->val = str_flags;

	paramHow = createParam("how");
	paramHow->val = str_how;

	paramInfd = createParam("infd");
	paramInfd->val = str_infd;

	paramLocalip = createParam("localIP");
	paramLocalip->val = str_localip;

	paramLocalport = createParam("localPort");
	paramLocalport->val = str_localport;

	paramNew = createParam("new");
	paramNew->val = str_new;

	paramNewfd = createParam("newfd");
	paramNewfd->val = str_newfd;

	paramOld = createParam("old");
	paramOld->val = str_old;

	paramOldfd = createParam("oldfd");
	paramOldfd->val = str_oldfd;

	paramOperation = createParam("operation");
	paramOperation->val = str_operation;

	paramOutfd = createParam("outfd");
	paramOutfd->val = str_outfd;

	paramOutfilename = createParam("outfilename");
	paramOutfilename->val = str_outfilename;

	paramPid = createParam("pid");
	paramPid->val = str_pid;

	paramPids = createParam("pids");
	paramPids->val = str_pids;

	paramPipename = createParam("pipename");
	paramPipename->val = str_pipename;

	paramPpid = createParam("ppid");
	paramPpid->val = str_ppid;

	paramRemoteip = createParam("remoteIP");
	paramRemoteip->val = str_remoteip;

	paramRemoteport = createParam("remotePort");
	paramRemoteport->val = str_remoteport;

	paramSocketname = createParam("socketname");
	paramSocketname->val = str_socketname;

	paramSocketname1 = createParam("socketname1");
	paramSocketname1->val = str_socketname1;

	paramSocketname2 = createParam("socketname2");
	paramSocketname2->val = str_socketname2;

	paramSrcfd = createParam("srcfd");
	paramSrcfd->val = str_srcfd;

	paramSrcpid = createParam("srcpid");
	paramSrcpid->val = str_srcpid;

	paramTrunc = createParam("trunc");
	paramTrunc->val = str_trunc;

	paramType = createParam("type");
	paramType->val = str_type;
}

