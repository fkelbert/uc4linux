/**
 * @file  action.h
 * @brief Header for action descriptions
 *
 * @see actionDescription_s
 *
 * @author cornelius moucha, Ricardo Neisse
 **/

#ifndef action_h
#define action_h

#include <string.h>
#include "memUtils.h"
#include "paramDesc.h"
#include "pdpInterface.h"

#ifdef  __cplusplus
  extern "C" {
#endif

/**
 * Creates a new action description\n
 * @param   actionName  action name referencing system action name
 * @param   actionClass action class (ACTION_USAGE, ACTION_SIGNALLING, ACTION_OTHER)
 * @return  reference to created action description on success or NULL otherwise
**/
actionDescription_ptr actionDescriptionNew(const char *actionName, unsigned int actionClass);

/**
 * Deallocates an action description\n
 * Only intended for internal usage! This method is called when action description store is deallocated.
 *
 * @param   data      action description for deallocation
 * @param   userData  internal parameter of hashtable deallocation
**/
void                  actionDescriptionFree(gpointer data, gpointer userData);

/**
 * Logging of an action description\n
 * Only intended for internal usage! This method is called when action description store is prepared for logging.
 * Internally calls paramDescriptionLog
 *
 * @param   key       name of action description
 * @param   value     action description for logging
 * @param   userData  internal parameter of g_hash_table_foreach
**/
void                  actionDescriptionLog(gpointer key, gpointer value, gpointer userData);

/**
 * Adding a parameter description to action description\n
 * Internally first checks for existing param description to avoid duplicates
 *
 * @param   actionDescription action description for adding
 * @param   name              name of parameter
 * @param   type              type of parameter, e.g. "string"
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int          actionDescAddParam(actionDescription_ptr actionDescription, const char *name, const char *type);

/**
 * Searching a parameter description using given parameter name.\n
 * A new parameter description is allocated if insert-flag is true
 *
 * @param   actionDescription action description for searching in
 * @param   paramName         name of parameter for searching
 * @param   insert            if true action description will be created if not present in store
 * @return  the found parameter description or NULL if none was found\n
 *          if insert was true returns the created action description
**/
paramDescription_ptr  actionDescFindParam(actionDescription_ptr actionDescription, const char *paramName, bool insert);


#ifdef  __cplusplus
  }
#endif

#endif






