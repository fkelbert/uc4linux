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
#include <dev/systrace.h>
#include <sys/syscall.h>
#include "action.h"
#include "esfevent.h"
#include "systrace_utils.h"
#include "openbsd_utils.h"
//#include "processSyscall.h"
#include "pdp.h"

extern action_desc_store_ptr pdp_action_desc_store;
static int PRINT_TRACE=1;
static int IGNORE_SYS_FILES=1;

static unsigned char* const fdStr[]={"stdin", "stdout","stderr","unknown"};

int may_ignore(char *filename);
event_ptr parseSyscall(int cfd, struct str_message* sys_msg);
void processResponse(notifyResponse_ptr eventResponse, struct str_message *sys_msg, struct systrace_answer *sys_ans);
event_ptr processSyscall(struct str_message *sys_msg);
unsigned int processStandardParameters(event_ptr event, struct str_message* sys_msg);
unsigned int processResponseParameter(event_ptr event, struct str_message* sys_msg);

event_ptr gProcessSyscall(struct str_message *sys_msg);

#endif /* SYSCALLHANDLER_H_ */
