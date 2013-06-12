/*
 * data_flow_monitor.h
 *  Created on: 14/lug/2010
 *      Author: lovat
 */


#ifndef DATA_FLOW_MONITOR_H_
#define DATA_FLOW_MONITOR_H_

#include <limits.h> // for INT_MAX
#define MAXNODES 1000
#define INF INT_MAX
#define SIZE_LAYER 10

#define FFOX_CLIPBOARD "FFOX_CLIPBOARD"
#define MAX_LENGTH_PARAM_LIST 30

#include "dataCont.h"
#include "list_of_events.h"
#include "esfevent.h"
#include "qdftConnector.hh"


typedef struct data_flow_monitor_s data_flow_monitor_t;
typedef data_flow_monitor_t       *data_flow_monitor_ptr;


struct data_flow_monitor_s
{
  char* layer;
  dataCont_ptr map;
};

data_flow_monitor_ptr data_flow_monitor;
//array of data-provenance graphs
qdftmanager_ptr qdft[MAXNODES];
//array of initial-representations sizes
long maxsize[MAXNODES];
//array of "old values for the edge", needed for reverting last transfer of data
long oldEdge[MAXNODES];


int qdft_size;

/*************************/
/*************************/
/*************************/
/*************************/
/*************************/
/*************************/
/*************************/

//hash table...for the time being implemented as a matrix
char *hashtable[MAXNODES];
int hashcount;
//hash table...for the time being implemented as a matrix
char *datahashtable[MAXNODES];
int datahashcount;

/*************************/
/*************************/
/*************************/
/*************************/
/*************************/
/*************************/
/*************************/

bool stateFormula();


data_flow_monitor_ptr data_flow_monitor_new();

void data_flow_monitor_init(data_flow_monitor_ptr data_flow_monitor);
unsigned int data_flow_monitor_reset(data_flow_monitor_ptr data_flow_monitor);
unsigned int data_flow_monitor_update(data_flow_monitor_ptr data_flow_monitor, event_ptr event);
int get_deny_because_of_if();
void reset_deny_because_of_if();

// information flow updates (from past_monitor.c)
int IF_update(event_ptr event);
char* IF_initCont(char* cont, char* qod);

#endif /* DATA_FLOW_MONITOR_H_ */
