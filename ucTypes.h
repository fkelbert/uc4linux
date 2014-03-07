#ifndef UC_TYPES_H
#define UC_TYPES_H

#define EVENT_NAME_READ "Read"
#define EVENT_NAME_CLOSE "Close"
#define EVENT_NAME_DUP "Dup"
#define EVENT_NAME_DUP2 "Dup2"
#define EVENT_NAME_EXECVE "Execve"
#define EVENT_NAME_EXIT "Exit"
#define EVENT_NAME_EXITGROUP "ExitGroup"
#define EVENT_NAME_FCNTL "Fcntl"
#define EVENT_NAME_KILL "Kill"
#define EVENT_NAME_MMAP "Mmap"
#define EVENT_NAME_OPEN "Open"
#define EVENT_NAME_OPENAT "OpenAt"
#define EVENT_NAME_PIPE "Pipe"
#define EVENT_NAME_RENAME "Rename"
#define EVENT_NAME_SOCKET "Socket"
#define EVENT_NAME_SOCKETPAIR "Socketpair"
#define EVENT_NAME_SPLICE "Splice"
#define EVENT_NAME_TEE "Tee"
#define EVENT_NAME_UNLINK "Unlink"
#define EVENT_NAME_WRITE "Write"

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

struct s_param {
	char *key;
	char *val;
};
typedef struct s_param param;

struct s_event {
	char *name;
	param **params;
	int cntParams;
	bool isActual;
	int iterParams;
};
typedef struct s_event event;


#define EVENT_STD_PARAMS_CNT 2
extern char *eventStdParams[];

param *createParam(char *key, char *val);
void destroyParam(param *p);
event *createEvent(char *name, int cntParams);
event *createEventWithStdParams(char *name, int cntParams);
void destroyEvent(event *e);
bool addParam(event *ev, param *p);

#endif

