/*
 * Author: Ricardo Neisse
 *  
 */
#include <libsoup/soup.h>

#include "logger.h"
#include "str_utils.h"
#include "time_utils.h"
#include "hash_table.h"
#include "crypto_utils.h"
#include "tss_utils.h"
#include "xml_utils.h"
#include "soup_utils.h"

GMainLoop *loop;
char *service_name = "event_manager";

void event_manager (SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data) {
  char *method_name;
  GValueArray *params;
  
  log (LOG_INFO, "%s service", service_name);  
  
  if (!msg_post(msg)) return;  
  
  soup_message_set_status (msg, SOUP_STATUS_OK);  
  
  if (!parse_method_call(msg, &method_name, &params)) return;
       
  if (!strcmp (method_name, "push_event")) {
    log (LOG_INFO, "Invoking method %s()", method_name);
    do_push_event (msg, params);
  } else {
    log (LOG_TRACE, "Method %s.%s() not implemented", service_name, method_name);
    soup_xmlrpc_set_fault (msg, SOUP_XMLRPC_FAULT_SERVER_ERROR_REQUESTED_METHOD_NOT_FOUND, "Unknown method %s", method_name);
  }
  g_free (method_name);
  g_value_array_free (params);
}

/**
 * Receives an array char* as  parameter
 * Each char* in the array is the name of one property to be attested
 */
int do_push_event (SoupMessage *msg, GValueArray *params) {
  int i;
  char *event_name;
  GHashTable *event_params;

  // should receive 2 parameters
  if (params->n_values != 2) {
    args_error (msg, params, 2);
    return;
  }

  // first parameter is event name
  if (!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &event_name)) {
    type_error (msg, G_TYPE_STRING, params, 0);
    return;
  }

  // second parameter is hash_table with event parameters (pair: name, value)
  if (!soup_value_array_get_nth (params, 1, G_TYPE_HASH_TABLE, &event_params)) {
    type_error (msg, G_TYPE_HASH_TABLE, params, 1);
    return;
  }
  
  log(LOG_INFO, "Received event [%s]", event_name);
  
//  char *param_name, *param_value;
//  if (soup_value_hash_lookup (arg, param_name, G_TYPE_STRING, &param_value)) { }

  soup_xmlrpc_set_response (msg, G_TYPE_STRING, "success");
}

int main (int argc, char* argv) {
  SoupServer *server;
  char *service_name;
  int port = 8083;
  
  init_glib();  
  
  log(LOG_TRACE, "Creating HTTP SOAP server port %d", port);
  if (!(server = soup_server_new("port", port, NULL))) {
    log(LOG_ERROR, "Unable to bind SoupServer to port %d", port);
    exit (1);
  }
  
  service_name = "/event_manager";
  log(LOG_INFO, "Adding handler [%s]", service_name);
  soup_server_add_handler (server, service_name, event_manager, NULL, NULL);

  // Run thread
  soup_server_run (server);   
}
