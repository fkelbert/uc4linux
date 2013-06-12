/**
 * @file  eventTypes.h
 * @brief Type definition header for event related implementation
 *
 * Provides type and structure definitions for event, eventMatch and appropriate parameter types
 *
 * @author cornelius moucha
 **/

#ifndef EVENTTYPES_H_
#define EVENTTYPES_H_

#include <regex.h>
#include "xmlUtils.h"
#include "action.h"
#include "timestamp.h"

/// @todo validate that internally only STRING, INT, LONG, BINARY and StringArray are used!
#define PARAM_STRING      0   // no type or STRING
#define PARAM_DATAUSAGE   1   // DATAUSAGE
#define PARAM_XPATH       2   // XPATH
#define PARAM_REGEX       3   // RE or REGEX
#define PARAM_CONTEXT     4   // CONTEXT
#define PARAM_BINARY      5   // BINARY
#define PARAM_INT         6   // INT
#define PARAM_LONG        7   // LONG
#define PARAM_BOOL        8   // BOOL
#define PARAM_STRINGARRAY 9   // STRINGARRAY

// Forward declaration of structures and types
typedef struct event_s            event_t;
typedef        event_t           *event_ptr;
typedef struct eventParam_s       eventParam_t;
typedef        eventParam_t      *eventParam_ptr;
typedef union  eventParamValue_u  eventParamValue_t;
typedef        eventParamValue_t *eventParamValue_ptr;
typedef struct eventMatch_s       eventMatch_t;
typedef        eventMatch_t      *eventMatch_ptr;
typedef struct paramMatch_s       paramMatch_t;
typedef        paramMatch_t      *paramMatch_ptr;

/// static textual representation of paramter types; used for logging
static char* const paramTypeStr[] ={"string", "dataUsage", "xpath", "regex", "context", "binary", "int", "long", "bool", "stringArray"};


/**
 * @struct event_s
 * @brief Structure for event handling
 *
 * Structure event_s is used for handling events and stores their associated action name, parameters, mode information and
 * information about its try status (whether it is a desired event or not).\n
 * Additionally an XML document representation might be stored depending on the used creation interface for the event.\n
 * This structure is involved in the notification of intercepted events from the PEP. For specification events in the policy, i.e. eventMatch,
 * please refer to the structure eventMatch_s.
 *
**/
struct event_s
{
  /// Unique id of the event
  unsigned long long    id;
  /// Referenced action description
  actionDescription_ptr actionDesc;
  /// TRUE if desired action or try event
  bool                  isTry;
  /// Timestamp of event creation/interception
  timestamp_ptr         timestamp;
  /// Hashtable of event parameters
  GHashTable           *params;
  /// XML representation for XPath evaluation
  xmlDocPtr             xmlDoc;
};

/**
 * @struct eventParam_s
 * @brief Structure for parameter handling for events
 *
 * Structure eventParam_s is used for handling event parameters and store the parameter description, type and value.\n
 * This structure is involved with adding parameters to an event structure event_s. For specification parameters in the policy,
 * i.e. paramMatch, please refer to the structure paramMatch_s.
**/
struct eventParam_s
{
  /// reference to the parameter description providing information about parameter name and values
  paramDescription_ptr paramDesc;
  /// Defines data type of this parameter: PARAM_STRING, PARAM_INT, ...
  unsigned int         paramType;
  /// Container for this parameter; can be instantiated as string, integer, ... depending on current paramType
  eventParamValue_ptr  paramValue;
};

///////////////////////////////////////////////////////////////////////////////
// Type definitions for eventMatch, paramMatch
///////////////////////////////////////////////////////////////////////////////
/**
 * @union eventParamValue_u
 * @brief Union for parameter values
 *
 * This union provides a generic way of assigning a value to a parameter and can be instantiated as string, integer,
 * long, binary data and string array.
 * @todo should be optimized to also support combinations of parameter types, e.g. DATAUSAGE | INT, CONTEXT | INT
**/
union eventParamValue_u
{
  /// used for STRING, DATAUSAGE, XPATH, RE, CONTEXT
  char    *paramString;
  /// used for INT, BOOL (strtol returns long)
  long     paramInt;
  /// represents 64-bit long (int64_t or long long)
  int64_t  paramLong;
  /// not yet supported!
  void    *paramBinary;
  /// not yet supported!
  void    *paramStringArray;
};

/**
 * @struct eventMatch_s
 * @brief Structure for eventMatch handling
 *
 * Structure eventMatch_s is used for handling eventMatch operators and stores their associated action name, parameters, mode information and
 * information about its try status (whether it is a desired event or not).\n
 * This structure is involved when parsing the policy and instantiating internal structures. Therefore this structure represents the specified trigger event
 * from the policy and specified events in the condition part. For notification of events, i.e. actual happening events,
 * please refer to the structure event_s.
**/
struct eventMatch_s
{ /// Match action description
  actionDescription_ptr matchAction;
  /// Match try events
  bool                  matchTry;
  /// Match event parameters
  GHashTable           *matchParams;
};

/**
 * @struct paramMatch_s
 * @brief Structure for parameter handling for eventMatch
 *
 * Structure paramMatch_s is used for handling eventMatch parameters and store the parameter description, type and value. Furthermore
 * it contains a flag, whether the matching result should be negated for the evaluation and a matching function, which is dynamically
 * assigned depending on the parameter type.
 * Additionally a precompiled regular expression may be stored if the parameter type requires it.\n
 * This structure is involved with adding parameters to an eventMatch structure eventMatch_s. For notification event parameters,
 * please refer to the structure eventParam_s.
**/
struct paramMatch_s
{
  /// reference to the parameter description providing information about parameter name and values
  paramDescription_ptr paramDesc;
  /// Defines data type of this parameter: PARAM_STRING, PARAM_INT, ...
  unsigned int         type;
  /// Container for this parameter; can be instantiated as string, integer, ... depending on current paramType
  eventParamValue_ptr  value;
  /// precompiled regular expression, if required by parameter
  regex_t             *re;
  /// flag whether matching result should be negated for evaluation
  bool                 negate;
  /// dynamically assigned matching function, depends on parameter type
  bool                 (*match)(event_ptr curEvent, paramMatch_ptr paramMatch);
};


#endif /* EVENTTYPES_H_ */





