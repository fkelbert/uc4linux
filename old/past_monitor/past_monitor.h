#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <pthread.h>

#include "event.h"
#include "time_utils.h"
#include "str_utils.h"
#include "xml_utils.h"

#ifndef _past_monitor_h
#define _past_monitor_h

// circular array
typedef struct c_array_s {
  int* values;
  int  p_first; // first position
  int  p_next;  // last position
} c_array_t;

typedef struct state_s {
  char value;                   // indicates the (sub)formula's value (TRUE/FALSE)
  int immutable;
  int count_true;
  int was_ever_true;            // used by always, if the formula was ever FALSE
  int was_ever_false;            // used by always, if the formula was ever FALSE
  c_array_t *saved_values;      // time window of saved states for before
} state_t;

typedef struct formula_s {
  int op;		    // operation
  int op1;		  // first operator
  int op2;		  // second operator
  int op3;		  // used for replim and repuntil
  int op4;		  // used for replim
  event_t ev;   // the event for Eall and Efst
} formula_t;

typedef struct monitor_s {
  char *id;		               // Unique policy identifier
  char *formula;			   // formula to be monitored
  int value;                   // formula is TRUE or FALSE
  int is_active;               // indicates if monitor is active or inactive
  int timestep;                // Timestep counter

  int64_t usec_start;          // Start time
  int64_t usec_last;           // Last update time
  int64_t usec_timestep;       // Timestep size

  int n_formulas;              // number of sub-formulas
  formula_t *formulas;         // array of sub-formulas
  state_t *s_cur;              // array of states for each node
  state_t *s_sav;              // saved state for desired event
} monitor_t;



// Initializes the past monitor manager
int init_past_monitors();

void reset_monitor_table();

// Creates a new past monitor
int add_past_monitor(char *id, char *policy, int64_t tv_timestep);

// Changes monitor state to active.
void activate_past_monitor(char *id);

// Updates all monitors with the new event
void update_all_past_monitors(event_t* new_event);

// Functions from past_scanner.l
void init_scanner(char *src);

void close_scanner();

// Functions from past_parser.y
int parse_policy (char *policy, int *num, formula_t **p_table);

// Starts a thread that periodically checks the monitors/formulas
void start_past_monitor_update_thread(int64_t thread_usleep);

// The function that is executed by the update thread
void past_monitor_update_thread(int64_t *thread_usleep);

// Find the monitor in the hash table
monitor_t *lookup_past_monitor(char *id);

// Check the current value of a Monitor formula (TRUE/FALSE)
int check_past_monitor_value(char *id);

// Deletes the specified monitor, if it exists
void delete_past_monitor(char *id);

// Changes monitor state to inactive
void disactivate_past_monitor(char *id);

// Resets the state of the monitor policy and the start time
void reset_past_monitor(char *id);

// Update one monitor with the event, returns TRUE or FALSE
// three possibilities for new_event:
// - NULL event: we only update the states if the timestep has passed
// - TRY event: we check but do not update the states
// - event: we update the states
int update_past_monitor(monitor_t *p_monitor, event_t* new_event);

// Searches for the monitor by name in the hash table and updates
int update_past_monitor_by_id(char *id, event_t* new_event);




int IF_update(event_t* event);

char* IF_initCont(char* cont);




#endif
