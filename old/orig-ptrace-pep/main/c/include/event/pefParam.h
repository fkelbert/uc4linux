/**
 * @file  pefParam.h
 * @brief Header for event parameter processing
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

#ifndef PEFPARAM_H_
#define PEFPARAM_H_

#include <limits.h>
#include "eventTypes.h"

#ifdef  __cplusplus
  extern "C" {
#endif

/**
 * Add a parameter to the event structure.\n
 * This method is internally called when the event is processed in eventParseXML or manually
 * in case of native interface, where the PEP creates the event structure itself.\n
 * @remark  this is the general interface for adding a parameter, not intended for public use; instead use
 *          the methods for specific data types.
 *
 * @param   event       event for adding the parameter
 * @param   paramDesc   reference to a description of the parameter in the action description store
 * @param   paramType   parameter type (e.g. PARAM_INT, PARAM_STRING, PARAM_REGEX)
 * @param   paramValue  container for parameter value
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int  eventAddParam(event_ptr event, paramDescription_ptr paramDesc, unsigned int paramType, eventParamValue_ptr paramValue);

/**
 * Add an integer parameter to the event structure.\n
 * This method is internally called when the event is processed in eventParseXML or manually
 * in case of native interface, where the PEP creates the event structure itself.
 *
 * @param   event       event for adding the parameter
 * @param   paramName   parameter name
 * @param   value       parameter value
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int  eventAddParamInt(event_ptr event, const char *paramName, long value);

/**
 * Add an long parameter to the event structure.\n
 * This method is internally called when the event is processed in eventParseXML or manually
 * in case of native interface, where the PEP creates the event structure itself.
 *
 * @param   event       event for adding the parameter
 * @param   paramName   parameter name
 * @param   value       parameter value
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int  eventAddParamLong(event_ptr event, const char *paramName, int64_t value);

/**
 * Add an string parameter to the event structure.\n
 * This method is internally called when the event is processed in eventParseXML or manually
 * in case of native interface, where the PEP creates the event structure itself.
 *
 * @param   event       event for adding the parameter
 * @param   paramName   parameter name
 * @param   value       parameter value
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int  eventAddParamString(event_ptr event, const char *paramName, const char *value);

/**
 * Add an binary parameter to the event structure.\n
 * This method is internally called when the event is processed in eventParseXML or manually
 * in case of native interface, where the PEP creates the event structure itself.
 * @todo    not yet implemented
 *
 * @param   event       event for adding the parameter
 * @param   paramName   parameter name
 * @param   value       parameter value
 * @param   size        size of the binary data
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int  eventAddParamBinary(event_ptr event, const char *paramName, const void *value, unsigned long size);

/**
 * Add an string array parameter to the event structure.\n
 * This method is internally called when the event is processed in eventParseXML or manually
 * in case of native interface, where the PEP creates the event structure itself.
 * @todo    not yet implemented
 *
 * @param   event       event for adding the parameter
 * @param   paramName   parameter name
 * @param   value       parameter value
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int  eventAddParamStringArray(event_ptr event, const char *paramName, const char *value);


/**
 * Search for a parameter in the event\n
 *
 * @param   event     reference to the event for searching the parameter
 * @param   paramName name of parameter for searching
 * @return  the found parameter or NULL if none was found
 */
eventParam_ptr      eventFindParam(event_ptr event, char *paramName);

/**
 * Deallocates a eventParam structure\n
 * Only intended for internal usage! This method is called when the superior eventMatch structure is deallocated.
 *
 * @param   data      eventParam structure for deallocation
 * @param   userData  internal parameter of deallocation
 */
void                eventParamFree(gpointer data, gpointer userData);

/**
 * Logging of an event parameter \n
 * Only intended for internal usage! This method is called when the superior event is prepared for logging.
 *
 * @param   key       name of parameter
 * @param   value     reference to the parameter structure
 * @param   userData  internal parameter of g_hash_table_foreach
 */
void                eventParamLog(gpointer key, gpointer value, gpointer userData);


/**
 * Retrieve a parameter value from the event by the given parameter name\n
 *
 * @param   event       event for retrieving the parameter
 * @param   paramName   parameter name
 * @return  container for the event parameter
 */
eventParamValue_ptr eventGetParamValue(event_ptr event, char *paramName);

/**
 * Retrieve a string parameter from the event by the given parameter name\n
 *
 * @param   event       event for retrieving the parameter
 * @param   paramName   parameter name
 * @return  parameter value
 */
char               *eventGetParamValueString(event_ptr event, char *paramName);

/**
 * Retrieve a integer parameter from the event by the given parameter name\n
 *
 * @param   event       event for retrieving the parameter
 * @param   paramName   parameter name
 * @return  parameter value
 */
long                eventGetParamValueInt(event_ptr event, char *paramName);

/**
 * Retrieve a long parameter from the event by the given parameter name\n
 *
 * @param   event       event for retrieving the parameter
 * @param   paramName   parameter name
 * @return  parameter value
 */
long long           eventGetParamValueLong(event_ptr event, char *paramName);

/**
 * Retrieve a boolean parameter from the event by the given parameter name\n
 *
 * @param   event       event for retrieving the parameter
 * @param   paramName   parameter name
 * @return  parameter value
 */
int                 eventGetParamValueBool(event_ptr event, char *paramName);

#ifdef  __cplusplus
  }
#endif

#endif /* PEFPARAM_H_ */
