/**
 * @file  mechanism.h
 * @brief Header for mechanism implementation
 *
 * Mechanisms refer to the main part of an OSL policy. They can either be a preventive mechanism or
 * a detective one. Both of them instantiate the ECA-principle: event, condition, action.
 * For detective mechanisms the event (the trigger) is NULL. The mentioned action may consist
 * of an authorization action (only for preventive mechanisms), which specify the intended
 * behaviour for the intercepted trigger event (allow, modify, delay or inhibit) in addition
 * to required execution action for this authorization decision. Furthermore the mechanism
 * may consist execution action independent of an authorization action, which should always
 * be executed when the condition is satisfied, e.g. for a notification message.
 *
 * @see authorizationAction.h, executionAction.h, condition.h
 *
 * @author cornelius moucha
 **/

#ifndef mechanism_h
#define mechanism_h

#include "pefEvent.h"
#include "pefEventMatch.h"
#include "condition.h"
#include "authorizationAction.h"

/******************************************************************************
 *  Mechanism definitions
******************************************************************************/
static char* const noNamespace="none";

/**
 * Creates a new mechanism based on the given arguments, i.e. the name and namespace, which
 * uniquely identifies this mechanism.
 * @param   name    mechanism name
 * @param   ns      mechanism namespace, e.g. the filename of the policy
 * @return  reference to created mechanism on success or NULL otherwise
**/
mechanism_ptr mechanismNew(const char *name, const char *ns);

/**
 * Deallocates an mechanism\n
 * Only intended for internal usage! This method is called when PDP is stopped or when the
 * mechanism/policy is revoked.
 *
 * @param   mech   mechanism for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  mechanismFree(mechanism_ptr mech);

/**
 * Profiling method calculating the allocated memory size of the given mechanism structure
 *
 * @param   mech   mechanism structure for calculation
 * @return  the size of the allocated memory for this mechanism
**/
unsigned long mechanismMemSize(const mechanism_ptr mech);

/**
 * Logging of a mechanism\n
 * @param   mech      reference to the mechanism for logging
**/
void          mechanismLog(mechanism_ptr mech);

/**
 * Parse a given XML document to instantiate a new mechanism
 * Internally called when a policy is deployed to the PDP
 *
 * @param   node  XML node for parsing
 * @param   ns    mechanism namespace
 * @return  reference to created mechanism on success or NULL otherwise
**/
mechanism_ptr mechanismParseXML(xmlNodePtr node, const char *ns);

/**
 * Calculate the mechanism's timestep size using the given amount and timeunit in the policy
 * @param   node  XML node for processing
 * @return  the mechanism's timestep size in microseconds
**/
uint64_t      xmlParseTimestepSize(const xmlNodePtr node);

/**
 * Adding an execution action to the mechanism\n
 * Internally called when parsing a mechanism in mechanismParseXML
 *
 * @param   mech    reference to the mechanism of this operation
 * @param   node    XML node for parsing
 * @param   name    name of the executeAction
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  mechanismAddExecuteAction(mechanism_ptr mech, xmlNodePtr node, const char *name);

/**
 * Serialization of a mechanism to an XML representation\n
 * Internally called when preparing the response of an event notification using the interface notifyEventXML,
 * which is based on pure XML communication
 *
 * @param   mech    reference to the mechanism of this operation
 * @return  the prepared XML document
**/
xmlDocPtr     mechanismSerializeToXML(const mechanism_ptr mech);

/**
 * Update a mechanism\n
 * Internally called from the mechanism own update thread to emulate the passing of time for
 * time-based evaluation, e.g. for temporal OSL operators
 *
 * @param   mechName  mechanism name
 * @param   mech      reference to the mechanism of this operation
 * @param   event     the event against which the mechanism should be updated; here a NULL event
 * @return  not used
**/
bool          mechanismUpdate(const char *mechName, mechanism_ptr mech, event_ptr event);

/**
 * Internal update thread of a mechanism\n
 * Emulate the passing of time for time-based evaluation, e.g. for temporal OSL operators, and triggers the mechanism updating
 * @param   mech      reference to the mechanism of this operation
**/
LPTHREAD_START_ROUTINE mechanismUpdateThread(void* mech);

#endif



