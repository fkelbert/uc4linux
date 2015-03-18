#include "ucTypes.h"

#define EVENT_STD_PARAMS_CNT 2
char *eventStdParams[] =
				{ "PEP", "Linux",
					"host", "<<PLACEHOLDER>> (To be filled by function ucTypesInit())" };

#if UC_JNI
JNIEnv *jniEnv;

void ucTypesSetJniEnv(JNIEnv *mainJniEnv) {
	jniEnv = mainJniEnv;
}
#endif

inline str createString(char *s) {
#if UC_JNI
	return JniNewStringUTF(jniEnv, s);
#elif UC_THRIFT
	return strdup(s);
#else
	printf("Unknown option.\n");
	return "";
#endif
}

inline void destroyString(str s) {
#if UC_JNI
	return;
#elif UC_THRIFT
	free(s);
#else
	printf("Unknown option.\n");
#endif
}

void ucTypesInit() {
	// get the hostname using uname()
	struct utsname utsname;
	if (uname(&utsname) == -1) {
		perror("uname failed.");
		exit(1);
	}

	eventStdParams[3] = strdup(utsname.nodename);
	uc_log("Determined hostname: %s\n", eventStdParams[3]);

	EVENT_NAME_ACCEPT = createString("Accept");
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
	param *p = malloc(sizeof(param));
	p->key = createString(key);
	p->val = createString(val);
	return p;
}

inline void destroyParam(param *p) {
	destroyString(p->key);
	destroyString(p->val);
	free(p);
}

inline event *createEvent(str name, int cntParams) {
	event *e = malloc(sizeof(event));
	e->name = name;
	e->isActual = true;
	e->cntParams = cntParams;
	e->iterParams = 0;
	e->params = malloc(cntParams * sizeof(param*));
	return e;
}



inline event *createEventWithStdParams(str name, int cntParams) {
	event *e = createEvent(name, cntParams + EVENT_STD_PARAMS_CNT);

	int i;
	for (i = 0; i < EVENT_STD_PARAMS_CNT; i++) {
		addParam(e, createParam(eventStdParams[i*2], eventStdParams[i*2+1]));
	}

	return e;
}

inline bool addParam(event *ev, param *p) {
	if (ev->iterParams < ev->cntParams) {
		ev->params[ev->iterParams] = p;
		ev->iterParams++;
		return true;
	}
	return false;
}

inline void destroyEvent(event *e) {
	int i;
	for (i = 0; i < e->cntParams; i++) {
		destroyParam(e->params[i]);
	}
	free(e->params);
}

