/**
 * @file  pefEvent.h
 * @brief Header for event processing
 *
 * A event refers to a intercepted system action, which is captured by the PEP and notified to the
 * PDP. In contrast to an eventMatch this refers to a actual, concrete system action instead of its specification
 * equivalent in the policy.
 * When the PEP intercepts an system event and instantiates an appropriate event structure including
 * its parameters, this event is matched against its eventMatch equivalent specified in the policy.
 *
 * @see event_s
 *
 * @author cornelius moucha
 **/

#ifndef event_h
#define event_h

#include <glib.h>
#include <regex.h>

#include "memUtils.h"
#include "xmlUtils.h"
#include "eventTypes.h"
#include "actionDescStore.h"
#include "pefParam.h"

#if PDP_PIPSOCKET == 0
  #include "data_flow_monitor.h"
#endif

#ifdef  __cplusplus
  extern "C" {
#endif

/**
 * Creates a new event structure
 *
 * @param   actionDescription action description referencing system action
 * @param   isTry             TRUE if this event refers to a desired action or resp. try event
 * @return  reference to created event structure on success or NULL otherwise
 */
event_ptr     eventNew(actionDescription_ptr actionDescription, bool isTry);

/**
 * Deallocates an event structure\n
 * This method deallocates the memory of an event structure.\n
 * In case of a preallocation by the PEP (e.g. in case of native interface), the caller is
 * responsible for cleaning up the event. For other interfaces (e.g. JNI, sockets), the event structure
 * is internally allocated by the PDP and therefore automatically deallocated after its processing.
 *
 * @param   event   event structure for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int  eventFree(event_ptr event);

/**
 * Profiling method calculating the allocated memory size of the given event structure
 *
 * @param   event   event structure for calculation
 * @return  the size of the allocated memory for this event
 */
unsigned long eventGetMemsize(event_ptr event);

/**
 * Log the given event using log_trace
 *
 * @param   event   event structure for logging
 * @param   msg     message for logging prior the event
**/
void          eventLog(char *msg, event_ptr event);

/**
 * Create a XML representation of the given event. Involved if the condition contains a xpath expression,
 * which depends on an xml representation. The created xml structure is directly attached to the given event.
 *
 * @param   event   event structure for creating XML representation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int  eventCreateXMLdoc(event_ptr event);

/**
 * Create a XML representation of the given event parameters.\n
 * Called internally from eventCreateXMLdoc for all parameters present in the event
 *
 * @param   key       parameter name; not used here
 * @param   value     parameter for processing
 * @param   userData  reference to the XML node, this parameter should be attached to
 */
void          eventCreateXMLparams(gpointer key, gpointer value, gpointer userData);

/**
 * Parse a given XML document to instantiate a new event structure
 *
 * @param   actionDescStore reference to the global action description store
 * @param   xmlDoc          XML node for parsing
 * @return  reference to created event structure on success or NULL otherwise
 */
event_ptr     eventParseXML(actionDescStore_ptr actionDescStore, xmlDocPtr xmlDoc);

/**
 * Parse a given XML document to instantiate a new event structure\n
 * @remark modified version for usage in AAL context; they use <event topic="..." instead of
 *         <event action="..." due to the involvement of ActiveMQ as message broker
 *
 * @param   actionDescStore reference to the global action description store
 * @param   xmlDoc          XML node for parsing
 * @return  reference to created event structure on success or NULL otherwise
 */
event_ptr     eventParseXML_AAL(actionDescStore_ptr actionDescStore, xmlDocPtr xmlDoc);

#ifdef  __cplusplus
  }
#endif


#endif


