/**
 * @file  paramDesc.h
 * @brief Header for parameter descriptions
 *
 * @see paramDescription_s
 *
 * @author cornelius moucha
 **/

#ifndef PARAMDESC_H_
#define PARAMDESC_H_

#include "base.h"
#include "actionDescType.h"

#ifdef  __cplusplus
  extern "C" {
#endif


/**
 * Deallocates an parameter description\n
 * Only intended for internal usage! This method is called when the superior action description is deallocated.
 *
 * @param   data      parameter description for deallocation
 * @param   userData  internal parameter of hashtable deallocation
 */
void         paramDescriptionFree(gpointer data, gpointer userData);

/**
 * Logging of a parameter description\n
 * Only intended for internal usage! This method is called when the superior action description is logged.
 * Internally calls paramValueLog.
 *
 * @param   key       name of parameter description
 * @param   value     parameter description for logging
 * @param   userData  internal parameter of g_hash_table_foreach
 */
void         paramDescriptionLog(gpointer key, gpointer value, gpointer userData);

/**
 * Adding a parameter value to parameter description\n
 * Internally first checks for existing param value to avoid duplicates
 *
 * @param   actionDescription action description to whose parameter the value should be appended
 * @param   name              name of parameter
 * @param   value             value of parameter
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int addParamValue(actionDescription_ptr actionDescription, const char *name, const char *value);

/**
 * Searching a parameter value in the list of allowed parameter values.
 *
 * @param   paramDescription parameter description for searching in
 * @param   value            value to search
 * @return  the found parameter value or NULL if none was found
 */
char*        paramValueFind(paramDescription_ptr paramDescription, const char *value);

/**
 * Logging of an parameter value\n
 * Only intended for internal usage! This method is called when the superior parameter description is logged.
 *
 * @param   value     parameter value for logging
 * @param   userData  the text prefix for logging
 */
void         paramValueLog(gpointer value, gpointer userData);

/**
 * Deallocates an parameter value\n
 * Only intended for internal usage! This method is called when the superior parameter description is deallocated.
 *
 * @param   data      parameter value
 * @param   userData  internal parameter of glist deallocation
 */
void         paramValueFree(gpointer data, gpointer userData);


#ifdef  __cplusplus
  }
#endif



#endif /* PARAMDESC_H_ */
