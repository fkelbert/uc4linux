/**
 * @file  paramDesc.c
 * @brief Implementation of parameter descriptions
 *
 * @see paramDesc.h, paramDescription_s

 * @author cornelius moucha
**/

#include "paramDesc.h"
#include "log_paramDesc_pef.h"

void paramDescriptionFree(gpointer data, gpointer userData)
{
  paramDescription_ptr actionParam=(paramDescription_ptr)data;
  if(actionParam==NULL) {log_error("Trying to free a NULL parameter description, aborting..."); return;}

  log_trace("Freeing action parameter description (%p)[%s]", actionParam->name, actionParam->name);
  free(actionParam->name);
  free(actionParam->type);

  g_slist_foreach(actionParam->values, (GFunc) paramValueFree, NULL);
  /// @todo can be done automatically with g_slist_free_full(GSLIST, GDestroyFunction)
  g_slist_free(actionParam->values);
  //g_slist_free_full(action_param->values, string_free);
  //g_slist_free_1(action_param->values);
  free(actionParam);
}

void paramDescriptionLog(gpointer key, gpointer value, gpointer userData)
{
  if(value==NULL) {log_error("Cannot log NULL value for paramDescription"); return;}
  paramDescription_ptr paramDesc=value;
  g_slist_foreach((GSList *)paramDesc->values,(GFunc)paramValueLog,"                        [%s]");
}

/// @todo separate return value for duplicate entry
unsigned int addParamValue(actionDescription_ptr actionDescription, const char *name, const char *value)
{
  checkNullInt(actionDescription, "Trying to add parameter value to NULL action description, aborting");
  checkNullInt(name,  "Trying to add parameter value to action description with NULL name, aborting");
  checkNullInt(value, "Trying to add NULL parameter value to action description, aborting");

  paramDescription_ptr paramDescription=g_hash_table_lookup(actionDescription->paramDescriptions, name);
  if(paramDescription==NULL)
  {
    log_warn( "Parameter description [%s] not found, can't add parameter value", name);
    return R_ERROR;
  }

  // Duplicates not allowed
  if(paramValueFind(paramDescription, value))
  {
    log_warn("Trying to add duplicate parameter value, aborting");
    return R_ERROR;
  }
  paramDescription->values=g_slist_append(paramDescription->values, strdup(value));
  log_trace("Adding value [%s] to parameter description [%s] of action [%s]", value, name, actionDescription->actionName);
  return R_SUCCESS;
}

void paramValueLog(gpointer value, gpointer userData)
{
  if(value==NULL || userData==NULL) {log_error("Cannot log NULL value for parameter value"); return;}
  log_trace(userData, (char *)value);
}

/// @todo add bool flag for automatic insertion!
char *paramValueFind(paramDescription_ptr paramDescription, const char *value)
{
  checkNullPtr(paramDescription, "Trying to find a value in a NULL parameter description, returning NULL");
  checkNullPtr(value,            "Searching for NULL param value, returning NULL");

  char *lvalue=NULL;
  lvalue=(char *)g_slist_find_custom(paramDescription->values, value, (GCompareFunc) strcmp);
  if(lvalue!=NULL) {log_trace("Param value [%s] found", value); return lvalue;}
  log_trace("Param value [%s] not found, returning NULL", value);
  return lvalue;
}

void paramValueFree(gpointer data, gpointer userData)
{
  char *value=(char *)data;
  if(value==NULL) {log_error("Trying to free a NULL string, aborting..."); return;}
  log_trace("Freeing action parameter value [%s]", value);
  free(value);
  return;
}


