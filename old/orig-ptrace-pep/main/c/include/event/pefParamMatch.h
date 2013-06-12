/**
 * @file    pefParamMatch.h
 * @brief   Header for parameter matching
 *
 * A paramMatch refers to the specification of a parameter in the policy, e.g. used in the
 * trigger event or a event matching operator in the condition. This datatype is not used for
 * events captured by the PEP.\n
 * When the PEP intercepts an system event and instantiates an appropriate event structure including
 * its parameters, these parameter instances are matched against their paramMatch equivalents specified
 * in the policy.
 *
 * @see pefEventMatch.h, paramMatch_s
 *
 * @author cornelius moucha
 **/

#ifndef PEFPARAMMATCH_H_
#define PEFPARAMMATCH_H_

#include "pdpOutPIP.h"
#include "eventTypes.h"
#include "xmlUtils.h"
#include "pefEvent.h"

/**
 * Deallocates a paramMatch structure\n
 * Only intended for internal usage! This method is called when the superior eventMatch structure is deallocated
 *
 * @param   data      paramMatch structure
 * @param   userData  internal parameter of deallocation
 */
void paramMatchFree(gpointer data, gpointer userData);

/**
 * Match event parameters against their specification equivalents\n
 * This method iterates over all parameters specified in the policy (NOT in the intercepted event!).\n
 * All of these specified parameters have to be present and match the values in the intercepted event
 * for a complete matching of the intercepted event. Each parameter (paramMatch) is matched using its
 * appropriate matching function, depending on the parameter type as specified in the policy
 *
 * @param   matchEvent eventMatch (i.e. trigger event or condition operator) specified in the policy
 * @param   event      event intercepted by the PEP and notified to the PDP for evaluation
 * @return  TRUE for matching of all parameters specified in the policy\n
 *          FALSE otherwise
 */
bool matchEventParameters(eventMatch_ptr matchEvent, event_ptr event);

/**
 * Match context parameters\n
 * Invokes a evaluation request to the PIP.
 *
 * @param   curEvent    event intercepted by the PEP and notified to the PDP for evaluation
 * @param   matchParam  concrete parameter specified in the policy, which has to be matched
 * @return  TRUE for parameter matching\n
 *          FALSE otherwise
**/
bool paramMatch_context(event_ptr curEvent, paramMatch_ptr matchParam);

/**
 * Match dataUsage parameters\n
 * This parameter type is used in case of an independent dataflow tracking system. In addition to the
 * PDP, the PIP also receives intercepted system events from the PEP and updates its internal dataflow model
 * for keeping track of any information flow within the system. For matching a dataUsage parameter
 * the PIP is invoked for evaluating, whether the current parameter refines the paramMatch specified in
 * the policy, i.e. whether the current data container (e.g. file, window, network buffer...) contains any data
 * from the initial container specified in the policy.\n
 * Invokes a evaluation request to the PIP.
 *
 * @param   curEvent    event intercepted by the PEP and notified to the PDP for evaluation
 * @param   matchParam  concrete parameter specified in the policy, which has to be matched
 * @return  TRUE for parameter matching\n
 *          FALSE otherwise
**/
bool paramMatch_dataUsage(event_ptr curEvent, paramMatch_ptr matchParam);

/**
 * Match regular expression\n
 * The actual parameter in the intercepted event is match against a regular expression specified in the
 * policy, e.g. the buffer from a network packet is matched against a specified pattern, indicating sensitive
 * content.
 *
 * @param   curEvent    event intercepted by the PEP and notified to the PDP for evaluation
 * @param   matchParam  concrete parameter specified in the policy, which has to be matched
 * @return  TRUE for parameter matching\n
 *          FALSE otherwise
**/
bool paramMatch_re(event_ptr curEvent, paramMatch_ptr matchParam);

/**
 * Match xpath parameters\n
 * The actual parameter in the intercepted event is match against a xpath expression specified in the
 * policy, e.g. the buffer from a network packet is matched against a specified xpath expression, indicating sensitive
 * content.
 * @remark  Depending on the used interface for notifying the PDP, this might require the instantiation
 *          of an xml structure representing the intercepted event, which results in performance decrease!
 *          For native or JNI calls, no xml representation is used in the event, therefore it has to be created!
 *
 * @param   curEvent    event intercepted by the PEP and notified to the PDP for evaluation
 * @param   matchParam  concrete parameter specified in the policy, which has to be matched
 * @return  TRUE for parameter matching\n
 *          FALSE otherwise
**/
bool paramMatch_xpath(event_ptr curEvent, paramMatch_ptr matchParam);

/**
 * Match string parameters\n
 * The actual parameter in the intercepted event is match against a string specified in the
 * policy.
 *
 * @param   curEvent    event intercepted by the PEP and notified to the PDP for evaluation
 * @param   matchParam  concrete parameter specified in the policy, which has to be matched
 * @return  TRUE for parameter matching\n
 *          FALSE otherwise
**/

bool paramMatch_compare(event_ptr curEvent, paramMatch_ptr matchParam);

/**
 * Match integer parameters\n
 * The actual parameter in the intercepted event is match against the integer value specified in the
 * policy. This method is used for all numerical values, i.e. integer, long and boolean.
 *
 * @param   curEvent    event intercepted by the PEP and notified to the PDP for evaluation
 * @param   matchParam  concrete parameter specified in the policy, which has to be matched
 * @return  TRUE for parameter matching\n
 *          FALSE otherwise
**/
bool paramMatch_int(event_ptr curEvent, paramMatch_ptr matchParam);


#endif /* PEFPARAMMATCH_H_ */




