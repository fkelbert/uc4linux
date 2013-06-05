/*
 * Author: Ricardo Neisse
 */
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <fcntl.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>

#include <libxml/parser.h>
#include <libxml/xmlversion.h>
#include <libxml/xmlmemory.h>
#include <libxml/debugXML.h>
#include <libxml/xmlschemas.h>
#include <libxml/xmlschemastypes.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>

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


void
print_xpath_nodes(xmlNodeSetPtr nodes, FILE* output) {
    xmlNodePtr cur;
    int size;
    int i;
    
    assert(output);
    size = (nodes) ? nodes->nodeNr : 0;
    
    fprintf(output, "Result (%d nodes):\n", size);
    for(i = 0; i < size; ++i) {
	assert(nodes->nodeTab[i]);
	
	if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
	    xmlNsPtr ns;
	    
	    ns = (xmlNsPtr)nodes->nodeTab[i];
	    cur = (xmlNodePtr)ns->next;
	    if(cur->ns) { 
	        fprintf(output, "= namespace \"%s\"=\"%s\" for node %s:%s\n", 
		    ns->prefix, ns->href, cur->ns->href, cur->name);
	    } else {
	        fprintf(output, "= namespace \"%s\"=\"%s\" for node %s\n", 
		    ns->prefix, ns->href, cur->name);
	    }
	} else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
	    cur = nodes->nodeTab[i];   	    
	    if(cur->ns) { 
    	        fprintf(output, "= element node \"%s:%s\"\n", 
		    cur->ns->href, cur->name);
	    } else {
    	        fprintf(output, "= element node \"%s\"\n", 
		    cur->name);
	    }
	} else {
	    cur = nodes->nodeTab[i];    
	    fprintf(output, "= node \"%s\": type %d\n", cur->name, cur->type);
	}
    }
}

static void print_data (gchar *key, gchar *value) {
	printf ("%s:%s\n", key, value);
}

