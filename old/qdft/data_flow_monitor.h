/*
 * data_flow_monitor.h
 *
 *  Created on: 14/lug/2010
 *      Author: lovat
 */

#ifndef DATA_FLOW_MONITOR_H_
#define DATA_FLOW_MONITOR_H_

#define SIZE_LAYER 10

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <ifaddrs.h>

#include "dataCont.h"
#include "list_of_events.h"
#include "event.h"
#include "logger.h"


#define INADDR_ANY_IP 		"0.0.0.0"
#define FDT_NETWORK_SOCKET 	"***NETWORK_SOCKET***"
#define IP_ADDR_SEP 		','

typedef struct data_flow_monitor_s data_flow_monitor_t;
typedef data_flow_monitor_t* data_flow_monitor_ptr;

struct data_flow_monitor_s {
  char* layer;
  dataCont_ptr map;
};

data_flow_monitor_ptr data_flow_monitor_new();

void data_flow_monitor_init(data_flow_monitor_ptr data_flow_monitor);

unsigned int data_flow_monitor_reset(data_flow_monitor_ptr data_flow_monitor);

unsigned int data_flow_monitor_update(data_flow_monitor_ptr data_flow_monitor, event_ptr event);

int get_deny_because_of_if();

void reset_deny_because_of_if();


void syscall_bind(dataCont_ptr dc, char *PID, char *sfd, char *ipaddr, char *port, char* ADDR);
void syscall_connect(dataCont_ptr dc, char *PID, char *sfd, char *local_ip_addr, char *local_port, char *ADDR);
void syscall_close(dataCont_ptr dc, char *PID, char *fd);

#endif /* DATA_FLOW_MONITOR_H_ */
