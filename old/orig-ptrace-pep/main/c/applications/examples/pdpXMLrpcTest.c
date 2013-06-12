/**
 * @file  pdpXMLrpcTest.c
 * @brief Exemplary client for communication with PDP over XML-RPC
 *
 * currently not working, needs to be updated!
 *
 * @author cornelius moucha
**/

#include "soup_utils.h"
#include "log_pdpXMLrpcTest_c.h"

int main()
{
  log_info("Starting PDP communication test using XML-RPC");

  SoupSession *session;
  char *service_uri = "http://127.0.0.1:8081/pdp";
  char *method = "deployMechanism";
  GValue g_return_value;

  g_thread_init(NULL);
  g_type_init();
  g_set_prgname("pdpXMLrpcTest");
  log_trace("Creating SoupSession");
  session = soup_session_sync_new();

  char *response;
  char *data="<?xml version='1.0'?><policySet xmlns='http://www.iese.fhg.de/esf/1.0/enforcementLanguage'><preventiveMechanism name='xx1'><description>xx1</description><timestep amount='1' unit='SECONDS'/><trigger action='open' isTry='true'></trigger><condition><XPathEval>/event/parameter[@name='command']/@value='cat'</XPathEval></condition><authorizationAction><inhibit/></authorizationAction></preventiveMechanism></policySet>";
  log_info("Deploying policy to PDP");
  if(!doXMLrpc(session, service_uri, method, &g_return_value, G_TYPE_STRING, data, G_TYPE_INVALID))
  {
    log_error("PDP not available, aborting...");
    exit(1);
  }

  log_info("Checking response from PDP");
  if(!checkXMLrpc(&g_return_value, G_TYPE_STRING, &response))
  {
    log_error("Protocol error in response from PDP, aborting...");
    exit(1);
  }

  log_info("Response from PDP=[%s]", response);

  char *method2 = "notifyEvent";
  char *data2="<?xml version='1.0' encoding='UTF-8'?><event isTry='true' action='open'><parameter name='command' value='cat'/></event>";
  log_info("Sending event to PDP");
  if(!doXMLrpc(session, service_uri, method2, &g_return_value, G_TYPE_STRING, data2, G_TYPE_INVALID))
  {
    log_error("PDP not available, aborting...");
    exit(1);
  }

  log_info("Checking response from PDP");
  if(!checkXMLrpc(&g_return_value, G_TYPE_STRING, &response))
  {
    log_error("Protocol error in response from PDP, aborting...");
    exit(1);
  }
  log_info("Response from PDP=[%s]", response);

  return 0;
}


