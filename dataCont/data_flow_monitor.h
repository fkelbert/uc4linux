/*
 * data_flow_monitor.h
 *
 *  Created on: 14/lug/2010
 *      Author: lovat
 */

#ifndef DATA_FLOW_MONITOR_H_
#define DATA_FLOW_MONITOR_H_

#define SIZE_LAYER 10

#include <sys/syscall.h>
#include <string.h>

#include "dataCont.h"
#include "list_of_events.h"
#include "event.h"
#include "logger.h"
#include "data_flow_monitor.h"
#include "network_utils.h"
#include "data_flow_monitor_service.h"
#include "libpbl/pbl.h"
#include "control_monitor.h"


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


void addAddrNaming(dataCont_ptr dc, char *HOSTxPID, char *cont_name, char *cont_id);
char *getAddrNaming(dataCont_ptr dc, char *HOSTxPID, char *cont_name);


int is_ADDR(char *pid);
char *to_ADDR(char *HOSTxADDR, char *HOSTxPID);
int is_GLOBAL_IP(char *addr);



void dfm_syscall_accept(dataCont_ptr dc, char *HOSTxPID, char *family, char *local_ip, char *local_port, char *remote_ip, char *remote_port, char *new_fd);
void dfm_syscall_bind(dataCont_ptr dc, char *PID, char *sfd, char *ipaddr, char *port);
void dfm_syscall_close(dataCont_ptr dc, char *PID, char *fd, char *filename);
void dfm_syscall_connect(dataCont_ptr dc, char *HOSTxPID, char *sfd, char *family, char *local_ip, char *local_port, char *remote_ip, char *remote_port);
void dfm_syscall_exit(dataCont_ptr dc, char *PID);
void dfm_syscall_listen(dataCont_ptr dc, char *PID, char *sfd);
void dfm_syscall_shutdown(dataCont_ptr dc, char *PID, char *sfd, char *how);
void dfm_syscall_socket(dataCont_ptr dc, char *PID, char *sfd);
void dfm_syscall_write(dataCont_ptr dc, char *HOSTxPID, char *fd, char *filename);

// helpers
void dfm_syscall_close_socket(dataCont_ptr dc, char *PID, char *fd);


unsigned char **get_all_mechanisms(list_of_data_ptr lod, int data_count, int *mechanisms_per_data);


#endif /* DATA_FLOW_MONITOR_H_ */
