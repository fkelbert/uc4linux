/*
 * syscall_handler.c
 *
 *      Authors: Ricardo Neisse, Carolina Lorini
 */

#ifndef syscall_handler_h
#define syscall_handler_h
#include <err.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/syscall.h>
#include <libsoup/soup.h>
#include <dev/systrace.h>

#include <libxml/parser.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>


#include "logger.h"
#include "event.h"
#include "control_monitor.h"
#include "soup_utils.h"
#include "openbsd_utils.h"
#include "systrace_utils.h"
#include "network_utils.h"
#include "filesystem_utils.h"
#include "past_monitor.h"
#include "xml_utils.h"
#include "threads.h"

#include "data_flow_monitor.h"
#include "syscalls.h"

#define SYSCALL_HANDLER_SERVICE_NAME "/syscall_handler"
#define SYSCALL_HANDLER_SERVICE_PORT 8081

/*
typedef struct list_seqnr_s{
	u_int16_t seqnr;
	struct list_seqnr_s *next;
} list_seqnr_t;

typedef list_seqnr_t * list_seqnr_ptr;
*/

int do_attach_process (SoupMessage *msg, GValueArray *params);

int attach_process(int cfd, int *pid);

int install_policy(int cfd, int pid);

int ask_syscalls(int cfd, int syscall_code, struct systrace_policy strpol);

event_t *handle_system_call(int cfd, struct str_message* sys_msg);

event_t *handle_syscall_shutdown(struct str_message* sys_msg);

void init_syscall_handler_service();

#endif
