/*
 * processSyscall.h
 *
 *  Created on: Jul 26, 2011
 *      Author: cornelius moucha
 */

#ifndef PROCESSSYSCALL_H_
#define PROCESSSYSCALL_H_

#include <sys/param.h>
#include <dev/systrace.h>
#include <sys/syscall.h>
#include "openbsd_utils.h"
#include "esfevent.h"
#include "pdp.h"

event_ptr processSyscall(bool isTry, struct str_message *sys_msg);
unsigned int processStandardParameters(event_ptr event, struct str_message* sys_msg);
unsigned int processResponseParameter(event_ptr event, struct str_message* sys_msg);

#endif /* PROCESSSYSCALL_H_ */
