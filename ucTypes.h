#ifndef UC_TYPES_H
#define UC_TYPES_H

#include <limits.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <sys/utsname.h>
#include "ucLog.h"
#include "ucJniBridge.h"

#define PID_LEN 6
#define FD_LEN 6

#define toPid(str,i) snprintf(str,PID_LEN,"%u", (unsigned int)i)
#define toFd(str,i) snprintf(str,FD_LEN,"%u", (unsigned int)i)

#define is_actual(tcp) 		(!exiting(tcp))
#define is_desired(tcp) 	(exiting(tcp))

#if UC_JNI
	#include <jni.h>
	typedef jstring str;
	JNIEnv *jniEnv;

	#define createString(s) 	JniNewStringUTF(jniEnv, s)
	#define destroyString(s)
	void ucTypesSetJniEnv(JNIEnv *mainJniEnv);
#elif UC_THRIFT
	typedef char* str;

	#define createString(s) 	strdup(s)
	#define destroyString(s)	free(s)
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

void ucTypesInit();

static event *theEvent;

//static param *paramAddr;
static param *paramAllowImpliesActual;
//static param *paramDir;
//static param *paramCmdline;
//static param *paramCpid;
//static param *paramCwd;
//static param *paramDomain;
//static param *paramDstfd;
//static param *paramDstfilename;
//static param *paramDstpid;
static param *paramFd;
//static param *paramFd1;
//static param *paramFd2;
static param *paramFilename;
//static param *paramFlags;
//static param *paramHost;
//static param *paramHow;
//static param *paramInfd;
static param *paramLocalip;
static param *paramLocalport;
//static param *paramNew;
static param *paramNewfd;
//static param *paramOld;
static param *paramOldfd;
//static param *paramOperation;
//static param *paramOutfd;
//static param *paramOutfilename;
//static param *paramPep;
static param *paramPid;
//static param *paramPids;
//static param *paramPpid;
static param *paramRemoteip;
static param *paramRemoteport;
static param *paramSocketname;
//static param *paramSocketname1;
//static param *paramSocketname2;
//static param *paramSrcfd;
//static param *paramSrcpid;
//static param *paramTrunc;
//static param *paramType;

#define MAX_PARAMS 32

inline param *createParam(char *key, char *val);
inline void destroyParam(param *p);

inline bool addParam(event *ev, param *p);

inline bool addParamAllowImpliesActual();
inline bool addParamFd(int fd);
inline bool addParamFilename(char *filename);
inline bool addParamLocalip(char *ip);
inline bool addParamLocalport(char *port);
inline bool addParamNewFd(int fd);
inline bool addParamOldFd(int fd);
inline bool addParamPid(int pid);
inline bool addParamRemoteip(char *ip);
inline bool addParamRemoteport(char *port);
inline bool addParamSocketname(char *sname);

inline event *createEventWithStdParams(str name, int cntParams) ;
inline void destroyEvent(event *e);


#endif


