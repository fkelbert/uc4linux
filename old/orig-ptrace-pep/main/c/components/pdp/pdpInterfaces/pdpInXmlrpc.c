/**
 * @file  pdpInXmlrpc.c
 * @brief Implementation of PDP communication interface using XML-RPC
 *
 * Provide input interface for policy management (deployment) and for
 * event notifications.
 *
 * @todo pdpInXmlrpc not fully implemented and tested!
 *
 * @author cornelius moucha
 **/

#include "pdpCommInterfaces.h"

// PDP IN interface via XML-RPC
#if PDP_XMLRPC > 0
  pefxmlrpc_ptr pdpxmlrpc;

  void pdpXMLrpc_handler(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data)
  {
    char *method_name;
    GValueArray *params;

    log_info("PDP handler service");
    if(!msg_post(msg)) return;

    soup_message_set_status (msg, SOUP_STATUS_OK);
    if(!parse_method_call(msg, &method_name, &params)) return;

    if(strcmp(method_name, "notifyEvent")==0)
    {
      log_info("Invoking notifyEvent");
      char *eventString = NULL;
      if(!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &eventString))
      {
        type_error(msg, G_TYPE_STRING, params, 0);
        return;
      }
      unsigned char *response=pdpNotifyEventXML(eventString);
      soup_xmlrpc_set_response (msg, G_TYPE_STRING, response);
    }
    else if(strcmp(method_name, "deployMechanism") == 0)
    {
        log_info("Invoking deployMechanism");
        char *policyString = NULL;
        if(!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &policyString))
        {
          type_error(msg, G_TYPE_STRING, params, 0);
          return;
        }

        log_trace("Mechanism received via XMLRPC :\n%s", policyString);

        xmlDocPtr doc=loadXMLmemory(policyString, strlen(policyString));
        if(doc==NULL)
        {
          log_error("%s - Failure reading event!", __func__);
          soup_xmlrpc_set_response (msg, G_TYPE_STRING, "error");
        }
        xmlNodePtr rootNode=xmlDocGetRootElement(doc);
        searchMechanismNodes(rootNode, FALSE);
        log_info("Successfully loaded mechanisms from xmlrpc-call");
        xmlFreeDoc(doc);
        soup_xmlrpc_set_response (msg, G_TYPE_STRING, "success");
    }
    else
    {
      log_warn("Method event_handler.%s() not implemented", method_name);
      soup_xmlrpc_set_fault (msg, SOUP_XMLRPC_FAULT_SERVER_ERROR_REQUESTED_METHOD_NOT_FOUND, "Unknown method %s", method_name);
    }
    g_free (method_name);
    g_value_array_free (params);
  }
#endif // PDP_XMLRPC == 1
