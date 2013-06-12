/*
 * event.c
 *
 *      Author: Ricardo Neisse
 */

#include "event.h"

void free_event(event_t* event) {
	int k;
//	plog(LOG_TRACE, "---- Event=%s, n_params=%i ", event->event_name, event->n_params);
	if (event!=NULL){
		if (event->event_name!=NULL) {
			free(event->event_name);

		}
		if (event->n_params != 0) {
			for (k = 0; k < event->n_params; k++) {
				//plog(LOG_TRACE, "---->>>>> param[%i]=%s value=%s", event->params[k].param_name,event->params[k].param_value);
				if (event->params[k].param_name != NULL)
					free(event->params[k].param_name);
				if (event->params[k].param_value != NULL)
					free(event->params[k].param_value);
				if (event->params[k].param_type != NULL)
					free(event->params[k].param_type);
			}
			if (event->n_params>0) {
				free(event->params);
			}
		}
		if(event->xml_doc!=NULL) {
			xmlFreeDoc(event->xml_doc);

		}

		free(event);

	}

}


/*
 * A is a refinement of B if
 * the number of parameters in A is the same or greater then in B and
 * all the names and values of the parameters in A are equal to the
 * names and values of the parameters in B
 */
/*
int A_is_refinement_of_B (event_t* A, event_t* B) {
	int k, j, c;
	c=0;
	char *tmpa, *tmpb;
	char *tmpcont;
	list_of_data_ptr lod;


	if (A->n_params >= B->n_params ) {
		for(k = 0; k < B->n_params; k++){
			for(j = 0; j < A->n_params; j++){
				if (strcmp(A->params[j].param_name, B->params[k].param_name) == 0) {
					if (strcmp(A->params[j].param_value, B->params[k].param_value) == 0) {
						c++;
					} else {
						//if B is a dataUsage event and A is a containerUsage event
						if (strcmp(A->params[j].param_name, "filename") == 0) {
							tmpa=strdup(A->params[j].param_value);
							tmpb=strdup(B->params[j].param_value);

							//Let's check if data stored in file A is B
							tmpcont=dataCont_getNaming(data_flow_monitor->map,"FNAME",tmpa);
							if (tmpcont==NULL) break; //no naming for A -> no data in the file

							lod=dataCont_getDataIn(data_flow_monitor->map, tmpcont);
							if (list_of_data_length(lod)==0) break; // no data in file A

							if (list_of_data_find(lod, tmpb)==0) {
								plog(LOG_TRACE, "    filename [%s] is a refinement of [%s]", tmpa, tmpb);
								c++;
							}

						} else {
							break;
						}

						break;
					}
				}
			}
		}
	}
	if (strcmp(A->event_name, B->event_name)==0 && c==B->n_params) {
		plog(LOG_TRACE, "    Event [%s] is a refinement of [%s]", A->event_name, B->event_name);
		return TRUE;
	}
	plog(LOG_TRACE, "    Event [%s] is NOT refinement of [%s]", A->event_name, B->event_name);
	return FALSE;
}
*/

// Create a XML representation of the event that
// is required for evaluation of XPath expressions
// We create a doc instance that is used by all monitor
// for xpath evaluation
void create_xml_doc(event_t* event) {
	// Document pointer
	xmlDocPtr xml_doc = NULL;
	// Node pointers
	xmlNodePtr event_node = NULL, id_node = NULL, param_node = NULL;
	int i;
	xml_doc = xmlNewDoc("1.0");
	event_node = xmlNewNode(NULL, "triggerEvent");
	xmlDocSetRootElement(xml_doc, event_node);
	id_node = xmlNewChild(event_node, NULL, "id", event->event_name);
	for (i=0; i < event->n_params; i++) {
		param_node = xmlNewChild(event_node, NULL, "parameter", NULL);
		xmlNewProp(param_node, "name", event->params[i].param_name);
		xmlNewProp(param_node, "value", event->params[i].param_value);
		xmlNewProp(param_node, "type", event->params[i].param_type);
	}
	event->xml_doc = xml_doc;
}

/*

  plog(LOG_TRACE,"A");
  event1 = (event_t*)malloc(sizeof(event_t));
  event1->event_name = "message";
  event1->desired = TRUE;
  event1->num_params = 4;
  event1->params = (param_t *)malloc(sizeof(param_t) * event1->num_params);
  event1->params[0].param_name = "a";
  event1->params[0].param_value = "aa";
  event1->params[1].param_name = "b";
  event1->params[1].param_value = "bb";
  event1->params[2].param_name = "c";
  event1->params[2].param_value = "cc";
  event1->params[3].param_name = "d";
  event1->params[3].param_value = "dd";

  event2 = (event_t*)malloc(sizeof(event_t));
  event2->event_name = "message";
  event2->desired = TRUE;
  event2->num_params = 4;
  event2->params = (param_t*)malloc(sizeof(param_t) * event2->num_params);
  event2->params[0].param_name = "a";
  event2->params[0].param_value = "aa";
  event2->params[1].param_name = "b";
  event2->params[1].param_value = "bb";
  event2->params[2].param_name = "c";
  event2->params[2].param_value = "cc";
  event2->params[3].param_name = "d";
  event2->params[3].param_value = "dd";

  plog(LOG_TRACE, " %d ", A_is_refinement_of_B(event1, event2));

 */
