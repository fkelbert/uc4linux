/**
 * @file  authorizationAction.h
 * @brief Header for authorizationAction implementation
 *
 * Authorization actions refer to the decision of the PDP for incoming events sent by the PEP.
 * In policies they specify what should happen with the intercepted event, i.e. whether it
 * should be allowed, modified, delayed or completely inhibited. Furthermore additional execution
 * actions might be required for an authorization action.
 *
 * @see executionAction.h
 *
 * @author cornelius moucha
 **/

#ifndef AUTHORIZATIONACTION_H_
#define AUTHORIZATIONACTION_H_

#include "xmlUtils.h"
#include "executeAction.h"
#include "actionDescStore.h"

/******************************************************************************
 *  authorizationAction definitions
******************************************************************************/
/**
 * Creates a new authorization action based on the given arguments, i.e. the allow/inhibit state and a potential delay
 * @param   name      the name of this authorization action, required for references with fallback
 * @param   response  the decision for the intercepted event (ACTION_ALLOW, ACTION_INHIBIT)
 * @param   delay     the amount for delaying the intercepted event in microseconds
 * @param   fallback  the fallback authorization action if specified in policy, otherwise implicit ACTION_INHIBIT
 * @return  reference to created authorization action on success or NULL otherwise
**/
authorizationAction_ptr authorizationActionNew(const char *name, unsigned int response, unsigned int delay, const char *fallback);

/**
 * Deallocates an authorization action\n
 * Only intended for internal usage! This method is called when the superior mechanism structure is deallocated.
 * @remark Although a reference to the authorization action is also contained in the notify response structure this should
 *         NOT be deallocated, because it is just a reference, not a copy!
 *
 * @param   authAction authorization action for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int            authorizationActionFree(authorizationAction_ptr authAction);

/**
 * Parse a given XML document to instantiate a new authorization action
 * Internally called when a policy is parsed using mechanismParseXML
 *
 * @param   mech  reference to the mechanism of this authorization action
 * @param   node  XML node for parsing
 * @return  reference to created authorization action on success or NULL otherwise
**/
authorizationAction_ptr authorizationActionParseXML(mechanism_ptr mech, const xmlNodePtr node);

authorizationAction_ptr authorizationActionProcessXMLnode(mechanism_ptr curMech, const xmlNodePtr node);

/**
 * Adding a modifier, i.e. a set of key-value pairs specifying the modification of a event parameter,
 * to the authorization action\n
 * Internally first checks for existing param description to avoid duplicates
 *
 * @param   mech        reference to the mechanism of this authorization action
 * @param   authAction  reference to the authorization action for the new modifier
 * @param   name        name of the parameter, which should be modified
 * @param   value       value of the modified parameter
 * @param   isXPath     flag indicating that the value is an xpath expression @deprecated isXpath will be replaced by type argument
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int            authorizationActionAddModifier(mechanism_ptr mech, authorizationAction_ptr authAction,
                                                       const char *name, const char *value , const char *isXPath);

/**
 * Profiling method calculating the allocated memory size of the given authorization action structure
 *
 * @param   authAction   authorization action structure for calculation
 * @return  the size of the allocated memory for this authorization action
**/
unsigned long           authorizationActionMemSize(const authorizationAction_ptr authAction);


#endif /* AUTHORIZATIONACTION_H_ */




