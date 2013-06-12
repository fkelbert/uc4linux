/**
 * @file  pdpTypes.h
 * @brief Type definition header for PDP related implementation
 *
 * Provides type and structure definitions for PDP internal structures, e.g. mechanisms, authorization actions
 *
 * @author cornelius moucha
 **/

#ifndef PDPTYPES_H_
#define PDPTYPES_H_

#include "timestamp.h"
#include "threadUtils.h"
#include "eventTypes.h"
#include "opStateTypes.h"
#include "actionDescStore.h"
#include "mechActionTypes.h"


#define ACTION_INHIBIT 0
#define ACTION_ALLOW   1
#define ACTION_ASK 2

// Forward type definitions
typedef struct mechanism_s    mechanism_t;
typedef        mechanism_t   *mechanism_ptr;
typedef struct condition_s    condition_t;
typedef        condition_t   *condition_ptr;
typedef struct operator_s     operator_t;
typedef        operator_t    *operator_ptr;

// internal type definitions
typedef struct pdp_s             pdp_t;
typedef        pdp_t            *pdp_ptr;
typedef struct notifyResponse_s  notifyResponse_t;
typedef        notifyResponse_t *notifyResponse_ptr;


/**
 * @struct mechanism_s
 * @brief Represents a mechanism as specified in the policy
 *
 * Mechanisms refer to the main part of an OSL policy. They can either be a preventive mechanism or
 * a detective one. Both of them instantiate the ECA-principle: event, condition, action.
 * For detective mechanisms the event (the trigger) is NULL. The mentioned action may consist
 * of an authorization action (only for preventive mechanisms), which specify the intended
 * behaviour for the intercepted trigger event (allow, modify, delay or inhibit) in addition
 * to required execution action for this authorization decision. Furthermore the mechanism
 * may consist execution action independent of an authorization action, which should always
 * be executed when the condition is satisfied, e.g. for a notification message.
 * \n\n
 * This structure represents a mechanism as it is specified in the policy. The pair of mechanism name and its namespace (e.g. policy filename)
 * uniquely identifies a mechanism in the system. It contains the required information such as a trigger event for preventive enforcement,
 * the OSL condition and authorization actions, which specify what should happen with an intercepted event. Furthermore additonal execute
 * actions are stored in this structure. The mechanism's own update thread is instantiated when the mechanism is deployed in the system and
 * is used for updating the internal state of the mechanisms relative to its notion of time, i.e. its timestep size.
 *
**/
struct mechanism_s
{
  /// mechanism name
  char                   *mechName;
  /// namespace of mechanism, e.g. filename of policy
  char                   *ns;
  /// unique mechanism identifier (mechanism namespace and name)
  char                   *mechFullname;
  /// timestep of mechanism initialization
  timestamp_ptr           startTime;
  /// last update timestep
  timestamp_ptr           lastUpdate;
  /// mechanism timestep size
  uint64_t                timestepSize;
  /// absolute number of current timestep
  uint64_t                timestep;
  /// mechanism trigger event; NULL for detective mechanisms
  eventMatch_ptr          trigger;
  /// mechanism condition
  condition_ptr           condition;
  /// mechanism authorization actions
  authorizationAction_ptr authorizationActions;
  /// number of executeActions in array
  unsigned int            cntExecuteActions;
  /// array of executeActions
  executeAction_ptr      *executeActions;
  /// internal used update thread for this mechanism
  pefThread_ptr           updateThread;
  /// internal used mutex for processing this mechanism
  pefMutex_ptr            mechMutex;
};

/**
 * @struct condition_s
 * @brief Condition of a mechanism
 *
 * Conditions refer to the OSL formula specified in the mechanisms. These conditions are evaluated upon receiving of a matching
 * event and internally at the end of every timestep (depending on the mechanism's timestep size). The latter evaluation is
 * necessary to model the passing of time in the state machine for the temporal operators. The condition is stored as tree of the
 * single operator nodes, similar to the XML representation in the policy. This enables a traversal walk in post-order
 * (left-right-depth-first) for condition evaluation.
**/
struct condition_s
{
  /// Amount of operator nodes in this condition
  unsigned int  cntNodes;
  /// Array of operator nodes
  operator_ptr *nodes;
};

/**
 * @struct operator_s
 * @brief Structure representing an OSL operator
 *
* The OSL operators specify the condition which has to be evaluated by the PDP upon incoming events and
* time based, depending on the mechanism's notion of time. This structure hols information about one conrete operator, including
* its type, its operands (i.e. sub-formulas) and a type-dependent evaluation function.
**/

struct operator_s
{
  /// operator type specifies how to process the operands
  unsigned char type;
  /// operands can be operator_ptr, char *, event_ptr
  void*         operands;
  /// state of this operator
  state_ptr     state;
  /// type-dependent evaluation function; assigned during instantiation
  bool          (*eval)(operator_ptr, event_ptr, mechanism_ptr);
};


/**
 * @struct notifyResponse_s
 * @brief Represents a decision of the PDP for a notified event
 *
 * This structure represents the decision of the PDP for a received event notification. After evaluating all relevant mechanism this
 * structure hold information about the authorization action to be taken, i.e. whether the event should be allowed, modified, delayed or inhibited
 * and a list of actions, which should be executed.
**/
struct notifyResponse_s
{
  /// the event received for evaluation
  event_ptr               event;
  /// the collected authorization actions of all relevant mechanisms
  authorizationAction_ptr authorizationAction;
};

/**
 * @struct pdp_s
 * @brief Represents the PDP and its interfaces
 *
 * Provide interface methods of the PDP for management (start, stop), policy management (deployment, revocation, listing) and for event notifications.
 * These interface methods are set to stubs if the PDP is not initialized or stopped.
 * Additionally it contains internal hashtables for mechanisms, the action description store and a mutex for accessing these hashtables.
**/
struct pdp_s
{
  bool               (*start)();
  bool               (*stop)();
  unsigned int       (*revokeMechanism)(const char *mechName, const char *ns);
  unsigned int       (*revokePolicy)(const char *policyDocPath);
  unsigned int       (*deployMechanism)(const char *policyDocPath, const char *mechanismName);
  unsigned int       (*deployPolicy)(const char *policyDocPath);
  unsigned int       (*deployPolicyString)(const char *policy);
  char*              (*listDeployedMechanisms)();
  char*              (*notifyEventXML)(const char *eventDoc);
  notifyResponse_ptr (*notifyEvent)(event_ptr levent);

  /// Hashtable for installed mechanisms
  GHashTable          *mechanismTable;
  /// Hashtable for context descriptions referenced in installed policies
  GHashTable          *contextTable;
  /// Action Description Store for storing descriptions of system actions and their parameters
  actionDescStore_ptr actionDescStore;

  /// Hashtable for registered PEPs; used for subscribing actions
  GHashTable          *registeredPEPs;
  /// Hashtable for registered PXPs; used for executing actions
  GHashTable          *registeredPXPs;

  /// PDP synchronization mutex
  pefMutex_ptr        mutex;
};

#endif /* PDPTYPES_H_ */




