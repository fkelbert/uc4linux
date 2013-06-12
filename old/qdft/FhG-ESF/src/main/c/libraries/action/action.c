/**
 * @file
 * @brief
 *
 * @author Ricardo Neisse
 **/
#include "action.h"
#include "log_action_c.h"

action_desc_store_ptr action_desc_store_new() {
  action_desc_store_ptr action_desc_store = NULL;
  action_desc_store = (action_desc_store_ptr) malloc(sizeof(action_desc_store_t));
  // Whe should provide a function to free the action descriptions
  action_desc_store->action_descs = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) action_desc_free);
  return action_desc_store;
}

unsigned int add_action_desc(action_desc_store_ptr action_desc_store, action_desc_ptr action_desc) {
  if (action_desc_store == NULL) {
    log_error("Trying to add action description to a NULL store, aborting");
    return R_ERROR;
  }
  if (action_desc == NULL) {
    log_error("Trying to add NULL action description to a store, aborting");
    return R_ERROR;
  }
  // Duplicates not allowed
  if(find_action_desc_by_name(action_desc_store,action_desc->action_name)!=NULL) {
    log_error("Trying to add duplicate action description to a store, aborting");
    return R_ERROR;
  }
  log_trace("Adding action description [%s] to store", action_desc->action_name);
  g_hash_table_insert(action_desc_store->action_descs, action_desc->action_name, action_desc);
  return R_SUCCESS;
}

action_desc_ptr find_action_desc_by_name(action_desc_store_ptr action_desc_store, char *action_name) {
  action_desc_ptr action_desc = NULL;
  if (action_desc_store == NULL) {
    log_error("Trying to find an action description in a NULL store, returning NULL");
    return NULL;
  }
  if (action_name == NULL) {
    log_error("Trying to find a NULL action description in a store, returning NULL");
    return NULL;
  }
  action_desc = (action_desc_ptr) g_hash_table_lookup(action_desc_store->action_descs, action_name);
  if (action_desc == NULL) {
    log_trace("Action description [%s] not found, returning NULL", action_name);
  } else {
    log_trace("Action description [%s] found in store", action_name);
  }
  return action_desc;
}

action_desc_ptr find_action_desc_by_name_or_insert(action_desc_store_ptr action_desc_store, char *action_name)
{
  action_desc_ptr action_desc=NULL;
  if(action_desc_store == NULL)
  {
    log_error("Trying to find an action description in a NULL store, returning NULL");
    return NULL;
  }
  if(action_name == NULL)
  {
    log_error("Trying to find a NULL action description in a store, returning NULL");
    return NULL;
  }
  action_desc=(action_desc_ptr)g_hash_table_lookup(action_desc_store->action_descs, action_name);
  if(action_desc == NULL)
  {
    log_trace("Action description [%s] not found, inserting description", action_name);
    if(action_desc==NULL)
    {
      action_desc=action_desc_new(action_name, ACTION_USAGE);
      if(add_action_desc(action_desc_store, action_desc)!=R_SUCCESS)
      {
        log_error("Failure inserting action description; action_description could not be added to store!");
        return NULL;
      }
    }
  }
  else
  {
    log_trace("Action description [%s] found in store", action_name);
  }
  return action_desc;
}

action_param_desc_ptr find_param_desc_by_name(action_desc_ptr action_desc, const char *param_name) {
  action_param_desc_ptr param_desc = NULL;
  if (action_desc == NULL) {
    log_error("Trying to find a parameter description in a NULL action description, returning NULL");
    return NULL;
  }
  if (param_name == NULL) {
    log_error("Trying to find a NULL parameter description in a store, returning NULL");
    return NULL;
  }
  param_desc = (action_param_desc_ptr) g_hash_table_lookup(action_desc->param_descs, param_name);
  if (param_desc == NULL)
    log_trace("Param description [%s] not found, returning NULL", param_name);
  else
    log_trace("Param description [%s] found", param_name);
  return param_desc;
}

action_param_desc_ptr find_param_desc_by_name_or_insert(action_desc_ptr action_desc, const char *param_name)
{
  action_param_desc_ptr param_desc = NULL;
  if (action_desc == NULL) {
    log_error("Trying to find a parameter description in a NULL action description, returning NULL");
    return NULL;
  }
  if (param_name == NULL) {
    log_error("Trying to find a NULL parameter name in an action description, returning NULL");
    return NULL;
  }
  param_desc = (action_param_desc_ptr) g_hash_table_lookup(action_desc->param_descs, param_name);
  if(param_desc == NULL)
  {
    if(add_param_desc(action_desc, param_name, "string")==R_ERROR)
    {
      log_error("Failure adding param description!");
      return NULL;
    }
    param_desc=g_hash_table_lookup(action_desc->param_descs, param_name);
  }
  else log_trace("Param description [%s] found", param_name);
  return param_desc;
}

char *find_param_value(action_param_desc_ptr param_desc, char *value) {
  char *lvalue = NULL;
  if (param_desc == NULL) {
    log_error("Trying to find a value in a NULL parameter description, returning NULL");
    return NULL;
  }
  if (value == NULL) {
    log_error("Searching for NULL param value, returning NULL");
    return NULL;
  }
  lvalue = (char *) g_slist_find_custom(param_desc->values, value, (GCompareFunc) strcmp);
  //lvalue = (char *) g_slist_find_custom (param_desc->values, value, (GCompareFunc) g_ascii_strcasecmp);
  if (lvalue == NULL) {
    log_trace("Param value [%s] not found, returning NULL", value);
  } else {
    log_trace("Param value [%s] found", value);
  }
  return lvalue;
}

