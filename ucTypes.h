#ifndef UC_TYPES_H
#define UC_TYPES_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/utsname.h>
#include "ucLog.h"
#include "ucJniBridge.h"

#if UC_JNI
	#include <jni.h>
	typedef jstring str;
#elif UC_THRIFT
	typedef char* str;
#else
	typedef char* str;	// Fallback
#endif



str EVENT_NAME_ACCEPT;
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

inline param *createParam(char *key, char *val);
inline void destroyParam(param *p);
inline event *createEventWithStdParams(str name, int cntParams);
inline void destroyEvent(event *e);
inline bool addParam(event *ev, param *p);

#define is_actual(tcp) 		(!exiting(tcp))
#define is_desired(tcp) 	(exiting(tcp))

#endif

