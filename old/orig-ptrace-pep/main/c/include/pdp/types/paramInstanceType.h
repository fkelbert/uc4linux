/**
 * @file  paramInstanceType.h
 * @brief Type definition header for parameter instances
 *
 * Provides type and structure definitions for parameter instances, i.e. parameters with a concrete value.
 *
 * @author cornelius moucha
 **/

#ifndef PARAMINSTANCETYPE_H_
#define PARAMINSTANCETYPE_H_

#include "base.h"
#include "paramDescType.h"

typedef struct parameterInstance_s    parameterInstance_t;
typedef        parameterInstance_t   *parameterInstance_ptr;

#define PARAM_INSTANCE_STRING 0
#define PARAM_INSTANCE_XPATH  1

/**
 * @struct parameterInstance_s
 * @brief Represents an authorization action as specified in the policy
 *
 * This structure represents a parameter instance, i.e. the parameter name and value.
 * In contrast to the generic parameter description in the action description store, this parameter instance refer
 * to a concrete value instead of a list of allowed parameter values.
**/
struct parameterInstance_s
{
  /// reference to the action description store, describing this parameter, e.g. the parameter name and list of allowed values
  paramDescription_ptr  paramDesc;
  /// concrete value of this parameter instance
  char                 *value;
  /// flag whether value contains xpath expression
  bool                  isXPath;
  /// type of this parameter instance; STRING or e.g. xpath
  unsigned int          type;
  /// evaluated value in case of type xpath
  char                 *evalValue;
};



#endif /* PARAMINSTANCETYPE_H_ */
