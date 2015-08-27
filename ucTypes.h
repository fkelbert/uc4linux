#ifndef UC_TYPES_H
#define UC_TYPES_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/utsname.h>
#include "ucLog.h"
#include "ucJniBridge.h"

#define EVENT_STD_PARAMS_CNT 2

#if UC_JNI
	#include <jni.h>
	typedef jstring str;
#elif UC_THRIFT
	typedef char* str;
#else
	typedef char* str;	// Fallback
#endif



str EVENT_NAME_ACCEPT;
str EVENT_NAME_CHROOT;
str EVENT_NAME_CLONE;
str EVENT_NAME_CLOSE;
str EVENT_NAME_CONNECT;
str EVENT_NAME_CREAT;
str EVENT_NAME_DUP;
str EVENT_NAME_DUP2;
str EVENT_NAME_EXECVE;
str EVENT_NAME_EXIT;
str EVENT_NAME_EXITGROUP;
str EVENT_NAME_FCNTL;
str EVENT_NAME_FTRUNCATE;
str EVENT_NAME_KILL;
str EVENT_NAME_MMAP;
str EVENT_NAME_MUNMAP;
str EVENT_NAME_OPEN;
str EVENT_NAME_PIPE;
str EVENT_NAME_READ;
str EVENT_NAME_RENAME;
str EVENT_NAME_SENDFILE;
str EVENT_NAME_SHUTDOWN;
str EVENT_NAME_SOCKET;
str EVENT_NAME_SOCKETPAIR;
str EVENT_NAME_SPLICE;
str EVENT_NAME_TEE;
str EVENT_NAME_TRUNCATE;
str EVENT_NAME_UNLINK;
str EVENT_NAME_WRITE;

struct s_param {
	str key;
	str val;
};
typedef struct s_param param;

struct s_event {
	str name;
	param **params;
	int cntParams;
	bool isActual;
	int iterParams;
};
typedef struct s_event event;

extern char *eventStdParams[];

void ucTypesInit();

#if UC_JNI
void ucTypesSetJniEnv(JNIEnv *mainJniEnv);
#endif


static inline str createString(char *s) {
#if UC_JNI
	return JniNewStringUTF(jniEnv, s);
#elif UC_THRIFT
	return strdup(s);
#else
	printf("Unknown option.\n");
	return "";
#endif
}

static inline void destroyString(str s) {
#if UC_JNI
	return;
#elif UC_THRIFT
	free(s);
#else
	printf("Unknown option.\n");
#endif
}



static inline param *createParam(char *key, char *val) {
	param *p = (param *) malloc(sizeof(param));
	p->key = createString(key);
	p->val = createString(val);
	return p;
}


static inline bool addParam(event *ev, param *p) {
	if (ev->iterParams < ev->cntParams) {
		ev->params[ev->iterParams] = p;
		ev->iterParams++;
		return true;
	}
	return false;
}



static inline void destroyParam(param *p) {
	destroyString(p->key);
	destroyString(p->val);
	free(p);
}

static inline event *createEvent(str name, int cntParams) {
	event *e = (event *) malloc(sizeof(event));
	e->name = name;
	e->isActual = true;
	e->cntParams = cntParams;
	e->iterParams = 0;
	e->params = (param**) malloc(cntParams * sizeof(param*));
	return e;
}




static inline event *createEventWithStdParams(str name, int cntParams) {
	event *e = createEvent(name, cntParams + EVENT_STD_PARAMS_CNT);

	int i;
	for (i = 0; i < EVENT_STD_PARAMS_CNT; i++) {
		addParam(e, createParam(eventStdParams[i*2], eventStdParams[i*2+1]));
	}

	return e;
}


static inline void destroyEvent(event *e) {
	int i;
	for (i = 0; i < e->cntParams; i++) {
		destroyParam(e->params[i]);
	}
	free(e->params);
}



#define is_actual(tcp) 		(!exiting(tcp))
#define is_desired(tcp) 	(exiting(tcp))

#endif

