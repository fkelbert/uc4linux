#ifndef UC_TYPES_H
#define UC_TYPES_H

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <jni.h>
#include <sys/utsname.h>
#include "ucLog.h"
#include "ucJniBridge.h"

#if UC_JNI
	typedef jstring string;
#elif UC_THRIFT
	typedef char* string;
#else
	typedef char* string;	// Fallback
#endif



string EVENT_NAME_ACCEPT;
string EVENT_NAME_CLONE;
string EVENT_NAME_CLOSE;
string EVENT_NAME_CONNECT;
string EVENT_NAME_DUP;
string EVENT_NAME_DUP2;
string EVENT_NAME_EXECVE;
string EVENT_NAME_EXIT;
string EVENT_NAME_EXITGROUP;
string EVENT_NAME_FCNTL;
string EVENT_NAME_FTRUNCATE;
string EVENT_NAME_KILL;
string EVENT_NAME_MMAP;
string EVENT_NAME_MUNMAP;
string EVENT_NAME_OPEN;
string EVENT_NAME_PIPE;
string EVENT_NAME_READ;
string EVENT_NAME_RENAME;
string EVENT_NAME_SENDFILE;
string EVENT_NAME_SHUTDOWN;
string EVENT_NAME_SOCKET;
string EVENT_NAME_SOCKETPAIR;
string EVENT_NAME_SPLICE;
string EVENT_NAME_TEE;
string EVENT_NAME_TRUNCATE;
string EVENT_NAME_UNLINK;
string EVENT_NAME_WRITE;

struct s_param {
	string key;
	string val;
};
typedef struct s_param param;

struct s_event {
	string name;
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
inline event *createEventWithStdParams(string name, int cntParams);
inline void destroyEvent(event *e);
inline bool addParam(event *ev, param *p);
inline string createString(char* str);

#define is_actual(tcp) 		(!exiting(tcp))
#define is_desired(tcp) 	(exiting(tcp))

#endif

