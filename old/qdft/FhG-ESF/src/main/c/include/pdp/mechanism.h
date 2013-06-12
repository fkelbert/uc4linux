#ifndef mechanism_h
#define mechanism_h

#include "esfevent.h"
#include "formula.h"
#include "time_utils.h"

#define ACTION_INHIBIT 0
#define ACTION_ALLOW   1

esf_typedef(action);
esf_typedef(action_match_op);
esf_typedef(mechanism_actions);
esf_typedef(mechanism);

/******************************************************************************
 *  Action definitions
******************************************************************************/
struct action_s
{
  action_desc_ptr action;
  unsigned int   n_params_to_action;
  action_match_op_ptr *params;
};

action_ptr    action_new(const unsigned char *name);
unsigned int  action_free(action_ptr);

unsigned int action_addParamInstance(mechanism_ptr curMechm, action_ptr lAction, const unsigned char *name,
                                     const unsigned char *value, const unsigned char *isXPath);

struct action_match_op_s {
  action_param_desc_ptr  param_desc;
  unsigned char         *value;
  bool                   is_xpath;
};

action_match_op_ptr action_match_op_new(action_param_desc_ptr, const unsigned char *, const unsigned char *);
unsigned int action_match_op_free();



/******************************************************************************
 *  Mechanism_actions definitions
******************************************************************************/
// mechanism_actions refer to authorizationActions
struct mechanism_actions_s
{
  unsigned int   response;                  // ALLOW, INHIBIT
  uint64_t       delay;                     // DELAY time in microseconds
  // MODIFY
  unsigned int           n_params_to_modify;      // number of parameters in array
  action_match_op_ptr     *params_to_modify;      // array of parameter INSTANCES to be modified
};

mechanism_actions_ptr mechanism_actions_new(unsigned int, unsigned int);
unsigned int          mechanism_actions_free(mechanism_actions_ptr);
mechanism_actions_ptr mechanism_actions_parse_xml(mechanism_ptr,xmlNodePtr);
unsigned int          mechanism_actions_addModifyParam(mechanism_ptr, mechanism_actions_ptr, const unsigned char *, const unsigned char *, const unsigned char *);
unsigned long         mechanism_actions_getMemSize(mechanism_actions_ptr);

unsigned int mechanism_execute_parse_xml(mechanism_ptr, xmlNodePtr);

/******************************************************************************
 *  Mechanism definitions
******************************************************************************/
struct mechanism_s
{
  unsigned char        *mech_name;            // unique mechanism identifier
  timestamp_ptr         timestamp_start;      // timestep of mechanism_initialization
  timestamp_ptr         timestamp_lastUpdate; // last update timestep
  uint64_t              timestepSize;         // mechanism timestepSize
  uint64_t              timestep;             // absolute number of current timestep
  event_match_op_ptr    trigger;
  formula_ptr           condition;            // mechanism condition
  mechanism_actions_ptr actions;              // mechanism actions
  // EXECUTE
  unsigned int          n_actions_to_execute;       // number of actions in array
  action_ptr           *actions_to_execute;         // array of actions to be executed
};

mechanism_ptr mechanism_new(unsigned char *);
unsigned int  mechanism_free(mechanism_ptr);
unsigned long mechanism_getMemSize(mechanism_ptr);
void          mechanism_log(mechanism_ptr);
mechanism_ptr mechanism_parse_xml(xmlNodePtr);
uint64_t      mechanism_parse_timestepSize(xmlNodePtr);
unsigned int mechanism_addAction(mechanism_ptr ,  xmlNodePtr, const unsigned char *);
xmlDocPtr mechanism_serialize_actions(mechanism_ptr mech);

#endif
