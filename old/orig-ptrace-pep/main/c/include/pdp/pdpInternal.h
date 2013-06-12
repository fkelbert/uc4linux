/**
 * @file  pdpInternal.h
 * @brief Generic definitions of pdp internal methods
 *
 * @author cornelius moucha
 **/

#ifndef PDPINTERNAL_H_
#define PDPINTERNAL_H_

#include <glib.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>
#include "mechanism.h"
#include "pdpTypes.h"

// internal PDP methods

/**
 * Installs a mechanism, representation as XML node, in the PDP
 * @param   node    mechanism representated as XML node
 * @param   ns      mechanism namespace, e.g. the filename of the policy
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int       mechanismInstall(xmlNodePtr node, const char *ns);

/**
 * Load a policy or mechanism from the given filename
 * @param   xmlFilename    filename of the policy
 * @param   mechanismName  mechanism name, which should be loaded from the policy (NULL => all mechanisms are loaded)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int       loadPolicy(const char *xmlFilename, const char *mechanismName);

/**
 * Load a policy or mechanism from the given string
 * @param   xml            xml string
 * @param   xmlSize        length of the xml string
 * @param   ns             namespace of the mechanisms
 * @param   mechanismName  mechanism name, which should be loaded from the policy (NULL => all mechanisms are loaded)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int       loadPolicyString(const char *xml, size_t xmlSize, const char *ns, const char *mechanismName);

/**
 * Internally iterates over all XML nodes, searching for "preventiveMechanism" and "detectiveMechanism"
 * @param   rootNode       XML root node
 * @param   mechanismName  mechanism name, which should be loaded from the policy (NULL => all mechanisms are loaded)
 * @param   ns             namespace of the mechanisms
 * @param   revoke         boolean flag, indicating whether a found mechanism should be installed or revoked
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int       searchMechanismNodes(xmlNodePtr rootNode, const char *mechanismName, const char *ns, bool revoke);

/**
 * Check whether the given event matches the eventMatch-operator in a mechanism's condition\n
 * Internally called for subscribed operator nodes to update the internal state
 * @param   curop   the condition operator (eventMatch-operator)
 * @param   levent  the intercepted event
**/
void               conditionTriggerEvent(operator_ptr curop, event_ptr levent);

/**
 * Check whether the given event triggers the mechanism
 * Internally called for subscribed mechanisms upon receiving an event
 * @param   mech      the subscribes mechanism
 * @param   response  the response for this notifyEvent invocation; collected from all subscribed mechanisms
**/
void               mechanismTriggerEvent(mechanism_ptr mech, notifyResponse_ptr response);

/**
 * Log a response for a notifyEvent invocation
 * @param   response  the response for this notifyEvent invocation; collected from all subscribed mechanisms
**/
void               notifyResponseLog(notifyResponse_ptr response);

/**
 * Serialize a response for a notifyEvent invocation to a string
 * @param   response  the response for this notifyEvent invocation; collected from all subscribed mechanisms
 * @return  the serialized notify resposnse
**/
char*              notifyResponseSerialize(notifyResponse_ptr response);

/**
 * Deallocate the memory consumed by a notifyResponse structure
 * @param   response  the response structure for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int       notifyResponseFree(notifyResponse_ptr response);

/**
 * Allocate a new response structure for a notifyEvent invocation
 * @param   levent   the event for which this response structure will be prepared
 * @return the prepared notifyResponse structure
**/
notifyResponse_ptr notifyResponseNew(event_ptr levent);

/**
 * Add the given mechanism to the notify response\n
 * If several mechanims are triggered from one event, their authorization action and executeActions have to be
 * combined for providing a response to the calling PEP. Therefore all top level executeActions are appended.
 * The authorization actions are combined in a way, that the more restrictive one wins, i.e. if the response was ALLOW
 * and the new mechanism require INHIBIT, then the final response will be INHIBIT. For delaying an event, the maximum
 * delay time will be returned. If the response already was INHIBIT and the new mechanism states ALLOW, it will be ignored!
 * This results in also ignoring potential execute actions for this allowing, because the event actually cannot be allowed
 * any more.
 * @remark Combining modifiers might raise a conflict, if two mechanism want to modify the same parameter differently!
 *         Currently all modifiers are simply appended without considering conflicts!
 * @param   response   the notifyResponse for adding this mechanism's information
 * @param   mech       the mechanism to add to the notifyResponse
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int       notifyResponseProcessMechanism(notifyResponse_ptr response, mechanism_ptr mech);

/**
 * Add the given modifier to the notify response\n
 * @remark Combining modifiers might raise a conflict, if two mechanism want to modify the same parameter differently!
 *         Currently all modifiers are simply appended without considering conflicts!
 * @param   curMech        the currently processed mechanism
 * @param   curMechAction  the current mechanism's authorization action
 * @param   name           modifier name
 * @param   value          modifier value
 * @param   isXPath        modifier flag for indicating xpath expression in value
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int       notifyResponseAddModifier(mechanism_ptr curMech, authorizationAction_ptr curMechAction,
                                             const char *name, const char *value, const char *isXPath);


#endif /* PDPINTERNAL_H_ */





