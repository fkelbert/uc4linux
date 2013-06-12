/*
 * list_of_events.h
 *
 *  Created on: 14/lug/2010
 *      Author: lovat
 */

#ifndef LIST_OF_EVENTS_H_
#define LIST_OF_EVENTS_H_

#include <glib/ghash.h>
#include "str_utils.h"
#include "eventTypes.h"

extern char *DFM_eventnames[];
extern GHashTable *syscallTable;

int syscall_to_int (char* syscall);

unsigned int initSyscallTable();
int syscallToInt(const unsigned char *);


typedef struct syscallParam_s
{
  unsigned int   paramType;
  char          *paramName;
} syscallParam_t;

typedef struct esfsyscall_s {
  char            *name;
  unsigned int     numParams;
  syscallParam_t   params[];
} esfsyscall_t;
typedef esfsyscall_t *esfsyscall_ptr;


#endif /* LIST_OF_EVENTS_H_ */
