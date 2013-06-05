/*
 * Author: Ricardo Neisse
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <libconfig.h>
#include <libsoup/soup.h>
#include <libsoup/soup-types.h>

#include "logger.h"
#include "str_utils.h"
#include "time_utils.h"
#include "hash_table.h"
#include "crypto_utils.h"
#include "tss_utils.h"
#include "xml_utils.h"
#include "soup_utils.h"

/*
 * Default configuration settings
 */
char *CONF_FILE = "attestation_client.conf";
char *D_SERVICE_URI = "http://127.0.0.1:8080/attestation_service";

char *ATTESTATION_PROPERTIES[] = {"pcr1"};
#define N_ATTESTATION_PROPERTIES sizeof(ATTESTATION_PROPERTIES)/sizeof(char*)

int main (int argc, char* argv) {
  
  SoupSession *session;
  char *service_uri;
  int n_attestation_properties=0;
  char **attestation_properties;
  char *method = "remote_attestation";
  GOptionContext *opts;
  GValueArray *g_attestation_properties, *g_attestation_values;
  GValue g_return_value;
  int i;

  log(LOG_INFO, "Starting attestation client");

  struct config_t cfg;
  log(LOG_TRACE, "Initializing configuration");
  config_init(&cfg);

  log(LOG_TRACE, "Loading configuration file [%s]",CONF_FILE);
  if (!config_read_file(&cfg, CONF_FILE)) {
    log(LOG_ERROR, "Failed reading file %s Line:[%d] Error:[%s]",CONF_FILE, config_error_line(&cfg), config_error_text (&cfg));
    log(LOG_TRACE, "Using default configuration");
    
    service_uri = D_SERVICE_URI;
    
    n_attestation_properties = N_ATTESTATION_PROPERTIES;
    attestation_properties = (char**) malloc(n_attestation_properties*sizeof(char*));
    for (i = 0; i < n_attestation_properties; i++)
      attestation_properties[i] = ATTESTATION_PROPERTIES[i];
            
  } else {    
    
    config_setting_t *service_uri_cfg = NULL;
    service_uri_cfg = config_lookup(&cfg, "service_uri");
    if (!service_uri_cfg) {
      log(LOG_TRACE, "Configuration [service_uri] not found, using default [%s]", D_SERVICE_URI);
      service_uri = D_SERVICE_URI;
    } else {
      service_uri = (char *)config_setting_get_string(service_uri_cfg);
      log(LOG_TRACE, "Using configuration [service_uri][%s]", service_uri);
    }

    config_setting_t *attestation_properties_cfg = NULL;
    attestation_properties_cfg = config_lookup(&cfg, "attestation_properties");    
    if (attestation_properties_cfg && config_setting_is_array(attestation_properties_cfg)) {
      n_attestation_properties = config_setting_length(attestation_properties_cfg);
      plog(LOG_TRACE, "Using configuration attestation_properties[%d]",n_attestation_properties);
      attestation_properties = (char**) malloc(n_attestation_properties*sizeof(char*));
      for (i = 0; i < n_attestation_properties; i++) {
        attestation_properties[i] =  (char *)config_setting_get_string_elem(attestation_properties_cfg, i);
      }     
    } else {
      plog(LOG_TRACE, "Configuration [attestation_properties] not found, using default value");
      n_attestation_properties = N_ATTESTATION_PROPERTIES;
      attestation_properties = (char**) malloc(n_attestation_properties*sizeof(char*));
      for (i = 0; i < n_attestation_properties; i++)
        attestation_properties[i] = ATTESTATION_PROPERTIES[i];
    }
  }

  init_glib();  
  
  g_set_prgname ("soap_client");
  opts = g_option_context_new (NULL);
  
  log(LOG_TRACE, "Creating SoupSession");
  session = soup_session_sync_new();

  g_attestation_properties = g_value_array_new (n_attestation_properties);
  for (i = 0; i < n_attestation_properties; i++) {
    soup_value_array_append (g_attestation_properties, G_TYPE_STRING, attestation_properties[i]);
    plog(LOG_DEBUG, "Requesting attestation property: %s", g_value_get_string (&g_attestation_properties->values[i]));
  }

  plog(LOG_TRACE, "Sending attestation request");
  if (!do_xmlrpc (session, service_uri, method, &g_return_value, G_TYPE_VALUE_ARRAY, g_attestation_properties, G_TYPE_INVALID)) {
    g_value_array_free (g_attestation_properties);
    plog(LOG_ERROR, "Message not send ok");
    // return FALSE;
    exit(1);
  }
  g_value_array_free (g_attestation_properties);

  log(LOG_TRACE, "Checking attestation response");
  if (!check_xmlrpc (&g_return_value, G_TYPE_VALUE_ARRAY, &g_attestation_values)) {
    log(LOG_ERROR, "SOAP message result error");
    g_value_array_free (g_attestation_values);
    exit(1);
  }
  
  // if (g_attestation_values->n_values != n_attestation_properties) {
  // Not necessarly 1 to 1 match
  // Attestation format is PCRXXX or PCR to request all values
  // log (LOG_ERROR, "Number of attestation property values returned does not match request");
  for (i = 0; i < g_attestation_values->n_values; i++) {
      plog(LOG_DEBUG, "Attestation value: [%02d][%s]", i , g_value_get_string (&g_attestation_values->values[i]));
  }
    
  g_value_array_free (g_attestation_values);  
  config_destroy(&cfg);      
  free(attestation_properties);
}
