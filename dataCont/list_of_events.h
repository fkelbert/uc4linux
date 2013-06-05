/*
 * list_of_events.h
 *
 *  Created on: 14/lug/2010
 *      Author: lovat
 */

#ifndef LIST_OF_EVENTS_H_
#define LIST_OF_EVENTS_H_

#include <unistd.h>
#include "list.h"
#include "str_utils.h"

#define SYS_recvfrom 5100
#define SYS_recvmsg 5101
#define SYS_sendmsg 5102
#define SYS_sendto 5103
#define SYS_socket 5104
#define SYS_bind 5105
#define SYS_shutdown 5106
#define SYS_listen 5107
#define SYS_connect 5108
#define SYS_accept 5109

int syscall_to_int (char* syscall);

#endif /* LIST_OF_EVENTS_H_ */
