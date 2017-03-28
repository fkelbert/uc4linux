#include "ucTypes.h"

#if UC_JNI
void ucTypesSetJniEnv(JNIEnv *mainJniEnv) {
	jniEnv = mainJniEnv;
}
#endif

char str_pid[PID_LEN];
char str_fd[FD_LEN];
char str_filename[FILENAME_MAX];
char str_localip[16];
char str_localport[6];
char str_newfd[FD_LEN];
char str_oldfd[FD_LEN];
char str_remoteip[16];
char str_remoteport[6];
char str_socketname[PATH_MAX + 1];

param *createParam(char *key, char *val) {
	param *p = (param *) malloc(sizeof(param));
	p->key = createString(key);
	p->val = createString(val);
	return p;
}

void destroyParam(param *p) {
	if (p == paramFd || p == paramPid || p == paramFilename || p == paramAllowImpliesActual || p ==paramOldfd || p == paramNewfd
			|| p == paramLocalip || p == paramLocalport || p == paramRemoteip || p == paramRemoteport || p == paramSocketname
			) {
		return;
	}

	destroyString(p->key);
	destroyString(p->val);
	free(p);
}

inline bool addParam(event *ev, param *p) {
	if (ev->iterParams < MAX_PARAMS) {
		ev->params[ev->iterParams] = p;
		ev->iterParams++;
		return true;
	}
	return false;
}

inline bool addParamAllowImpliesActual() {
	return addParam(theEvent, paramAllowImpliesActual);
}

inline bool addParamFd(int fd) {
	toFd(str_fd, fd);
	return addParam(theEvent, paramFd);
}

inline bool addParamPid(int pid) {
	toPid(str_pid, pid);
	return addParam(theEvent, paramPid);
}

inline bool addParamFilename(char *filename) {
	snprintf(str_filename, FILENAME_MAX, "%s", filename);
	return addParam(theEvent, paramFilename);
}

inline bool addParamLocalip(char *ip) {
	snprintf(str_localip, 16, "%s", ip);
	return addParam(theEvent, paramLocalip);
}

inline bool addParamLocalport(char *port) {
	snprintf(str_localport, 6, "%s", port);
	return addParam(theEvent, paramLocalport);
}

inline bool addParamNewFd(int fd) {
	toFd(str_newfd, fd);
	return addParam(theEvent, paramNewfd);
}

inline bool addParamOldFd(int fd) {
	toFd(str_oldfd, fd);
	return addParam(theEvent, paramOldfd);
}

inline bool addParamRemoteip(char *ip) {
	snprintf(str_remoteip, 16, "%s", ip);
	return addParam(theEvent, paramRemoteip);
}

inline bool addParamRemoteport(char *port) {
	snprintf(str_remoteport, 6, "%s", port);
	return addParam(theEvent, paramRemoteport);
}

inline bool addParamSocketname(char *sname) {
	snprintf(str_socketname, PATH_MAX + 1, "%s", sname);
	return addParam(theEvent, paramSocketname);
}

inline event *createEventWithStdParams(str name, int cntParams) {
	theEvent->name = name;
	theEvent->isActual = true;
	theEvent->cntParams = cntParams + 2;
	theEvent->iterParams = 2;
	return theEvent;
}

inline void destroyEvent(event *e) {
	int i;
	for (i = 2; i < e->cntParams; i++) {
		destroyParam(e->params[i]);
	}
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

	paramAllowImpliesActual = createParam("allowImpliesActual", "true");

	paramFd = createParam("fd", "");
	paramFd->val = str_fd;

	paramFilename = createParam("filename", "");
	paramFilename->val = str_filename;

	paramLocalip = createParam("localIP", "");
	paramLocalip->val = str_localip;

	paramLocalport = createParam("localPort", "");
	paramLocalport->val = str_localport;

	paramNewfd = createParam("newfd", "");
	paramNewfd->val = str_newfd;

	paramOldfd = createParam("oldfd", "");
	paramOldfd->val = str_oldfd;

	paramPid = createParam("pid", "");
	paramPid->val = str_pid;

	paramRemoteip = createParam("remoteIP", "");
	paramRemoteip->val = str_remoteip;

	paramRemoteport = createParam("remotePort", "");
	paramRemoteport->val = str_remoteport;

	paramSocketname = createParam("socketname", "");
	paramSocketname->val = str_socketname;


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

