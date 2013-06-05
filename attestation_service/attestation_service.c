/*
 * Author: Ricardo Neisse
 */

#include <libsoup/soup.h>
#include <trousers/tss.h>
#include <trousers/trousers.h>

#include "logger.h"
#include "str_utils.h"
#include "time_utils.h"
#include "hash_table.h"
#include "crypto_utils.h"
#include "tss_utils.h"
#include "xml_utils.h"
#include "soup_utils.h"

void attestation_service  (SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data) {
  
  char *method_name;
  GValueArray *params;

  plog (LOG_INFO, "Acessing attestation_service");
  if (!msg_post(msg)) return;
  soup_message_set_status (msg, SOUP_STATUS_OK);  
  if (!parse_method_call(msg, &method_name, &params)) return;

  if (!strcmp (method_name, "remote_attestation")) {
    plog (LOG_INFO, "Invoking method remote_attestation()");
    do_remote_attestation (msg, params); 
  // } else if (!strcmp (method_name, "direct_anonymous_attestation")) {
  //    log (LOG_INFO, "Invoking method direct_anonymous_attestation()");
  //    do_direct_anonymous_attestation (msg, params); 
  } else {
    plog (LOG_TRACE, "Method attestation_service.%s() not implemented", method_name);
    soup_xmlrpc_set_fault (msg, SOUP_XMLRPC_FAULT_SERVER_ERROR_REQUESTED_METHOD_NOT_FOUND, "Unknown method %s", method_name);
  }
  g_free (method_name);
  g_value_array_free (params);
}

/**
 * Receives an array char* as  parameter
 * Each char* in the array is the name of one property to be attested
 */
int do_remote_attestation (SoupMessage *msg, GValueArray *params) {

  char *method_name, *val, *tmp_s;
  GValueArray *g_attestation_properties, *g_attestation_values;
  int i,k;
  TSS_RESULT tResult;
  TSS_HCONTEXT hContext;
  TSS_HTPM hTpm;
  struct pcr pcrOut;

  plog (LOG_TRACE, "Retrieving array parameter for method remote_attestation()");
  if (!soup_value_array_get_nth (params, 0, G_TYPE_VALUE_ARRAY, &g_attestation_properties)) {
    plog (LOG_ERROR, "Wrong parameter for method remote_attestation(array)", method_name);
    type_error (msg, G_TYPE_VALUE_ARRAY, params, 0);
    return;
  }

  g_attestation_values = g_value_array_new (g_attestation_properties->n_values);

  for (i = 0; i < g_attestation_properties->n_values; i++) {
    if (!soup_value_array_get_nth (g_attestation_properties, i, G_TYPE_STRING, &val)) {
      plog (LOG_ERROR, "Array item in parameter of method remote_attestation(array) is not of type String, aborting...", method_name);
      type_error (msg, G_TYPE_STRING, g_attestation_properties, i);
      return;
    }
    // val is the name of the property to be attested
    plog(LOG_DEBUG,"Requesting property value [%s]", val);
    if (!strcmp(val, "pcrs")) {
      
      plog(LOG_INFO,"Connecting to TPM/TSS");
      tResult = tss_connect(&hContext, &hTpm);  
      if (tResult!=TSS_SUCCESS) {
        plog(LOG_FATAL,"TPM/TSS API is not available, aborting...");
        continue;
      }            
      // There are 16 PCR registers in the TPM
      if (elog(LOG_INFO)) {
        for (i=0; i<16; i++) {
          pcrOut.index = i;
          tResult = TPM_PcrRead(hTpm, &pcrOut);
          if (tResult == TSS_SUCCESS) {
            // Allocate two chars per byte + 1 for NULL terminator            
            tmp_s = (char*) malloc(((pcrOut.valueLen)*2*sizeof(char))+1);
            //log(LOG_TRACE, "Allocating %d chars for hash value", ((pcrOut.valueLen)*2*sizeof(char))+1);
            for(k=0; k < pcrOut.valueLen; k++) {              
              snprintf((tmp_s+(k*2)), 3, "%02x", pcrOut.value[k]);
            }
            plog(LOG_TRACE, "Returning PCR[%02d][%s]",i,tmp_s);
            // printPcrHex(&pcrOut, 0, 1);
            soup_value_array_append (g_attestation_values, G_TYPE_STRING, tmp_s);
          } else {
            soup_value_array_append (g_attestation_values, G_TYPE_STRING, "-- Property value not available --");
          }
        }        
      }
    } else
      soup_value_array_append (g_attestation_values, G_TYPE_STRING, "-- Property value not available --");
      
  }  
  soup_xmlrpc_set_response (msg, G_TYPE_VALUE_ARRAY, g_attestation_values);
  g_value_array_free (g_attestation_values);  
}


static gboolean auth_callback (SoupAuthDomain *domain, SoupMessage *msg, const char *username, const char *password, gpointer user_data) {
  return TRUE;
}


GMainLoop *loop;

int main (int argc, char* argv) {
  SoupServer *server;
  SoupAuthDomain *domain;
  int port = 8080;
  
  plog(LOG_INFO, "Starting HTTP XML-RPC server [%d]", port);
  
  init_glib();  
  
  plog(LOG_TRACE, "Creating SoupServer");
  if (!(server = soup_server_new("port", port, NULL))) {
    plog(LOG_ERROR, "Unable to bind SoupServer to port %d", port);
    exit (1);
  }
  /*
  log(LOG_TRACE, "Creating authentication domain");
  domain = soup_auth_domain_basic_new (
    SOUP_AUTH_DOMAIN_REALM, "Attestation service",
    SOUP_AUTH_DOMAIN_BASIC_AUTH_CALLBACK, auth_callback,
    SOUP_AUTH_DOMAIN_BASIC_AUTH_DATA, auth_data,
    SOUP_AUTH_DOMAIN_ADD_PATH, "/attestation_service",
    NULL);
  soup_server_add_auth_domain (server, domain);
  */
  
  plog(LOG_INFO, "Adding [attestation_service] handler [/attestation_service]");
  soup_server_add_handler (server, "/attestation_service", attestation_service, NULL, NULL);
  // Run thread
  soup_server_run (server);
  g_object_unref (domain);     
}
