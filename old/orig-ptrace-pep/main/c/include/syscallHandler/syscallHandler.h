/*
 * syscallHandler.h
 *
 *  Created on: Jul 26, 2011
 *      Author: cornelius moucha
 */

#ifndef SYSCALLHANDLER_H_
#define SYSCALLHANDLER_H_

#include <fcntl.h>
#include <sys/param.h>
#include <sys/syscall.h>
#include "action.h"

#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/reg.h>
#include <sys/syscall.h>
#include <stdio.h>
#include <sys/user.h>
#include <string.h>
#include <stdlib.h>
#include <pwd.h>
#include "pdp.h"


extern pdp_ptr pdp;

extern GHashTable *pidfdtable;
extern GHashTable *pidtable;
extern GHashTable *userpidtable;
extern GHashTable *commandtable;

static int PRINT_TRACE=1;
static int IGNORE_SYS_FILES=1;

static unsigned char* const fdStr[]={"stdin", "stdout","stderr","unknown"};

int mayIgnore(char *filename);
const char *byte_to_binary (int x);
struct passwd *getUserInfo(int pid);
int allowSystemCall(int pid, event_ptr event);
int denySystemCallAfter(int pid, event_ptr event);
int denySystemCallBefore(int pid, event_ptr event);
event_ptr processStandardParameters(int pid, bool inCall, int code);
event_ptr parseSyscall(pid_t pid, long firstcode, bool inCall);
void getString(pid_t child, long addr, char *dataStr);
void putString(pid_t child, long addr, char *dataStr);
char* get_file_name(int pid, int fd);
unsigned int delete_fd_filename_mapping(int pid, int fd);
unsigned int store_fd_filename_mapping(int pid, int fd, char *filename);
unsigned int initializeFDTable();
unsigned int copy_fd_filename_mapping(int pid, int newpid);
unsigned int delete_table(int pid);
unsigned int change_command_value(int pid, char *command);
unsigned int create_command(int pid, char *command);
char *get_command_value(int pid);
int waitforCall(pid_t child, int Waitingfor);

void processResponseBefore(notifyResponse_ptr eventResponse, int pid, int direct);
void processResponseAfter(notifyResponse_ptr eventResponse, int pid, int direct);

notifyResponse_ptr prepareResponse(event_ptr event, int action);
char *get_userpid_value(int pid);
unsigned int initializePidTable();
unsigned int create_pid(int pid);
struct inCall *get_pid_value(int pid);
unsigned int delete_pid(int pid);
unsigned int change_pid_value(int pid, struct inCall *inc);


#define IGNORE_SYS_FILES 1



struct inCall
{
	int inCall;
	notifyResponse_ptr eventResponse;
	long code;

};

#endif /* SYSCALLHANDLER_H_ */
