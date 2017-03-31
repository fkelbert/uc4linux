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
	JNIEnv *jniEnv;

	void ucTypesSetJniEnv(JNIEnv *mainJniEnv);
#elif UC_THRIFT
#endif

#define EVENT_NAME_ACCEPT "Accept"
#define EVENT_NAME_CHROOT "Chroot"
#define EVENT_NAME_CLONE "Clone"
#define EVENT_NAME_CLOSE "Close"
#define EVENT_NAME_CONNECT "Connect"
#define EVENT_NAME_CREAT "Creat"
#define EVENT_NAME_DUP "Dup"
#define EVENT_NAME_DUP2 "Dup2"
#define EVENT_NAME_EXECVE "Execve"
#define EVENT_NAME_EXIT "Exit"
#define EVENT_NAME_EXITGROUP "ExitGroup"
#define EVENT_NAME_FCNTL "Fcntl"
#define EVENT_NAME_FTRUNCATE "Ftruncate"
#define EVENT_NAME_KILL "Kill"
#define EVENT_NAME_MMAP "Mmap"
#define EVENT_NAME_MUNMAP "Munmap"
#define EVENT_NAME_OPEN "Open"
#define EVENT_NAME_PIPE "Pipe"
#define EVENT_NAME_READ "Read"
#define EVENT_NAME_RENAME "Rename"
#define EVENT_NAME_SENDFILE "Sendfile"
#define EVENT_NAME_SHUTDOWN "Shutdown"
#define EVENT_NAME_SOCKET "Socket"
#define EVENT_NAME_SOCKETPAIR "Socketpair"
#define EVENT_NAME_SPLICE "Splice"
#define EVENT_NAME_TEE "Tee"
#define EVENT_NAME_TRUNCATE "Truncate"
#define EVENT_NAME_UNLINK "Unlink"
#define EVENT_NAME_WRITE "Write"


struct s_param {
	char *key;
	char *val;
};
typedef struct s_param param;

struct s_event {
	char name[64];
	param **params;
	int cntParams;
	bool isActual;
	int iterParams;
};
typedef struct s_event event;

void ucTypesInit();

static event *theEvent;

static param *paramAddr;
static param *paramAllowImpliesActual;
static param *paramDir;
static param *paramCmdline;
static param *paramCpid;
static param *paramCwd;
static param *paramDomain;
static param *paramDstfd;
static param *paramDstfilename;
static param *paramDstpid;
static param *paramFd;
static param *paramFd1;
static param *paramFd2;
static param *paramFilename;
static param *paramFlags;
static param *paramHost;
static param *paramHow;
static param *paramInfd;
static param *paramLocalip;
static param *paramLocalport;
static param *paramNew;
static param *paramNewfd;
static param *paramOld;
static param *paramOldfd;
static param *paramOperation;
static param *paramOutfd;
static param *paramOutfilename;
static param *paramPep;
static param *paramPid;
static param *paramPids;
static param *paramPipename;
static param *paramPpid;
static param *paramRemoteip;
static param *paramRemoteport;
static param *paramSocketname;
static param *paramSocketname1;
static param *paramSocketname2;
static param *paramSrcfd;
static param *paramSrcpid;
static param *paramTrunc;
static param *paramType;

#define MAX_PARAMS 32

inline bool addParamAddr(char *addr);
inline bool addParamAllowImpliesActual();
inline bool addParamCmdline(char *cmd);
inline bool addParamCpid(int pid);
inline bool addParamCwd(char *cwd);
inline bool addParamDir(char *dir);
inline bool addParamDomain(char *domain);
inline bool addParamDstfd(int fd);
inline bool addParamDstfilename(char *filename);
inline bool addParamFd(int fd);
inline bool addParamFd1(int fd);
inline bool addParamFd2(int fd);
inline bool addParamFilename(char *filename);
inline bool addParamFlags(char *flags);
inline bool addParamHow(char *how);
inline bool addParamInfd(int fd);
inline bool addParamLocalip(unsigned long ip);
inline bool addParamLocalport(unsigned long port);
inline bool addParamNew(char *filename);
inline bool addParamNewfd(int fd);
inline bool addParamOld(char *filename);
inline bool addParamOldfd(int fd);
inline bool addParamOperation(char *operation);
inline bool addParamOutfd(int fd);
inline bool addParamOutfilename(char * filename);
inline bool addParamPid(int pid);
inline bool addParamPids(char *pids);
inline bool addParamPipename(char *pipename);
inline bool addParamPpid(int pid);
inline bool addParamRemoteip(unsigned long ip);
inline bool addParamRemoteport(unsigned long port);
inline bool addParamSocketname(char *sname);
inline bool addParamSocketname1(char *sname);
inline bool addParamSocketname2(char *sname);
inline bool addParamSrcfd(int fd);
inline bool addParamTrunc(int trunc);
inline bool addParamType(char *type);

inline event *createEventWithStdParams(char *name, int cntParams) ;

#endif

