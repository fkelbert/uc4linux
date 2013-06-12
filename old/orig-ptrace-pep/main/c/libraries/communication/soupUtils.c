/**
 * @file  soupUtils.c
 * @brief Implemention of utility methods for providing an interface for XML-RPC (using libsoup)
 *
 * @author Ricardo Neisse, cornelius moucha
 **/

#include "soupUtils.h"

// move to somewhere else (more globally as it's not related to soup!
void initGLib()
{
  log_trace("Initializing glib2");
  g_thread_init(NULL);
  g_type_init();
}

int msgPost(SoupMessage *msg)
{
  if(msg->method!=SOUP_METHOD_POST)
  {
    log_trace("Request is not POST, aborting...");
    soup_message_set_status(msg, SOUP_STATUS_NOT_IMPLEMENTED);
    return 0;
  }
  return 1;
}

int parseMethodCall(SoupMessage *msg, char **method_name, GValueArray **params)
{
  if(!soup_xmlrpc_parse_method_call(msg->request_body->data, msg->request_body->length, method_name, params))
  {
    soup_xmlrpc_set_fault(msg, SOUP_XMLRPC_FAULT_PARSE_ERROR_NOT_WELL_FORMED, "Could not parse method calls");
    log_trace("Could not parse method call, aborting.");
    return 0;
  }
  log_trace("Method call parsed %s", *method_name);
  return 1;
}

void argsError(SoupMessage *msg, GValueArray *params, int expected)
{
  log_error("Wrong number of parameters: expected %d, got %d", expected, params->n_values);
  soup_xmlrpc_set_fault(msg, SOUP_XMLRPC_FAULT_SERVER_ERROR_INVALID_METHOD_PARAMETERS, "Wrong number of parameters: expected %d, got %d", expected, params->n_values);
}

void typeError(SoupMessage *msg, GType expected, GValueArray *params, int bad_value)
{
  log_error("Bad parameter #%d: expected %s, got %s", bad_value + 1, g_type_name(expected), g_type_name(G_VALUE_TYPE (&params->values[bad_value])));

  soup_xmlrpc_set_fault(msg, SOUP_XMLRPC_FAULT_SERVER_ERROR_INVALID_METHOD_PARAMETERS, "Bad parameter #%d: expected %s, got %s",
      bad_value + 1, g_type_name(expected), g_type_name(G_VALUE_TYPE (&params->values[bad_value])));
}

gboolean doXMLrpc(SoupSession *session, char* uri, const char *method, GValue *retval, ...)
{
  SoupMessage *msg;
  va_list args;
  GValueArray *params=NULL;
  GError *err=NULL;
  char *body;

  log_trace("Processing XML-RPC message args");
  va_start (args, retval);
  params=soup_value_array_from_args(args);
  va_end (args);

  log_trace("Creating XML-RPC method call");
  body=soup_xmlrpc_build_method_call(method, params->values, params->n_values);
  g_value_array_free(params);
  if(!body)
  {
    log_error("SOAP message body error, aborting call");
    return FALSE;
  }

  log_trace("Creating XML-RPC message");
  msg=soup_message_new("POST", uri);

  log_trace("Setting XML-RPC request");
  soup_message_set_request(msg, "text/xml", SOUP_MEMORY_TAKE, body, strlen(body));

  log_trace("Sending XML-RPC message");
  soup_session_send_message(session, msg);

  if(!SOUP_STATUS_IS_SUCCESSFUL (msg->status_code))
  {
    log_error("Problems sending XML-RPC message [%d %s], aborting call", msg->status_code, msg->reason_phrase);
    g_object_unref(msg);
    return FALSE;
  }

  if(!soup_xmlrpc_parse_method_response(msg->response_body->data, msg->response_body->length, retval, &err))
  {
    if(err)
    {
      log_error(" Error parsing XML-RPC response [%d %s], aborting call", err->code, err->message);
      g_error_free(err);
    }
    else
    {
      log_error("Unknwon error parsing XML-RPC response, aborting call");
    }
    g_object_unref(msg);
    return FALSE;
  }
  g_object_unref(msg);
  return TRUE;
}

gboolean checkXMLrpc(GValue *value, GType type, ...)
{
  va_list args;
  if(!G_VALUE_HOLDS (value, type))
  {
    log_error("Checking of gvalue and type failed");
    g_value_unset(value);
    return FALSE;
  }
  va_start (args, type);
  SOUP_VALUE_GETV (value, type, args);
  va_end (args);
  return TRUE;
}

void runGLoop(GMainLoop *gloop)
{
  g_main_loop_run (gloop);
}

pefxmlrpc_ptr xmlrpcInit(char *service_name, int port, void *eventHandler)
{
  initGLib();
  pefxmlrpc_ptr lxmlrpc=(pefxmlrpc_ptr)memAlloc(sizeof(pefxmlrpc_t));
  checkNullPtr(lxmlrpc, "Could not allocate memory for XML-RPC server");
  log_info("Creating XML-RPC HTTP server port %d", port);

  lxmlrpc->server=NULL;
  if(!(lxmlrpc->server = soup_server_new("port", port, NULL)))
  {
    log_error("Unable to bind SoupServer to port %d", port);
    return NULL;
  }
  lxmlrpc->service_name=(char*)memCalloc(strlen(service_name)+1, sizeof(char));
  strcpy(lxmlrpc->service_name, service_name);

  log_info("Adding handler [%s]", service_name);
  soup_server_add_handler (lxmlrpc->server, service_name, eventHandler, NULL, NULL);
  lxmlrpc->gloop = g_main_loop_new (NULL, TRUE);

  soup_server_run_async(lxmlrpc->server);
  pthread_create(&lxmlrpc->gloop_thread, NULL, (void*)runGLoop, lxmlrpc->gloop);
  log_info("Successfully initialized soup server for service=[%s] on port=[%d]", service_name, port);
  return lxmlrpc;
}

unsigned int xmlrpcFree(pefxmlrpc_ptr xmlrpcInterface)
{
  log_debug("Stopping xml-rpc interface");
  soup_server_quit (xmlrpcInterface->server);
  //soup_server_disconnect (xmlrpcInterface->server);

  g_main_loop_quit (xmlrpcInterface->gloop);
  if(xmlrpcInterface->gloop_thread!=NULL)
  {
    log_debug("Trying to terminate gloop-thread");
    int ret=pthread_cancel(xmlrpcInterface->gloop_thread);
    if(ret!=0) perror("Error canceling thread!");
  }
  log_info("XML-RPC interface stopped");
  return R_SUCCESS;
}
