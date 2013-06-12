/**
 * @file  mechActionTypes.h
 * @brief Type definition header for authorization and execution actions
 *
 * Provides type and structure definitions for authorization and execution actions used in mechanisms
 *
 * @author cornelius moucha
 **/

#ifndef MECHACTIONTYPES_H
#define MECHACTIONTYPES_H

#include "actionDescType.h"
#include "paramInstanceType.h"

typedef struct executeAction_s        executeAction_t;
typedef        executeAction_t       *executeAction_ptr;

typedef struct authorizationAction_s  authorizationAction_t;
typedef        authorizationAction_t *authorizationAction_ptr;

/**
 * @struct executeAction_s
 * @brief Represents an execute action as specified in the policy
 *
 * This structure represents an execute action as it is specified in the policy.
 * Execute actions refer to additional actions which have to be executed. An authorization action may require
 * an successful execution of an action for an allowing of the intercepted event. Furthermore execute actions
 * can be specified in the policy independent of the decision, e.g. for notifications, which should be executed
 * whenever the condition is satisfied.
 *
**/
struct executeAction_s
{
  /// reference to the action description store describing the action, e.g. the action name
  actionDescription_ptr  actionDesc;
  /// number of parameters for this action
  unsigned int           cntParams;
  /// array of parameter instances for this action
  parameterInstance_ptr *params;
};

/**
 * @struct authorizationAction_s
 * @brief Represents an authorization action as specified in the policy
 *
 * This structure represents an authorization action as it is specified in the policy.
 * Authorization actions refer to the decision of the PDP for incoming events sent by the PEP. In policies they specify what should
 * happen with the intercepted event, i.e. whether it should be allowed, modified, delayed or completely inhibited. Furthermore
 * additional execution actions might be required for an authorization action.
 *
**/
struct authorizationAction_s
{
  /// tbd
  char                    *name;
  bool                     start;
  /// ALLOW or INHIBIT of the intercepted event
  unsigned int             response;
  /// DELAY time in microseconds for the intercepted event
  uint64_t                 delay;
  /// MODIFY the intercepted events: number of parameters in array
  unsigned int             cntParamsModify;
  /// MODIFY the intercepted events: array of modification parameters
  parameterInstance_ptr   *paramsModify;
  /// Fallback authorization action if required executeActions could not be executed; implicit fallback is always INHIBIT
  authorizationAction_ptr  fallback;
  /// Temporary field: name of fallback authorizationAction
  char                    *fallbackName;
  /// number of executeActions in array
  unsigned int            cntExecuteActions;
  /// array of executeActions
  executeAction_ptr      *executeActions;

};

#endif /* MECHACTIONTYPES_H */





