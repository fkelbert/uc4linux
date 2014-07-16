#include "ucTypes.h"

#define EVENT_STD_PARAMS_CNT 2
char *eventStdParams[] =
				{ "PEP", "Linux",
					"host", "PLACEHOLDER" };

#define JniNewStringUTF(env,str)	(*env)->NewStringUTF(env, str)

JNIEnv *jniEnv;

void ucTypesInit(JNIEnv *mainJniEnv) {
	jniEnv = mainJniEnv;

	// get the hostname using uname()
	struct utsname utsname;
	if (uname(&utsname) == -1) {
		perror("uname failed.");
		exit(1);
	}

	eventStdParams[3] = strdup(utsname.nodename);
	uc_log("Determined hostname: %s\n", eventStdParams[3]);

	EVENT_NAME_ACCEPT = JniNewStringUTF(jniEnv, "Accept");
	EVENT_NAME_CLONE = JniNewStringUTF(jniEnv, "Clone");
	EVENT_NAME_CLOSE = JniNewStringUTF(jniEnv, "Close");
	EVENT_NAME_CONNECT = JniNewStringUTF(jniEnv, "Connect");
	EVENT_NAME_DUP = JniNewStringUTF(jniEnv, "Dup");
	EVENT_NAME_DUP2 = JniNewStringUTF(jniEnv, "Dup2");
	EVENT_NAME_EXECVE = JniNewStringUTF(jniEnv, "Execve");
	EVENT_NAME_EXIT = JniNewStringUTF(jniEnv, "Exit");
	EVENT_NAME_EXITGROUP = JniNewStringUTF(jniEnv, "ExitGroup");
	EVENT_NAME_FCNTL = JniNewStringUTF(jniEnv, "Fcntl");
	EVENT_NAME_FTRUNCATE = JniNewStringUTF(jniEnv, "Ftruncate");
	EVENT_NAME_KILL = JniNewStringUTF(jniEnv, "Kill");
	EVENT_NAME_MMAP = JniNewStringUTF(jniEnv, "Mmap");
	EVENT_NAME_MUNMAP = JniNewStringUTF(jniEnv, "Munmap");
	EVENT_NAME_OPEN = JniNewStringUTF(jniEnv, "Open");
	EVENT_NAME_PIPE = JniNewStringUTF(jniEnv, "Pipe");
	EVENT_NAME_READ = JniNewStringUTF(jniEnv, "Read");
	EVENT_NAME_RENAME = JniNewStringUTF(jniEnv, "Rename");
	EVENT_NAME_SENDFILE = JniNewStringUTF(jniEnv, "Sendfile");
	EVENT_NAME_SHUTDOWN = JniNewStringUTF(jniEnv, "Shutdown");
	EVENT_NAME_SOCKET = JniNewStringUTF(jniEnv, "Socket");
	EVENT_NAME_SOCKETPAIR = JniNewStringUTF(jniEnv, "Socketpair");
	EVENT_NAME_SPLICE = JniNewStringUTF(jniEnv, "Splice");
	EVENT_NAME_TEE = JniNewStringUTF(jniEnv, "Tee");
	EVENT_NAME_TRUNCATE = JniNewStringUTF(jniEnv, "Truncate");
	EVENT_NAME_UNLINK = JniNewStringUTF(jniEnv, "Unlink");
	EVENT_NAME_WRITE = JniNewStringUTF(jniEnv, "Write");
}

param *createParam(char *key, char *val) {
	param *p = malloc(sizeof(param));
	p->key = JniNewStringUTF(jniEnv, key);
	p->val = JniNewStringUTF(jniEnv, val);
	return p;
}

void destroyParam(param *p) {
	free(p);
}

event *createEvent(jstring name, int cntParams) {
	event *e = malloc(sizeof(event));
	e->name = name;
	e->isActual = true;
	e->cntParams = cntParams;
	e->iterParams = 0;
	e->params = malloc(cntParams * sizeof(param*));
	return e;
}

event *createEventWithStdParams(jstring name, int cntParams) {
	event *e = createEvent(name, cntParams + EVENT_STD_PARAMS_CNT);

	int i;
	for (i = 0; i < EVENT_STD_PARAMS_CNT; i++) {
		addParam(e, createParam(eventStdParams[i*2], eventStdParams[i*2+1]));
	}

	return e;
}

bool addParam(event *ev, param *p) {
	if (ev->iterParams < ev->cntParams) {
		ev->params[ev->iterParams] = p;
		ev->iterParams++;
		return true;
	}
	return false;
}

void destroyEvent(event *e) {
	int i;
	for (i = 0; i < e->cntParams; i++) {
		destroyParam(e->params[i]);
	}
	free(e->params);
}

