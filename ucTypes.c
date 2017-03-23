#include "ucTypes.h"


#if UC_JNI
void ucTypesSetJniEnv(JNIEnv *mainJniEnv) {
	jniEnv = mainJniEnv;
}
#endif

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

inline param *createParam(char *key, char *val) {
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

inline void destroyParam(param *p) {
	destroyString(p->key);
	destroyString(p->val);
	free(p);
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
