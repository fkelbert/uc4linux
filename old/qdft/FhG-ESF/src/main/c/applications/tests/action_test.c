/**
 * @file
 * @brief
 *
 * @author Ricardo Neisse
 **/
#include "action.h"
#include "log_action_test_c.h"

int main(int argc, char **argv) {

  // Store and action descriptions
  action_desc_store_ptr action_desc_store;
  action_desc_ptr open_desc, unlink_desc, action_desc;
  action_param_desc_ptr param_desc;
  char *param_value;

  // Action description open
  open_desc = action_desc_new("open", ACTION_USAGE);
  add_param_desc(open_desc, "filename", "string");
  add_param_value(open_desc, "filename", "/etc/passwd");
  add_param_value(open_desc, "filename", "/etc/passwd");
  add_param_value(open_desc, "filename", "/etc/tests");

  // Action description unlink
  unlink_desc = action_desc_new("unlink", ACTION_USAGE);

  // Action description store
  action_desc_store = action_desc_store_new();
  add_action_desc(NULL, NULL);
  add_action_desc(action_desc_store, NULL);
  add_action_desc(action_desc_store, open_desc);
  add_action_desc(action_desc_store, unlink_desc);

  action_desc_store_log(action_desc_store);

  action_desc = find_action_desc_by_name(action_desc_store, "read");

  action_desc = find_action_desc_by_name(action_desc_store, "open");
  if (action_desc != NULL)
  {
	  param_desc = find_param_desc_by_name(action_desc, "does_not_exist");
	  param_desc = find_param_desc_by_name(action_desc, "filename");
	  if (param_desc != NULL)
	  {
	    param_value = find_param_value(param_desc, "/etc/does_not_exist");
	    param_value = find_param_value(param_desc, "/etc/passwd");
	  }
  }


  action_desc_store_free(action_desc_store);
  return 0;
}
