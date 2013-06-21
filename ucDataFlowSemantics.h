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

#include <unistd.h>
#include <stdio.h>

void ucPIPupdate(struct tcb *tcp);

void ucDataFlowSemanticsWrite(struct tcb *tcp);
void ucDataFlowSemanticsRead(struct tcb *tcp);
void ucDataFlowSemanticsExit(struct tcb *tcp);

#endif /* UCSEMANTICS_H_ */
