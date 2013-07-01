/*
 * ucDeclass.h
 *
 *  Created on: Jul 1, 2013
 *      Author: user
 */

#ifndef UC_DECLASS_H_
#define UC_DECLASS_H_

#include <glib.h>
#include <stdlib.h>
#include <sys/time.h>
#include "ucSyscall.h"

extern int ucDeclassEvents[];

struct declass {
		pid_t pid;
		struct timeval time;
		int syscallno;
		char *data;
};

#define ucDeclass_errorExit(msg) \
			fprintf(stderr, "%s\n", msg); \
			fprintf(stderr, "Happened in: %s:%d\n", __FILE__, __LINE__); \
			fprintf(stderr, "Exiting.\n"); \
			exit (1);


void ucDeclass__init();

void ucDeclassEvent(pid_t pid, char scno);

#endif /* UC_DECLASS_H_ */
