/*
 * list_of_events.h
 *
 *  Created on: 14/lug/2010
 *      Author: lovat
 */

#ifndef LIST_OF_EVENTS_H_
#define LIST_OF_EVENTS_H_

#include <glib.h>
#include <mxml.h>
#include "eventTypes.h"

extern int DFM_defaultActions[];
extern char *DFM_eventnames[];
extern GHashTable *syscallTable;
extern GHashTable *defaultSyscallTable;



unsigned int initSyscallTable();
int syscallToInt(const unsigned char *);
int syscallAction(int syscallCode);
unsigned int initDefaultAnswerTable(char *file);

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

#define SOCKET_OFFSET 347

#define SYS_socket 348
#define SYS_bind 349
#define SYS_connect 350
#define SYS_listen 351
#define SYS_accept 352
#define SYS_getsockname 353
#define SYS_getpeername 354
#define SYS_socketpair 355
#define SYS_send 356
#define SYS_recv 357
#define SYS_sendto 358
#define SYS_recvfrom 359
#define SYS_shutdown 360
#define SYS_setsockopt 361
#define SYS_getsockopt 362
#define SYS_sendmsg 363
#define SYS_recvmsg 364

#endif /* LIST_OF_EVENTS_H_ */
