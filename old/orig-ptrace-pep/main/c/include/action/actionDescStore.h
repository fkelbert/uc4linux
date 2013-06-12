/**
 * @file    actionDescStore.h
 * @brief   Header for action description store
 *
 *
 * A central storage for action descriptions used in the context of the PDP
 * for referencing intercepted events. The action description store is used
 * for holding references to mechanisms subscribed to the appropriate actions.
 * Implemented using a hash table indexed by action name.
 *
 * @remark not thread safe => use mutex!
 * @see    action description in action.h
 *
 * @author  cornelius moucha
**/

#ifndef ACTIONDESCSTORE_H_
#define ACTIONDESCSTORE_H_

#include "action.h"

#ifdef  __cplusplus
  extern "C" {
#endif

// Forward type definitions
typedef struct actionDescStore_s  actionDescStore_t;
typedef        actionDescStore_t *actionDescStore_ptr;

/**
 * @struct actionDescStore_s
 * @brief  central storage for action descriptions
 *
 * The action description store is just a container for action descriptions with internal
 * methods for adding new descriptions and finding already stored ones.
**/
struct actionDescStore_s
{
  /// Hashtable of stored action descriptions; using glib2 implementation of hashtables
  GHashTable *actionDescriptions;
  /// Internal method for adding new description
  unsigned int           (*add)(actionDescStore_ptr actionDescStore, actionDescription_ptr actionDescription);
  /// Internal method for finding a stored description based on given name
  actionDescription_ptr (*find)(actionDescStore_ptr actionDescStore, const char *actionName, bool insert);
};

/**
 * Creates a new action description store
 *
 * @return  reference to created store on success or NULL otherwise
 */
actionDescStore_ptr actionDescriptionStoreNew();

/**
 * Deallocates an action description store
 *
 * @param   ads action description store for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int        actionDescriptionStoreFree(actionDescStore_ptr ads);

/**
 * Logging method for action description store
 * Internally logs all stored action descriptions including their parameters with LOG_TRACE
 *
 * @param   ads action description store for logging
 */
void                actionDescriptionStoreLog(actionDescStore_ptr ads);

/**
 * Adding action description to storage
 * Internally first checks for existing description to avoid duplicates
 *
 * @param   ads               action description store
 * @param   actionDescription action description for adding to store
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
 */
unsigned int          actionDescriptionAdd(actionDescStore_ptr ads, actionDescription_ptr actionDescription);

/**
 * Searching an action description using given action name.
 * A new action description is allocated if insert-flag is true
 *
 * @param   ads          action description store
 * @param   actionName   action description for searching in store
 * @param   insert       if true action description will be created if not present in store
 * @return  the found action description or NULL if none was found\n
 *          if insert was true returns the created action description
 */
actionDescription_ptr actionDescriptionFind(actionDescStore_ptr ads, const char *actionName, bool insert);


#ifdef  __cplusplus
  }
#endif

#endif /* ACTIONDESCSTORE_H_ */
