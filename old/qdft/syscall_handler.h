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
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/types.h>
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

#include "data_flow_monitor.h"
#include "syscalls.h"

/*
typedef struct list_seqnr_s{
	u_int16_t seqnr;
	struct list_seqnr_s *next;
} list_seqnr_t;

typedef list_seqnr_t * list_seqnr_ptr;
*/



int attach_process(int cfd, int *pid);

int install_policy(int cfd, int pid);

int ask_syscalls(int cfd, int syscall_code, struct systrace_policy strpol);

event_t *handle_system_call(int cfd, struct str_message* sys_msg);


char *get_all_ip_addresses(char *buf, int size, int lo);
char *host_x_pid(char *buf, int size, int pid);

#endif
