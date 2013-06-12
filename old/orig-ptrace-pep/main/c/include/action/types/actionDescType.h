/**
 * @file  actionDescType.h
 * @brief Header for data types related to action descriptions
 *
 * @author cornelius moucha
 **/

#ifndef ACTIONTYPES_H_
#define ACTIONTYPES_H_

#include <glib.h>
#include "paramDescType.h"
#include "pdpInterface.h"

// Classes of actions
/// usage action (default action class); action classes are not used internally!
#define ACTION_USAGE      0
/// signalling action
#define ACTION_SIGNALLING 1
/// other action
#define ACTION_OTHER      2

/// static textual representation of action classes; used for logging
static char* const actionClassString[]={"usage", "signalling", "other"};

typedef struct actionDescription_s  actionDescription_t;
typedef        actionDescription_t *actionDescription_ptr;


/**
 * @struct actionDescription_s
 * @brief  structure for action description
 *
 * An action description is internally used for describing system actions, e.g. opening a file.
 * When the PEP intercepts such an action, an event is created referencing this action and its
 * parameters (e.g. the filename for opening).
 * Conceptually the action descriptions are given in the preamble of the policy to indicate
 * actions used by the policy mechanisms and describe the system's behaviour. In principle all
 * system actions and all their allowed parameter values should be given in the policy, because
 * not properly introduced actions or parameter values indicate an unintended system behaviour.
 * \n\n
 * For now this is not mandatory! The action descriptions are set up based on incoming events referencing
 * a new action.
 *
**/
struct actionDescription_s
{
  /// action name referencing system action name
  char         *actionName;
  /// action class (usage, signalling or other)
  unsigned int  actionClass;
  /// hashtable of action parameters
  GHashTable   *paramDescriptions;
  /// list of mechanisms referencing this action, i.e. internal subscription to this action
  GSList       *mechanisms;
  /// list of condition nodes referencing this action, i.e. internal subscription to this action
  GSList       *conditions;
  /// Interface for PEP subscription for this action
  pdpInterface_ptr pepInterface;
  /// Interface for PXP executions for this action
  pdpInterface_ptr pxpInterface;
};


#endif /* ACTIONTYPES_H_ */






