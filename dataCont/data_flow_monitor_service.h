#ifndef DATA_FLOW_MONITOR_SERVICE_H
#define DATA_FLOW_MONITOR_SERVICE_H


#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>

#include "soup_utils.h"
#include "dataCont.h"
#include "network_utils.h"
#include "threads.h"
#include "control_monitor.h"
#include "str_utils.h"

#define DFMS_SERVICE_URI_LEN		1024
#define DFMS_PORT 					8082
#define DFMS_NAME 					"/data_flow_monitor_service"
#define DFMS_MAX_CALL_TRIES 		10

// method names
#define DFMS_CALL_DATAFORSINK		"dataForSink"
#define DFMS_CALL_ACCEPT			"accept"
#define DFMS_CALL_SHUTDOWN			"shutdown"
#define DFMS_CALL_INSERTMECHANISM	"insertMechanism"


#define EMPTY_STRING 				""





// method definitions

void 			data_flow_monitor_handler(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data);
int 			data_flow_monitor_service(dataCont_ptr dc);

void 			service_handler_accept(SoupMessage *msg, GValueArray *params);
char 			*service_caller_accept(char *remote_ip, char *remote_port, char *local_ip, char *local_port, char *local_cont_id);

void 			service_handler_dataForSink(SoupMessage *msg, GValueArray *params);
void 			service_caller_dataForSink(char *remote_ip, list_of_data_ptr lod, unsigned int lod_len, char *cont, int *mechanisms_per_data, unsigned char **mechanisms);

void 			service_handler_shutdown(SoupMessage *msg, GValueArray *params);
void 			service_caller_shutdown(char *remote_ip, char *name, char *how);

void 			service_handler_insertMechanism(SoupMessage *msg, GValueArray *params);

/*
void data_flow_monitor_service(dataCont_ptr dc);
void *wait_for_remote_pips(void *arg);
void new_remote_pip_thread(void *arg);
void service_handler_accept_req(int *socket, char *readBuf);
char *service_caller_accept(char *remote_ip, char *remote_port, char *local_ip, char *local_port, char *local_cont_id);
void *wait_message(void *arg);
void service_handler_shutdown(char *msg);
void service_caller_shutdown(char *remote_ip, char *name, char *how);*/
#endif