int main(int argc, char **argv) {
	char *service_uri = "http://127.0.0.1:8090/control_monitor_service";
	char *method = "push_event";
	SoupSession *session;
	GOptionContext *opts;
	GHashTable *g_event;
	GHashTable *g_action;
	GValue g_return_value;

	char *schemaFilename = "event.xsd";
    char *mechanismsFilename = "events.xml";
    xmlSchemaPtr schema;
    xmlDocPtr doc;
	xmlNode *rootElement;

	xmlNode *e_event;
	xmlNode *e_id;
	xmlNode *e_parameter;

    xmlNode *curNode;

    xmlLineNumbersDefault(1);
	doc = loadXmlDoc(mechanismsFilename, schemaFilename);
	if (doc==NULL)
		return(0);

	rootElement = xmlDocGetRootElement(doc);

	// xml_dump(rootElement);

	e_event = next_valid(rootElement->children);

//	return(0);

		/*
		if (strcmp(curNode->name, "event")==0) {
			plog(LOG_TRACE, "Found event");

			e_event = curNode;
			g_event = soup_value_hash_new();

			e_id = curNode->children;
			while(!strcmp(e_id->name, "id")==0) {
				e_id = e_id->next;
			}
			plog(LOG_TRACE,"%s",e_id->children->content);
			// plog(LOG_TRACE,"%s",e_id->children->content);
			// soup_value_hash_insert (g_event, "id", G_TYPE_STRING, e_id->children->content);


			e_parameter = e_id->next;
			while(!strcmp(e_parameter->name, "parameter")==0) {
				e_parameter = e_parameter->next;
			}
			plog(LOG_TRACE,"%s",e_parameter->children->content );

			soup_value_hash_insert (g_event, "user", G_TYPE_STRING, "neisse");
			g_hash_table_destroy (g_event);
		}
		curNode = curNode->next;
	}
			*/

	plog(LOG_DEBUG, "Initializing event_signaler");
	init_glib();
	g_set_prgname ("event_signaler");
	opts = g_option_context_new (NULL);

	plog(LOG_TRACE, "Creating SoupSession");
	session = soup_session_sync_new();

	g_event = soup_value_hash_new ();
	soup_value_hash_insert (g_event, "id", G_TYPE_STRING, "login");
	soup_value_hash_insert (g_event, "user", G_TYPE_STRING, "neisse");
	plog(LOG_TRACE, "Sending event to control monitor");
	if (!do_xmlrpc (session, service_uri, method, &g_return_value, G_TYPE_HASH_TABLE, g_event, G_TYPE_INVALID)) {
	    plog(LOG_ERROR, "Message not send ok");
	    exit(1);
	  }
	  g_hash_table_destroy (g_event);

	  log(LOG_TRACE, "Checking control monitor response");
	  if (!check_xmlrpc (&g_return_value, G_TYPE_HASH_TABLE, &g_action)) {
	    log(LOG_ERROR, "SOUP message result error");
	    exit(1);
	  }

	  g_hash_table_destroy (g_action);

	  return(0);

			
		    /*
		     * XPath search
		    xmlNode *curNode = NULL;
		    xmlXPathContextPtr xpathCtx; 
    		xmlXPathObjectPtr xpathObj; 
		    xpathCtx = xmlXPathNewContext(doc);
			if(xpathCtx == NULL) {
				log(LOG_ERROR,"Unable to create new XPath context");
			} else {
				if(xmlXPathRegisterNs(xpathCtx, "ns", "http://www.master-fp7.eu/Master-EnfLang-FHG") != 0) {
					log (LOG_ERROR, "Error: unable to register NS");
				} else {
					xpathObj = xmlXPathEvalExpression("//ns:observationMechanism", xpathCtx);					
					if(xpathObj == NULL) {
						log (LOG_ERROR, "Unable to evaluate xpath expression");
						xmlXPathFreeContext(xpathCtx); 
					} else {
						xmlNodeSetPtr nodes = xpathObj->nodesetval;
						//nodes = nodes->nodeTab[0]->children;
						int size = (nodes) ? nodes->nodeNr : 0;
						log (LOG_INFO, "Found %d control mechanisms", size);
						print_xpath_nodes(nodes, stdout);
					}
				}
			}
		    */
		    
		    
		    //for (curNode = rootElement; curNode; curNode = curNode->next) {
		    	//if (curNode->name)
				// if (urNode->type == XML_ELEMENT_NODE) {
				//	printf("node type: Element, name: %s\n", curNode->name);
				//}		    	
		    //}
			
			/*			
			e_mechanisms = xmlDocGetRootElement(doc);
			e_mechanism = e_mechanisms->children;
			// if(!strncmp(node->name,"mytag",5)
			// && node->ns
			// && !strcmp(node->ns->href,"http://www.mysite.com/myns/1.0")) {
			while (e_mechanism) {
				// go through all the mechanisms				
				// for each mechanism we have event, condition, and action
		        if (e_mechanism->type == XML_ELEMENT_NODE) {
		        	e_event = e_mechanism->children;
		        	do {
		        	// while(e_event!= XML_ELEMENT_NODE) e_event = e_event->next;
					// e_condition = e_event->next;
		        	//while(e_condition!= XML_ELEMENT_NODE) e_condition = e_condition->next;		        	
					//e_action = e_condition->next;				
		        	//while(e_action!= XML_ELEMENT_NODE) e_action = e_action->next;
					log(LOG_TRACE, "name: %s", e_event->name);				
					log(LOG_INFO, "content %s", e_event);
					//log(LOG_INFO, "formula", xmlNodeListGetString(doc, e_condition, 1));
		        	} while (e_event = e_event->next);
		        }
		        e_mechanism = e_mechanism->next;
			}
    }
    
   
    if (schema != NULL)
		xmlSchemaFree(schema);
    xmlSchemaCleanupTypes();
    xmlCleanupParser();
    xmlMemoryDump();
			*/

}




