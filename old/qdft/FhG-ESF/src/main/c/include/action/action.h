/**
 * @file action.h
 * @brief Header for actions
 *
 * @author Ricardo Neisse
 **/
#ifndef action_h
#define action_h

#include <string.h>
#include <glib.h>
#include "base.h"
#include "logger.h"

// Classes of actions
#define ACTION_USAGE     0
#define ACTION_SIGNALING 1
#define ACTION_OTHER     2

static char* const action_class_str[]={"usage", "signaling", "other"};

/**
 * An action parameter description, represents possible action parameters
 * including the possible values.
 */ 
typedef struct action_param_desc_s
{
  char *name;
  char *type;
  GSList *values;
} action_param_desc_t;
typedef action_param_desc_t *action_param_desc_ptr;

/**
 * An action description, represents a possible action in the system
 * including the possible values.
 */ 
typedef struct action_desc_s
{
  char* action_name;
  unsigned int action_class;
  GHashTable *param_descs;
  GSList *mechanisms;
} action_desc_t;
typedef action_desc_t *action_desc_ptr;

/**
 * An store for action descriptions, using
 * a hash table indexed by name (thread UNSAFE)
 */
typedef struct action_desc_store_s
{
  GHashTable *action_descs;
} action_desc_store_t;
typedef action_desc_store_t *action_desc_store_ptr;

void string_free(gpointer data, gpointer user_data);

char *find_param_value(action_param_desc_ptr param_desc, char *value);
void param_desc_log(gpointer key, gpointer value, gpointer user_data);
void param_value_log(gpointer value, gpointer user_data);
unsigned int add_param_value(action_desc_ptr action_desc, char *name, char *value);
void param_desc_free(gpointer data, gpointer user_data);

action_desc_store_ptr action_desc_store_new();
unsigned int add_action_desc(action_desc_store_ptr action_desc_store, action_desc_ptr action_desc);
action_param_desc_ptr find_param_desc_by_name(action_desc_ptr action_desc, const char *param_name);
action_param_desc_ptr find_param_desc_by_name_or_insert(action_desc_ptr action_desc, const char *param_name);
unsigned int add_param_desc(action_desc_ptr action_desc, const char *name, char *type);
void action_desc_log(gpointer key, gpointer value, gpointer user_data);
void action_desc_free(gpointer data, gpointer user_data);

action_desc_ptr action_desc_new(char *action_name, unsigned int action_class);
action_desc_ptr find_action_desc_by_name(action_desc_store_ptr action_desc_store, char *action_name);
action_desc_ptr find_action_desc_by_name_or_insert(action_desc_store_ptr action_desc_store, char *action_name);
void action_desc_store_log(action_desc_store_ptr action_desc_store);
unsigned int action_desc_store_free(action_desc_store_ptr action_desc_store);

#endif
