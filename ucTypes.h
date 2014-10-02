#ifndef UC_TYPES_H
#define UC_TYPES_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <jni.h>
#include <sys/utsname.h>
#include "ucLog.h"

jstring EVENT_NAME_ACCEPT;
jstring EVENT_NAME_CLONE;
jstring EVENT_NAME_CLOSE;
jstring EVENT_NAME_CONNECT;
jstring EVENT_NAME_DUP;
jstring EVENT_NAME_DUP2;
jstring EVENT_NAME_EXECVE;
jstring EVENT_NAME_EXIT;
jstring EVENT_NAME_EXITGROUP;
jstring EVENT_NAME_FCNTL;
jstring EVENT_NAME_FTRUNCATE;
jstring EVENT_NAME_KILL;
jstring EVENT_NAME_MMAP;
jstring EVENT_NAME_MUNMAP;
jstring EVENT_NAME_OPEN;
jstring EVENT_NAME_PIPE;
jstring EVENT_NAME_READ;
jstring EVENT_NAME_RENAME;
jstring EVENT_NAME_SENDFILE;
jstring EVENT_NAME_SHUTDOWN;
jstring EVENT_NAME_SOCKET;
jstring EVENT_NAME_SOCKETPAIR;
jstring EVENT_NAME_SPLICE;
jstring EVENT_NAME_TEE;
jstring EVENT_NAME_TRUNCATE;
jstring EVENT_NAME_UNLINK;
jstring EVENT_NAME_WRITE;

struct s_param {
	jstring key;
	jstring val;
};
typedef struct s_param param;

struct s_event {
	jstring name;
	param **params;
	int cntParams;
	bool isActual;
	int iterParams;
};
typedef struct s_event event;

extern char *eventStdParams[];

void ucTypesInit(JNIEnv *mainJniEnv);

inline param *createParam(char *key, char *val);
inline void destroyParam(param *p);
inline event *createEventWithStdParams(jstring name, int cntParams);
inline void destroyEvent(event *e);
inline bool addParam(event *ev, param *p);

#define is_actual(tcp) 		(!exiting(tcp))
#define is_desired(tcp) 	(exiting(tcp))

#endif

