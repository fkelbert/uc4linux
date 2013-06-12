/**
 * @file
 * @brief
 *
 * @author Ricardo Neisse
 **/
#include "action.h"
#include "event.h"
#include "log_event_test_c.h"

int main(int argc, char **argv) {

  // Store and action descriptions
  action_desc_store_ptr action_desc_store;
  action_desc_ptr open_desc, unlink_desc, action_desc;
  action_param_desc_ptr param_desc, param_desc2;
  char *param_value;
  
  // event
  event_ptr event, event1;
  event_trace_ptr event_trace;

  // Action description open
  open_desc = action_desc_new("open", ACTION_USAGE);
  add_param_desc(open_desc, "filename", "string");
  add_param_desc(open_desc, "username", "string");
  add_param_value(open_desc, "filename", "/etc/passwd");

  // Action description store
  action_desc_store = action_desc_store_new();
  add_action_desc(action_desc_store, open_desc);

  action_desc = find_action_desc_by_name(action_desc_store, "open");
  param_desc = find_param_desc_by_name(action_desc, "filename");
  param_desc2 = find_param_desc_by_name(action_desc, "username");
  
  event_match_op_ptr event_match;
  event_match = event_match_op_new(action_desc, MATCH_ALL, TRUE);
  event_match_add_param(event_match, param_desc, "\\etc\\passwd", FALSE);
  event_match_add_param(event_match, param_desc2, "neisse", FALSE);
  
  
  event = event_new(action_desc, IDX_START, TRUE);  
  event_add_param(event, param_desc, "\\etc\\passwd");  
  event_add_param(event, param_desc2, "neisse");  
  event_find_param_by_name(event, "filename");
  
  event_matches(event_match, event, NULL);

  event_match_op_free(event_match);
  
  event1 = event_new(action_desc, IDX_START, TRUE);  
  
  // event_log("An user custom message!", event);
  
  event_trace = event_trace_new();
  
  event_trace_add(event_trace, event);
  event_trace_add(event_trace, event1);
  
  //timestamp_ptr start_time = timestamp_new();
  
  discrete_trace_ptr discrete_trace = discrete_trace_new(0);
  
  timestep_window_ptr window=NULL;  
  
  // get window and not add
  window = discrete_trace_next_window(discrete_trace);
  timestep_window_add_event(window, event);
  
  window = discrete_trace_next_window(discrete_trace);  
  
  window = discrete_trace_next_window(discrete_trace);
  timestep_window_add_event(window, event1);  
 
  dicrete_trace_get_window(discrete_trace, 1);
  
  discrete_trace_free(discrete_trace);
  
  event_trace_free(event_trace);
 
  action_desc_store_free(action_desc_store);
}
