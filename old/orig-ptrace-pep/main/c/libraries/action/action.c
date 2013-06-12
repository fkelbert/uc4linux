/**
 * @file    action.c
 * @brief   Implementation of action descriptions
 *
 * @see action.h, actionDescription_s
 *
 * @author cornelius moucha, Ricardo Neisse
 **/

#include "action.h"
#include "log_action_pef.h"

actionDescription_ptr actionDescriptionNew(const char *actionName, unsigned int actionClass)
{
  checkNullPtr(actionName, "Creating action description requires 'actionName' (given NULL)!");

  actionDescription_ptr actionDescription=(actionDescription_ptr) memAlloc(sizeof(actionDescription_t));
  checkNullPtr(actionDescription, "Could not allocate memory for action description");
  actionDescription->actionName=strdup(actionName);
  actionDescription->actionClass=actionClass;
  actionDescription->paramDescriptions=g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) paramDescriptionFree);

  actionDescription->mechanisms=NULL;
  actionDescription->conditions=NULL;
  actionDescription->pepInterface=pdpInterfaceStub;
  actionDescription->pxpInterface=pdpInterfaceStub;
  log_trace("Created action description [%s]", actionName);
  return actionDescription;
}

void actionDescriptionFree(gpointer data, gpointer userData)
{
  actionDescription_ptr actionDescription=(actionDescription_ptr) data;
  if(actionDescription==NULL) {log_error("Trying to free a NULL action description, aborting..."); return;}

  log_trace("Freeing action description [%s]", actionDescription->actionName);
  free(actionDescription->actionName);
  g_hash_table_destroy(actionDescription->paramDescriptions);

  // freeing subscribed mechanisms
  g_slist_free(actionDescription->mechanisms);

  // Deallocating interface
  //log_trace("Deallocating pepInterfaces");
  //if(actionDescription->pepInterface!=NULL) pdpInterfaceFree(actionDescription->pepInterface);

  //log_trace("Deallocating pxpInterfaces");
  //if(actionDescription->pxpInterface!=NULL) pdpInterfaceFree(actionDescription->pxpInterface);

  free(actionDescription);
}

void actionDescriptionLog(gpointer key, gpointer value, gpointer userData)
{
  actionDescription_ptr actionDescription=(actionDescription_ptr) value;
  log_trace("=== Action description: [%s]", actionDescription->actionName);
  log_trace("                 Class: [%s]", actionClassString[actionDescription->actionClass]);
  g_hash_table_foreach(actionDescription->paramDescriptions, (GHFunc)paramDescriptionLog, "             Parameter: [%s]");
  log_trace("================================");
}

/// @todo actionDescAddParam -> return paramDescription instead of R_SUCCESS/R_ERROR
unsigned int actionDescAddParam(actionDescription_ptr actionDescription, const char *name, const char *type)
{
  checkNullInt(actionDescription, "Trying to add parameter description to NULL action description, aborting");
  checkNullInt(name, "Trying to add parameter description to action description with NULL name, aborting");
  checkNullInt(type, "Trying to add parameter description to action description with NULL type, aborting");

  // Duplicates not allowed
  if(actionDescFindParam(actionDescription, name, FALSE))
  {
    log_warn( "Trying to add duplicate parameter description to action, aborting");
    return R_ERROR;
  }
  paramDescription_ptr actionParam=(paramDescription_ptr)memAlloc(sizeof(paramDescription_t));
  checkNullInt(actionParam, "Error allocating memory for paramDescription");
  actionParam->name=strdup(name);
  actionParam->type=strdup(type);
  actionParam->values=NULL;
  g_hash_table_insert(actionDescription->paramDescriptions, actionParam->name, actionParam);
  log_trace("Successfully added parameter description [%s] of type [%s] to action [%s]", name, type, actionDescription->actionName);
  return R_SUCCESS;
}

paramDescription_ptr actionDescFindParam(actionDescription_ptr actionDescription, const char *paramName, bool insert)
{
  checkNullPtr(actionDescription, "Trying to find a parameter description in a NULL action description, returning NULL");
  checkNullPtr(paramName,         "Trying to find a NULL parameter description in a store, returning NULL");

  paramDescription_ptr paramDescription=NULL;


  paramDescription=(paramDescription_ptr) g_hash_table_lookup(actionDescription->paramDescriptions, paramName);

  if(paramDescription!=NULL)
  {
    log_trace("Param description [%s] found", paramName);
    return paramDescription;
  }

  if(!insert)
  {
    log_trace("Param description [%s] not found, returning NULL", paramName);
    return paramDescription;
  }

  if(actionDescAddParam(actionDescription, paramName, "string")==R_ERROR)
  {
    log_warn("Failure adding param description!");
    return NULL;
  }
  paramDescription=g_hash_table_lookup(actionDescription->paramDescriptions, paramName);
  return paramDescription;
}






