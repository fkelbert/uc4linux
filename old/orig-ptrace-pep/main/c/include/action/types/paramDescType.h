/**
 * @file  paramDescType.h
 * @brief Header for data types related to parameter descriptions
 *
 * @author cornelius moucha
 **/

#ifndef PARAMDESCTYPE_H_
#define PARAMDESCTYPE_H_

#include <glib.h>

typedef struct paramDescription_s  paramDescription_t;
typedef        paramDescription_t *paramDescription_ptr;

/**
 * @struct paramDescription_s
 * @brief  structure for parameter description
 *
 * An parameter description is internally used for describing action parameters, e.g. the filename
 * of an open action in the system. Note: these parameter description refer to the specification of the
 * action parameters, not to the actual parameters of an intercepted action resp. event.
 * Therefore a parameter description consists of the parameter name, the parameter type (here is currently
 * only string used and supported), and ALL possible values for this parameter, indicating which parameter
 * values are allowed and which refer to an unintended system behaviour.
 * \n\n
 * For now this is not mandatory! The action descriptions are set up based on incoming events referencing
 * a new action. The same holds for the parameter descriptions and their values.
 *
 * @remarks the list of allowed parameter values is not used currently.
 * @see parameter descriptions of actual intercepted system action parameters: parameterInstance_s
 *
**/
struct paramDescription_s
{
  /// parameter name
  char    *name;
  /// parameter type (currently only "string" is used and therefore supported)
  char    *type;
  /// list of all allowed parameter values
  GSList  *values;
};

#endif /* PARAMDESCTYPE_H_ */
