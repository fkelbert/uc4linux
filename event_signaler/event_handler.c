/*
 * Author: Ricardo Neisse
 */

#include <pthread.h>
#include <libsoup/soup.h>

#include "logger.h"
#include "str_utils.h"
#include "time_utils.h"
#include "hash_table.h"
#include "crypto_utils.h"
#include "tss_utils.h"
#include "xml_utils.h"
#include "soup_utils.h"

void event_handler (SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data) {
  char *method_name;
  GValueArray *params;
  
  log (LOG_INFO, "Monitor manager service");  
  
  if (!msg_post(msg)) return;
  
  soup_message_set_status (msg, SOUP_STATUS_OK);  
  
  if (!parse_method_call(msg, &method_name, &params)) return;
     
  if (!strcmp (method_name, "attach_process")) {
    log (LOG_INFO, "Invoking method attach_process()");
    do_attach_process (msg, params);
  } else {
    log (LOG_TRACE, "Method monitor_manager.%s() not implemented", method_name);
    soup_xmlrpc_set_fault (msg, SOUP_XMLRPC_FAULT_SERVER_ERROR_REQUESTED_METHOD_NOT_FOUND, "Unknown method %s", method_name);
  }
  g_free (method_name);
  g_value_array_free (params);
}

/**
 * Receives an array char* as  parameter
 * Each char* in the array is the name of one property to be attested
 */
int do_attach_process (SoupMessage *msg, GValueArray *params) {
  int i;
  const char *val;
  char *in, *out;

  if (!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &in)) {
    type_error (msg, G_TYPE_STRING, params, 0);
    return;
  }
  log(LOG_INFO, "Attaching process [%s]",in);
  out = "success";
  soup_xmlrpc_set_response (msg, G_TYPE_STRING, out);
}


GMainLoop *gloop;

void run_gloop(void) {
    log(LOG_TRACE, "Starting gloop thread");
    g_main_loop_run (gloop);  
}

int main (int argc, char* argv) {  
  SoupServer *server;
  char *handler;
  int port = 8081;
  char *service_name = "/event_handler";
  pthread_t gloop_thread;
  
  log(LOG_INFO, "Starting event_handler");
  
  init_glib();  
  
  log(LOG_TRACE, "Creating HTTP SOAP server port %d", port);
  if (!(server = soup_server_new("port", port, NULL))) {
    log(LOG_ERROR, "Unable to bind SoupServer to port %d", port);
    exit (1);
  }
  
  log(LOG_INFO, "Adding handler [%s]", service_name);
  soup_server_add_handler (server, service_name, event_handler, NULL, NULL);
  
  gloop = g_main_loop_new (NULL, TRUE);

  soup_server_run_async (server);

  log(LOG_TRACE, "Starting gloop thread");
  pthread_create(&gloop_thread, NULL, run_gloop, NULL);  
   
  // event_handler call to push event in event_manager
  SoupSession *session;
  char *service_uri = "http://127.0.0.1:8083/event_manager";
  char *method = "push_event";
  GValue g_return_value;
  char *return_value;
  
  char *event_name;
  GHashTable *event_params;
  
  log(LOG_TRACE, "Creating SoupSession");
  session = soup_session_sync_new();

  event_name ="file_open";
  event_params = soup_value_hash_new ();
  soup_value_hash_insert (event_params, "process_name", G_TYPE_STRING, "bash");
  soup_value_hash_insert (event_params, "user_id", G_TYPE_INT, 0);
  soup_value_hash_insert (event_params, "user_login", G_TYPE_STRING, "root");
  soup_value_hash_insert (event_params, "file_name", G_TYPE_STRING, "\\etc\\passwd");
  log(LOG_TRACE, "Sending event to event_manager");
  if (!do_xmlrpc (session, service_uri, method, &g_return_value, 
      G_TYPE_STRING, event_name,
      G_TYPE_HASH_TABLE, event_params,
      G_TYPE_INVALID)) {
    log(LOG_ERROR, "Event not sent, aborting");
    exit(1);
  }  
  g_hash_table_destroy (event_params);

  log(LOG_TRACE, "Checking response from event_manager");
  if (!check_xmlrpc (&g_return_value, G_TYPE_STRING, &return_value)) {
    log(LOG_ERROR, "Response not ok, aborting");
    exit(1);
  }

  if (strcmp (return_value, "success")) {
    log(LOG_ERROR, "Response not ok, aborting", return_value);
    exit(1);
  }

  log(LOG_INFO, "Response is [%s]", return_value);
  
  int number;
  scanf("%d", &number);
  
  log(LOG_TRACE, "UNREF");
  g_main_loop_unref (gloop);    
}



