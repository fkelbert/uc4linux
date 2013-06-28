/*
 * ucSemantics.h
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#ifndef UCSEMANTICS_H_
#define UCSEMANTICS_H_

#include "defs.h"
#include "syscall.h"


#include <sys/fcntl.h>
#include <sys/types.h>
#include <limits.h>
#include <unistd.h>
#include <stdio.h>
#include <dirent.h>

void ucPIPupdate(struct tcb *tcp);

void ucDataFlowSemantics_write(struct tcb *tcp);
void ucDataFlowSemantics_read(struct tcb *tcp);
void ucDataFlowSemantics_exit(struct tcb *tcp);

#endif /* UCSEMANTICS_H_ */
