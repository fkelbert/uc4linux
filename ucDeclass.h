/*
 * ucDeclass.h
 *
 *  Created on: Jul 1, 2013
 *      Author: user
 */

#ifndef UC_DECLASS_H_
#define UC_DECLASS_H_

#include "ucSyscall.h"
#include "ucPIP_main.h"
#include <glib.h>		// include this library _after_ defs.h, because of macro redefinition
#include <stdlib.h>
#include <sys/time.h>

#define UC_DECLASS_ENABLED 1
#define UC_DECLASS_PRINT 1

#if defined(UC_DECLASS_PRINT) && UC_DECLASS_PRINT
	#define ucDeclass_printSPlus(pid) ucDeclass_printSPlus_impl(pid)
	void ucDeclass_printSPlus_impl(pid_t pid);
#else
	#define ucDeclass_printSPlus(pid)
#endif



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
#define timevalDup(timevalPtr, value) if (!(timevalPtr = malloc(sizeof(struct timeval)))) { ucDeclass_errorExitMemory();} memcpy(timevalPtr, &value, sizeof(struct timeval))

gboolean ucDeclass_printSPlusElement(gpointer key, gpointer value, gpointer data);


void ucDeclass__init();

#endif /* UC_DECLASS_H_ */
