/**
 * @file    actionDescStore.c
 * @author  cornelius moucha, Ricardo Neisse
 * @brief   Implementation of action description store
 *
 * @see actionDescStore.h
 *
**/

#include "actionDescStore.h"
#include "log_actionDescStore_pef.h"

actionDescStore_ptr actionDescriptionStoreNew()
{
  actionDescStore_ptr actionDescStore=(actionDescStore_ptr)memAlloc(sizeof(actionDescStore_t));
  checkNullPtr(actionDescStore, "Error allocating memory for action description store");
  actionDescStore->actionDescriptions=g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) actionDescriptionFree);

  actionDescStore->add=actionDescriptionAdd;
  actionDescStore->find=actionDescriptionFind;
  return actionDescStore;
}

unsigned int actionDescriptionStoreFree(actionDescStore_ptr ads)
{
  checkNullInt(ads, "Trying to free a NULL action description store, aborting...");
  log_trace("Freeing action description store");

  g_hash_table_destroy(ads->actionDescriptions);
  free(ads);
  return R_SUCCESS;
}

unsigned int actionDescriptionAdd(actionDescStore_ptr ads, actionDescription_ptr actionDescription)
{
  checkNullInt(ads, "Trying to add action description to a NULL store, aborting");
  checkNullInt(actionDescription, "Trying to add NULL action description to a store, aborting");

  // Duplicates not allowed
  if(actionDescriptionFind(ads, actionDescription->actionName, FALSE)!=NULL)
  {
    log_warn("Trying to add duplicate action description to a store, aborting");
    return R_ERROR;
  }
  log_trace("Adding action description [%s] to store", actionDescription->actionName);
  g_hash_table_insert(ads->actionDescriptions, actionDescription->actionName, actionDescription);
  return R_SUCCESS;
}

actionDescription_ptr actionDescriptionFind(actionDescStore_ptr ads, const char *actionName, bool insert)
{
  checkNullPtr(ads, "Trying to search in a NULL action description store, aborting...");
  checkNullPtr(actionName, "Trying to find a NULL action description in a store, returning NULL");

  actionDescription_ptr actionDescription=(actionDescription_ptr)g_hash_table_lookup(ads->actionDescriptions, actionName);
  if(actionDescription!=NULL) {log_trace("Action description [%s] found in store", actionName); return actionDescription;}
  if(!insert) {log_trace("Action description [%s] not found, returning NULL", actionName); return actionDescription;}

  log_trace("Action description [%s] not found, inserting description", actionName);
  if(actionDescription==NULL)
  {
    actionDescription=actionDescriptionNew(actionName, ACTION_USAGE);
    if(actionDescriptionAdd(ads, actionDescription)!=R_SUCCESS)
    {
      log_warn( "Failure inserting action description; action_description could not be added to store!");
      return NULL;
    }
  }
  return actionDescription;
}

void actionDescriptionStoreLog(actionDescStore_ptr ads)
{
  if(ads==NULL) {log_warn("Logging NULL action description store not possible."); return;}
  log_trace("=== Action description store:");
  g_hash_table_foreach(ads->actionDescriptions, (GHFunc) actionDescriptionLog, NULL);
  log_trace("================================");
}