unsigned int action_desc_store_free(action_desc_store_ptr action_desc_store) {
  if (action_desc_store == NULL) {
    log_error("Trying to free a NULL action description store, aborting...");
    return R_ERROR;
  }
  log_trace("Freeing action description store");
  g_hash_table_destroy(action_desc_store->action_descs);
  free(action_desc_store);
  return R_SUCCESS;
}

void string_free(gpointer data, gpointer user_data) {
  char *value = (char *) data;
  if (value == NULL) {
    log_error("Trying to free a NULL string, aborting...");
    return;
  }
  log_trace("Freeing action parameter value [%s]", value);
  free(value);
  return;
}

void param_desc_free(gpointer data, gpointer user_data) {
  action_param_desc_ptr action_param = (action_param_desc_ptr) data;
  if (action_param == NULL) {
    log_error("Trying to free a NULL parameter description, aborting...");
    return;
  }
  log_trace("Freeing action parameter description [%s]", action_param->name);
  free(action_param->name);
  free(action_param->type);
  // we should free all values in the GSList
  g_slist_foreach(action_param->values, (GFunc) string_free, NULL);
  g_slist_free(action_param->values);
  free(action_param);
}

void action_desc_free(gpointer data, gpointer user_data)
{
  action_desc_ptr action_desc = (action_desc_ptr) data;
  if (action_desc == NULL)
  {
    log_error("Trying to free a NULL action description, aborting...");
    return;
  }
  log_trace("Freeing action description [%s]", action_desc->action_name);
  free(action_desc->action_name);
  g_hash_table_destroy(action_desc->param_descs);
  free(action_desc);
}

action_desc_ptr action_desc_new(char *action_name, unsigned int action_class) {
  log_trace("Creating action description [%s]", action_name);
  action_desc_ptr action_desc = (action_desc_ptr) malloc(sizeof(action_desc_t));
  if (action_name == NULL) {
    log_error("Action name must be set");
    return NULL;
  }
  if (action_desc == NULL) {
    log_error("Could not allocate memory for action description");
    return NULL;
  }
  action_desc->action_name = strdup(action_name);
  action_desc->action_class = action_class;
  // Whe should provide a function to free all the hash table items
  action_desc->param_descs = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) param_desc_free);

  action_desc->mechanisms = NULL;
  return action_desc;
}

void param_value_log(gpointer value, gpointer user_data) {
  log_trace(user_data, (char *) value);
}

void param_desc_log(gpointer key, gpointer value, gpointer user_data) {
  log_trace(user_data, (char *) key);
  action_param_desc_ptr param_desc = value;
  g_slist_foreach((GSList *) param_desc->values, (GFunc) param_value_log, "                        [%s]");
}

void action_desc_log(gpointer key, gpointer value, gpointer user_data) {
  action_desc_ptr action_desc = (action_desc_ptr) value;
  log_trace("=== Action description: [%s]", action_desc->action_name);
  log_trace("                 Class: [%s]", action_class_str[action_desc->action_class]);
  g_hash_table_foreach(action_desc->param_descs, (GHFunc) param_desc_log, "             Parameter: [%s]");
  log_trace("================================");
}

void action_desc_store_log(action_desc_store_ptr action_desc_store) {
  log_trace("=== Action description store:");
  g_hash_table_foreach(action_desc_store->action_descs, (GHFunc) action_desc_log, NULL);
  log_trace("================================");
}

/**
 * TODO: Check for existing/duplicate params
 */
unsigned int add_param_desc(action_desc_ptr action_desc, const char *name, char *type)
{
  if(action_desc == NULL)
  {
    log_error("Trying to add parameter description to NULL action description, aborting");
    return R_ERROR;
  }
  if (name == NULL) {
    log_error("Trying to add parameter description to action description with NULL name, aborting");
    return R_ERROR;
  }
  if (type == NULL) {
    log_error("Trying to add parameter description to action description with NULL type, aborting");
    return R_ERROR;
  }
  // Duplicates not allowed
  if(find_param_desc_by_name(action_desc,name))
  {
    log_error("Trying to add duplicate parameter description to action, aborting");
    return R_ERROR;
  }
  action_param_desc_ptr action_param = (action_param_desc_ptr) malloc(sizeof(action_param_desc_t));
  action_param->name = strdup(name);
  action_param->type = strdup(type);
  action_param->values = NULL;
  g_hash_table_insert(action_desc->param_descs, action_param->name, action_param);
  log_trace("Adding parameter description [%s] of type [%s] to action [%s]", name, type, action_desc->action_name);
  return R_SUCCESS;
}

/**
 * TODO: Check for existing values
 */
unsigned int add_param_value(action_desc_ptr action_desc, char *name, char *value) {
  if (action_desc == NULL) {
    log_error("Trying to add parameter value to NULL action description, aborting");
    return R_ERROR;
  }  
  if (name == NULL) {
    log_error("Trying to add parameter value to action description with NULL name, aborting");
    return R_ERROR;
  }  
  if (value == NULL) {
    log_error("Trying to add NULL parameter value to action description, aborting");
    return R_ERROR;
  }
  action_param_desc_ptr action_param = g_hash_table_lookup(action_desc->param_descs, name);
  if (action_param==NULL) {
    log_error("Parameter description [%s] not found, can't add parameter value", name);
    return R_ERROR;
  }
  // Duplicates not allowed
  if(find_param_value(action_param, value)) {
    log_error("Trying to add duplicate parameter value, aborting");
    return R_ERROR;
  }   
  action_param->values = g_slist_append(action_param->values, strdup(value));
  log_trace("Adding value [%s] to parameter description [%s] of action [%s]", value, name, action_desc->action_name);
  return R_SUCCESS;
}

