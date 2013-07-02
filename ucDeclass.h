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
#include "ucPIP.h"

#define UC_DECLASS_ENABLED 1
#define UC_DECLASS_DEBUG 1

#if defined(UC_DECLASS_DEBUG) && UC_DECLASS_DEBUG
	#define ucDeclass_printSPlus(out, pid) ucDeclass_printSPlus_impl(out, pid)
	void ucDeclass_printSPlus_impl(FILE *out, pid_t pid);
#else
	#define ucDeclass_printSPlus(out, pid)
#endif


struct printSPlusData {
		pid_t pid;
		FILE *out;
};

#define ucDeclass_errorExit(msg) \
			fprintf(stderr, "%s\n", msg); \
			fprintf(stderr, "Happened in: %s:%d\n", __FILE__, __LINE__); \
			fprintf(stderr, "Exiting.\n"); \
			exit (1);

#define ucDeclass_errorExitMemory() \
			fprintf(stderr, "Unable to allocate enough memory\n"); \
			fprintf(stderr, "Happened in: %s:%d\n", __FILE__, __LINE__); \
			fprintf(stderr, "Exiting.\n"); \
			exit (1);

#define pidDup(pidPtr, value) if (!(pidPtr = malloc(sizeof(pid_t)))) { ucDeclass_errorExitMemory();	} *pidPtr = value

gboolean ucDeclass_printSPlusElement(gpointer key, gpointer value, gpointer data);


void ucDeclass__init();

#endif /* UC_DECLASS_H_ */
