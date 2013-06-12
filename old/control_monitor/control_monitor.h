#ifndef CONTROL_MONITOR_H
#define CONTROL_MONITOR_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>

#include "event.h"
#include "dataCont.h"
#include "libpbl/pbl.h"

/* Time units */
#define CM_USECOND             10
#define CM_MSECOND             11
#define CM_SECOND              12
#define CM_MINUTE              13
#define CM_HOUR                14
#define CM_DAY                 15
#define CM_WEEK                16
#define CM_MONTH               17

// Possible mechanims responses
#define ALLOW               0
#define INHIBIT             1

xmlDocPtr mech_doc;

#define MECHANISM_TABLE_SIZE    8       //starting size of the mechanism hash table

typedef struct action_s {
  char *id;
  int type;
  int n_params;
  param_t *params;
  int component;
} action_t;

typedef struct mechanism_actions_s {
  int response;             // ALLOW, INHIBIT, MODIFY or DELAY (and ALLOW!)
  // DELAY
  int delay;                //delay time in microseconds
  // For MODIFY
  int n_params;             //number of parameters in array
  param_t *modify;          //array of parameters to be modified
  // EXECUTE
  int n_actions;            //number of actions in array
  action_t *execute;        //array of actions to be executed
} mechanism_actions_t;

typedef struct mechanism_s {
  char *id;                     // unique mechanism identifier
  int is_active;
  int64_t usec_start;
  event_t* trigger;             // mechanism trigger event
  char *condition;              // mechanism condition
  mechanism_actions_t *actions; // mechanism actions
} mechanism_t;


/* Initializes the mechanism manager
 * @RETURN  0               sucess
 * @RETURN  -1              error: could not initialize library
 */
int init_control_monitor();

/* Creates a new mechanism
 * @PARAM   id              unique mechanism identifier
 * @PARAM   trigger         mechanism trigger event
 * @PARAM   condition       mechanism condition
 * @PARAM   actions         array of mechanism actions
 * @PARAM   num_actions     number of mechanism actions in actions array
 * @RETURN  0               sucess
 * @RETURN  -1              error: monitor could not be created for mechanism condition 
 * @RETURN  -2              error: id is already being used
 */
int add_mechanism(char *id, event_t* trigger, char *condition, mechanism_actions_t *actions, int64_t usec_timestep);
void reset_control_table();

/* Deletes the mechanism, if it exists
 * @PARAM   id              unique mechanism identifier
 */
void delete_mechanism(char *id);
void delete_all_mechanisms();

/* Resets the state of the mechanism condition and the mechanism activation time
 * @PARAM   id              unique mechanism identifier
 */
void reset_mechanism(char *id);

/* Changes mechanism state to active.
 *  @PARAM  id              unique mechanism identifier
 */
void activate_mechanism(char *id);

/* Changes mechanism state to inactive
 *  @PARAM  id              unique mechanism identifier
 */
void disactivate_mechaninsm(char *id);

/* Checks if a mechanism exists
 * @PARAM   id              unique mechanism identifier
 * @RETURN  ACTIVE          if mechanism exists and monitor's state is active 
 * @RETURN  INACTIVE        if mechanism exists and monitor's state is inactive
 * @RETURN  -1              if mechanism does not exist
 */
int lookup_mechanism(char *id);

/* Checks if a desired event can become an actual event and if it triggers any control mechanisms
 * @PARAM   desired_event   requested event
 * @RETURN  NULL            desired_event is allowed
 * @RETURN  !NULL           event triggered 
 */
mechanism_actions_t* new_event_request(event_t* desired_event);

// Log the action
void plog_actions(int log_level, mechanism_actions_t *r_actions);

void parseEvent(xmlNode * node1, event_t **a_event);
int parseParams(xmlNode * node, param_t **p_params);
void parseFormula(xmlNode * node, char *condition);
void parseControlMechanism(xmlNode * node0, char **r_id, event_t ** r_event, char *condition, mechanism_actions_t **r_actions);




/**
 * Maps the data with the specified data ID to the specified mechanism
 * @PARAM 	data		the data ID
 * @PARAM 	mechanism	the mechanism to map to the data ID
 */
void mapDataToMechanism(char *data, xmlNode *mechanism);


/**
 * Returns 	the PblSet* of mechanisms for the specified data ID. The Set contains elements of type xmlNode*.
 * @PARAM 	data		the data ID
 * @RETURN	NULL		if there are no such mechanisms
 * @RETURN	!NULL		the PblSet* of mechanisms
 */
PblSet *getMechanismsForData(char *data);



/**
 * Maps the data with the specified data ID to the specified mechanism
 * @PARAM 	data		the data ID
 * @PARAM 	mechanism	the mechanism to map to the data ID
 */
void mapDataToMechanism(char *data, xmlNode *mechanism);


/**
 * Returns 	the PblSet* of mechanisms for the specified data ID. The Set contains elements of type xmlNode*.
 * @PARAM 	data		the data ID
 * @RETURN	NULL		if there are no such mechanisms
 * @RETURN	!NULL		the PblSet* of mechanisms
 */
PblSet *getMechanismsForData(char *data);

unsigned char **getMechanismsForDataAsXML(char *data, size_t *size);
//unsigned char **getMechanismsForDataListAsXML(list_of_data_ptr lod, size_t *size);

int A_is_refinement_of_B (event_t* A, event_t* B);


int parse_and_add_mechanism(xmlNodePtr node);

#endif
