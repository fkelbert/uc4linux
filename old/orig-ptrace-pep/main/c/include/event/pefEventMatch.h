/**
 * @file    pefEventMatch.h
 * @brief   Header for event matching
 *
 * A eventMatch refers to the specification of an event in the policy, e.g. used for the
 * trigger event or a event matching operator in the condition. This datatype is not used for
 * events captured by the PEP.\n
 * When the PEP intercepts an system event and instantiates an appropriate event structure including
 * its parameters, this event is matched against its eventMatch equivalent specified in the policy.
 *
 * @see pefParamMatch.h, eventMatch_s
 *
 * @author cornelius moucha
 **/

#ifndef PEVEVENTMATCH_H_
#define PEVEVENTMATCH_H_

#include "pdpTypes.h"
#include "pdpInterface.h"
#include "eventTypes.h"
#include "actionDescStore.h"
#include "pefParamMatch.h"


/**
 * Creates a new eventMatch structure
 *
 * @param   matchAction action description referencing system action
 * @param   matchTry    TRUE if this eventMatch refers to a desired action or resp. try event
 * @return  reference to created eventMatch structure on success or NULL otherwise
 */
eventMatch_ptr  eventMatchNew(actionDescription_ptr matchAction, bool matchTry);

/**
 * Deallocates an eventMatch structure\n
 * This method deallocates the memory of an eventMatch structure. Intended for internal usage only when the mechanism
 * is deallocated!
 *
 * @param   matchEvent eventMatch structure for deallocation
 * @param   curMech    reference to the mechanism containing this eventMatch structure
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int    eventMatchFree(eventMatch_ptr matchEvent, mechanism_ptr curMech);

/**
 * Parse a given XML node to instantiate a new eventMatch structure
 *
 * @param   actionDescStore reference to the global action description store
 * @param   rootNode        XML node for parsing
 * @return  reference to created eventMatch structure on success or NULL otherwise
 */
eventMatch_ptr  eventMatchParseXML(actionDescStore_ptr actionDescStore, xmlNodePtr rootNode);

/**
 * Match an event against its specification equivalent\n
 * This method compares the referenced action and the event trial state
 * for the matching. Additionally matchEventParameters is called for matching all parameters specified in the
 * policy against the intercepted ones.
 *
 * @param   matchEvent eventMatch (i.e. trigger event or condition operator) specified in the policy
 * @param   event      event intercepted by the PEP and notified to the PDP for evaluation
 * @return  TRUE for matching of all parameters specified in the policy\n
 *          FALSE otherwise
 */
bool            eventMatches(eventMatch_ptr matchEvent, event_ptr event);

/**
 * Add a parameter to the eventMatch structure.\n
 * This method is internally called when the policy, resp. the eventMatch, is processed in eventMatchParseXML.
 *
 * @param   matchEvent       eventMatch (i.e. trigger event or condition operator) specified in the policy
 * @param   paramDescription reference to a description of the parameter in the action description store
 * @param   value            specified value in the policy.
 * @param   type             parameter type (e.g. PARAM_INT, PARAM_STRING, PARAM_REGEX)
 * @param   negate           flag whether the matching result should be negated
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int    eventMatchAddParam(eventMatch_ptr matchEvent, paramDescription_ptr paramDescription,
                                   char *value, unsigned int type, bool negate);

#endif /* PEVEVENTMATCH_H_ */




