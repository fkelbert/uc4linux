/*
 * soup_utils.c
 *
 *      Author: Ricardo Neisse
 */

#include "soup_utils.h"

#include <stdio.h>

int is_initialized = 0;

void init_glib() {
	if (!is_initialized) {
		plog(LOG_TRACE, "Initializing Glib");
		is_initialized = 1;
	}
}




int msg_post(SoupMessage *msg) {
  if (msg->method != SOUP_METHOD_POST) {
    plog (LOG_TRACE, "Request is not POST, aborting...");
    soup_message_set_status (msg, SOUP_STATUS_NOT_IMPLEMENTED);
    return 0;
  }
  plog (LOG_TRACE, "Request is POST");
  return 1;
}

int parse_method_call(SoupMessage *msg, char **method_name, GValueArray **params) {
  //plog (LOG_TRACE, "Body data \n\n[%s]\n\n", msg->request_body->data);
  if (!soup_xmlrpc_parse_method_call (msg->request_body->data, msg->request_body->length, method_name, params)) {  	  	
    soup_xmlrpc_set_fault (msg, SOUP_XMLRPC_FAULT_PARSE_ERROR_NOT_WELL_FORMED, "Could not parse method calls");    
    plog (LOG_TRACE, "Could not parse method call, aborting.");
    return 0;
  } 
  plog(LOG_TRACE, "Method call parsed %s", *method_name);
  return 1;
}  

void args_error (SoupMessage *msg, GValueArray *params, int expected) {
  plog (LOG_ERROR,
      "Wrong number of parameters: expected %d, got %d",
      expected, params->n_values);
  soup_xmlrpc_set_fault (msg,
      SOUP_XMLRPC_FAULT_SERVER_ERROR_INVALID_METHOD_PARAMETERS,
      "Wrong number of parameters: expected %d, got %d",
      expected, params->n_values);
}

void type_error (SoupMessage *msg, GType expected, GValueArray *params, int bad_value) {
  plog(LOG_ERROR, "Bad parameter #%d: expected %s, got %s",
      bad_value + 1, g_type_name (expected),
      g_type_name (G_VALUE_TYPE (&params->values[bad_value])));

  soup_xmlrpc_set_fault (msg,
      SOUP_XMLRPC_FAULT_SERVER_ERROR_INVALID_METHOD_PARAMETERS,
      "Bad parameter #%d: expected %s, got %s",
      bad_value + 1, g_type_name (expected),
      g_type_name (G_VALUE_TYPE (&params->values[bad_value])));
}

gboolean do_xmlrpc (SoupSession *session, char* uri, const char *method, GValue *retval, ...) {
  SoupMessage *msg;
  va_list args;
  GValueArray *params = NULL;
  GError *err = NULL;
  char *body;
  
  plog(LOG_TRACE, "Processing XML-RPC message args");
  va_start (args, retval);
  params = soup_value_array_from_args (args);
  va_end (args);

  plog(LOG_TRACE, "Creating XML-RPC method call");
  body = soup_xmlrpc_build_method_call (method, params->values, params->n_values);
  g_value_array_free (params);
  if (!body) {
    plog(LOG_ERROR, "SOAP message body error, aborting call");
    return FALSE;
  }
  
  plog(LOG_TRACE, "Creating XML-RPC message");
  msg = soup_message_new ("POST", uri);
  
  plog(LOG_TRACE, "Setting XML-RPC request");
  soup_message_set_request (msg, "text/xml", SOUP_MEMORY_TAKE, body, strlen (body));

  plog(LOG_TRACE, "Sending XML-RPC message");
  soup_session_send_message (session, msg);

  if (!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code)) {
    plog(LOG_ERROR, "Problems sending XML-RPC message [%d %s], aborting call", msg->status_code, msg->reason_phrase);
    g_object_unref (msg);
    return FALSE;
  }

  if (!soup_xmlrpc_parse_method_response (msg->response_body->data, msg->response_body->length, retval, &err)) {
    if (err) {
      plog (LOG_ERROR, " Error parsing XML-RPC response [%d %s], aborting call", err->code, err->message);
      g_error_free (err);
    } else {
      plog (LOG_ERROR, "Unknown error parsing XML-RPC response, aborting call");
    }
    g_object_unref (msg);
    return FALSE;
  }  
  g_object_unref (msg);
  return TRUE;
}

gboolean check_xmlrpc (GValue *value, GType type, ...) {
  va_list args;
  if (!G_VALUE_HOLDS (value, type)) {
    plog(LOG_ERROR, "Checking of gvalue and type failed");
    g_value_unset (value);
    return FALSE;
  }

  va_start (args, type);
  SOUP_VALUE_GETV (value, type, args);
  va_end (args);
  return TRUE;
}
