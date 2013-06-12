/*
 * StringWorker.h
 *
 *  Created on: 02/11/2012
 *      Author: kosklain
 */

#ifndef STRINGWORKER_H_
#define STRINGWORKER_H_

#include <stdlib.h>
#include <sys/ptrace.h>
#include "Constants.h"


void getString(pid_t child, long addr, char *dataStr);
void putString(pid_t child, long addr, char *dataStr);

#endif /* STRINGWORKER_H_ */
