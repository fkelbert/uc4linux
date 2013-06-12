/*
 * Authors: Ricardo Neisse and Carolina Lorini.
 *
 */

/*
 * TODO: 
 *
 *  - Mechanism with NULL events are not supported,
 *    the past monitor should be extended to notify
 *    a possible mechanism associated with it so we
 *    can execute the appropriate actions without having
 *    a second update thread
 *
 *  - Parameters of an execute actions of a monitor might
 *    have a value copied from an XPath expression that
 *    references the trigger event
 *    if param->is_xpath then value = EvalXPath(value);
 *
 */

#include "event.h"
#include "hash_table.h"
#include "past_monitor.h"
#include "control_monitor.h"
#include "logger.h"
#include "libpbl/pbl.h"


table_t *mechanism_table;

xmlNode *currently_parsed_mechanism;

PblMap *mapDataToSetOfMechanisms;



// Thread mutex is defined in past_monitor file
// extern pthread_mutex_t mtx;
// extern struct timeval tv_timestep;

void reset_control_table() {
	init_table(&mechanism_table, MECHANISM_TABLE_SIZE);
}

int init_control_monitor() {
	plog(LOG_DEBUG, "Initializing control monitors");

	//initializes data structures
	if(init_table(&mechanism_table, MECHANISM_TABLE_SIZE)) {
		plog(LOG_ERROR, "Error initializing hash table");
		return R_ERROR;
	}

	mapDataToSetOfMechanisms = pblMapNewHashMap();
	if (mapDataToSetOfMechanisms == NULL) {
		plog(LOG_ERROR, "Error initializing hash table.");
		return R_ERROR;
	}


	//initializes past osl policy monitor
	if(!init_past_monitors()) {
		plog(LOG_ERROR, "Error initializing past monitors");
		return R_ERROR;
	}

	plog(LOG_TRACE, "Control monitors initialized");
	return R_SUCCESS;
}


int add_mechanism(char *id, event_t* trigger, char *condition, mechanism_actions_t *actions, int64_t usec_timestep) {
	entry_t *entry;
	mechanism_t* p_mechanism;
	event_t* p_trigger;
	mechanism_actions_t *p_actions;
	int n_actions, i ,j;

	plog(LOG_INFO, "- Adding new control mechanism [%s]", id);
	plog(LOG_INFO, "  trigger=[%s]", trigger->event_name);
	plog(LOG_INFO, "  condition=[%s]", condition);

	// pthread_mutex_lock(&mtx);

	//if id is not already in mechanism table, creates a new mechanism
	entry = lookup(&mechanism_table, id);
/*
	// create a new mechanism id, if a mechanism with this id has already been deployed
	nr = 1;
	while(entry) {
		tmpid = calloc(strlen(id)+12, sizeof(char));
		snprintf(tmpid, strlen(id)+12,"%s_%010d",id,nr);
		entry = lookup(&mechanism_table, tmpid);
		nr++;
	}

	id = tmpid;
*/
	if(!entry){
		//creates a new monitor for the mechanism condition
		int result = add_past_monitor(id, condition, usec_timestep);
		if(!result) {
			plog(LOG_INFO, "Error creating monitor for mechanism [%s]", id);
			// Monitor could not be created for mechanism condition
			return R_ERROR;
		}

		//allocates new mechanism structure
		p_mechanism = (mechanism_t*)malloc(sizeof(mechanism_t));

		//sets mechanism activation time
		p_mechanism->usec_start = usec_now();

		//sets mechanism state
		p_mechanism->is_active = FALSE;

		//copy id
		p_mechanism->id = strdup(id);

		//copy trigger
		if(trigger){
			p_trigger = (event_t *)malloc(sizeof(event_t));
			p_trigger->event_name = strdup(trigger->event_name);
			plog(LOG_INFO,"  trigger=[%s]", p_trigger->event_name );
			p_trigger->n_params = trigger->n_params;
			p_trigger->mode = trigger->mode;
			if(trigger->n_params > 0){
				p_trigger->params = (param_t *)malloc(sizeof(param_t) * trigger->n_params);
				for(i = 0; i < trigger->n_params; i++){
					p_trigger->params[i].param_name = strdup(trigger->params[i].param_name);
					p_trigger->params[i].param_type = strdup(trigger->params[i].param_type);

					fflush(NULL);

//					if (strcmp(trigger->params[i].param_type,"dataUsage")==0){
//						plog(LOG_INFO, " * Parameter [%s] of trigger event [%s] ",trigger->params[i].param_name,p_trigger->event_name);
//						plog(LOG_INFO, " * in mechanism [%s] is of type dataUsage",id );
//						plog(LOG_INFO, " * Storing initial mapping...");
//						tmpval=IF_initCont(trigger->params[i].param_value);
//						p_trigger->params[i].param_value = strdup(tmpval);
//					} else {

						p_trigger->params[i].param_value = strdup(trigger->params[i].param_value);
//					}
				}
			}
			p_mechanism->trigger = p_trigger;
		} else {
			p_mechanism->trigger = NULL;
		}

		// Copy condition
		p_mechanism->condition = strdup(condition);

		// Create mechanism actions
		p_mechanism->actions = (mechanism_actions_t *)malloc(sizeof(mechanism_actions_t));
		p_actions = p_mechanism->actions;

		p_mechanism->actions->response = actions->response;

		if (actions->response == ALLOW) {
			plog(LOG_INFO, "  action=[ALLOW]");
		} else {
			plog(LOG_INFO, "  action=[INHIBIT]");
		}

		// Copy mechanism delay
		p_mechanism->actions->delay = actions->delay;
		if (actions->delay > 0) {
			plog(LOG_INFO,"  action=[DELAY] duration=[%d]", actions->delay);
		}


		if(actions->modify){
			plog(LOG_INFO,"  action=[MODIFY] n_params=[%d]", actions->n_params);
			p_actions->n_params = actions->n_params;
			p_actions->modify = (param_t *)malloc(sizeof(param_t)*actions->n_params);
			for(i = 0; i < p_actions->n_params; i++){
				p_actions->modify[i].param_name = strdup(actions->modify[i].param_name);
				p_actions->modify[i].param_value = strdup(actions->modify[i].param_value);
				p_actions->modify[i].param_type = strdup(actions->modify[i].param_type);
			}
		} else {
			p_actions->n_params = 0;
			p_actions->modify = NULL;
		}

		// Copy actions to be executed
		p_actions->n_actions = actions->n_actions;
		n_actions = actions->n_actions;
		if(n_actions > 0){
			plog(LOG_INFO, "  action=[EXECUTE] n_actions=[%d]", actions->n_actions);
			p_actions->execute = (action_t *)malloc(sizeof(action_t)*n_actions);
			for(i = 0; i < n_actions; i++){
				p_actions->execute[i].id = strdup(actions->execute[i].id);
				p_actions->execute[i].type = actions->execute[i].type;
				p_actions->execute[i].component = actions->execute[i].component;
				p_actions->execute[i].n_params = actions->execute[i].n_params;
				//copy action parameters
				if(actions->execute[i].n_params > 0){
					p_actions->execute[i].params = (param_t *)malloc(sizeof(param_t) * actions->execute[i].n_params);
					for(j = 0; j < actions->execute[i].n_params; j++){
						p_actions->execute[i].params[j].param_name = strdup(actions->execute[i].params[j].param_name);
						p_actions->execute[i].params[j].param_value = strdup(actions->execute[i].params[j].param_value);
						p_actions->execute[i].params[j].param_type = strdup(actions->execute[i].params[j].param_type);
						p_actions->execute[i].params[j].is_xpath = actions->execute[i].params[j].is_xpath;
					}
				}
			}
		}

		//allocates new table entry
		entry = (entry_t *)malloc(sizeof(entry_t));
		entry->name = strdup(id);
		entry->value = p_mechanism;
		insert(&mechanism_table, entry);

		// pthread_mutex_unlock(&mtx);
		return R_SUCCESS;
	} else{
		// pthread_mutex_unlock(&mtx);
		return R_ERROR;
	}

	return R_SUCCESS;
}

void delete_mechanism(char *id) {
	entry_t *entry;
	mechanism_t *p_mechanism;
	int i,j;
	// Auxiliary variables
	event_t* trigger;
	mechanism_actions_t *actions;
	action_t *execute;

	plog(LOG_DEBUG, "Deleting mechanism [%s]", id);

	// pthread_mutex_lock(&mtx);

	entry = lookup(&mechanism_table, id);

	p_mechanism = (mechanism_t *)entry->value;

	if(entry) {
		p_mechanism->is_active = FALSE;

		delete_past_monitor(id);
		free(p_mechanism->id);

		//free trigger event
		if(p_mechanism->trigger){
			trigger = p_mechanism->trigger;
			if(trigger->n_params > 0){
				for(i = 0; i < trigger->n_params; i++){
					free(trigger->params[i].param_name);
					free(trigger->params[i].param_value);
					free(trigger->params[i].param_type);
				}
				free(trigger->params);
			}
			free(trigger->event_name);
			free(trigger);
		}

		//free condition
		free(p_mechanism->condition);

		//free list of actions
		actions = p_mechanism->actions;
		if(actions->n_actions > 0){
			for(i = 0; i < actions->n_actions; i++){
				execute = &(actions->execute[i]);
				free(execute->id);
				//free action parameters
				if(execute->n_params > 0){
					for(j = 0; j < execute->n_params; j++){
						free(execute->params[j].param_name);
						free(execute->params[j].param_value);
						free(trigger->params[i].param_type);
					}
					free(execute->params);
				}
			}
			free(actions->execute);
		}

		//free modify parameters
		if(actions->modify){
			for(i = 0; i < actions->n_params; i++){
				free(actions->modify[i].param_name);
				free(actions->modify[i].param_value);
				free(trigger->params[i].param_type);
			}
			free(p_mechanism->actions->modify);
		}

		// free action object
		free(p_mechanism->actions);

		// delete mechanism
		delete_entry(&mechanism_table,id);
	}

	// pthread_mutex_unlock(&mtx);
}


void delete_all_mechanisms() {
	entry_t *entry;
	int i;
	for(i = 0; i < mechanism_table->size_table; i++){
		entry = mechanism_table->hash[i];
		// plog(LOG_INFO, "Deleting mechanism %s", entry->name);
		delete_mechanism(entry->name);
	}
	plog(LOG_INFO, "All mechanisms deleted");
}

void reset_mechanism(char *id) {
	//if mechanism exists, resets mechanism condition
	// state and mechanism activation time
	entry_t *entry;
	plog(LOG_DEBUG, "Reseting mechanism [%s]", id);
	// pthread_mutex_lock(&mtx);
	entry = lookup(&mechanism_table,id);
	if(entry){
		disactivate_past_monitor(id);
		activate_past_monitor(id);
	}
	// pthread_mutex_unlock(&mtx);
}

void activate_mechanism(char *id) {
	entry_t *entry;
	mechanism_t *p_mechanism;
	plog(LOG_DEBUG, "Activating control mechanism id=[%s]", id);
	//pthread_mutex_lock(&mtx);
	entry = lookup(&mechanism_table, id);
	if(entry){
		p_mechanism = (mechanism_t *)entry->value;
		p_mechanism->is_active = TRUE;
		activate_past_monitor(id);
	}
	// pthread_mutex_unlock(&mtx);
}

void disactivate_mechanism(char *id) {
	entry_t *entry;
	mechanism_t *p_mechanism;
	plog(LOG_DEBUG, "Disactivating mechanism [%s]", id);
	// pthread_mutex_lock(&mtx);
	entry = lookup(&mechanism_table, id);
	if(entry){
		p_mechanism = (mechanism_t *)entry->value;
		p_mechanism->is_active = TRUE;
		disactivate_past_monitor(id);
	}
	// pthread_mutex_unlock(&mtx);
}

int lookup_mechanism(char *id) {
	entry_t *entry;
	mechanism_t *p_mechanism;
	plog(LOG_TRACE, "Looking up mechanism [%s]", id);
	entry = lookup(&mechanism_table, id);
	if(entry) {
		p_mechanism = (mechanism_t *)entry->value;
		return (p_mechanism->is_active);
	} else {
		return R_ERROR;
	}
}





mechanism_actions_t* new_event_request(event_t* desired_event) {
	entry_t *entry;
	mechanism_t *p_mechanism;
	int condition = FALSE;
	event_t* trigger;
	int i, k, j; //,k,j,x;

	// mechanism_actions_t* result=NULL;

	// TODO: add synchronization to some blocks considering that
	// mechanisms can be created through XML-RPC invocations

	// TODO: Each event can trigger only 1 mechanism, the first match,
	// how could we add combining algorithms or priorities to them?


	int IS_NULL_EVENT;
	if (desired_event==NULL) {
		IS_NULL_EVENT = TRUE;
	} else {
		IS_NULL_EVENT = FALSE;
	}

	if (IS_NULL_EVENT) {
		plog(LOG_DEBUG, " - NULL event received");
	} else {
		plog(LOG_DEBUG, " - New desired event received [%s]", desired_event->event_name);
	}
	// pthread_mutex_lock(&mtx);


	if(!mechanism_table->number_index){
		// If there are no loaded mechanisms, there is no action (default=ALLOW)
		// pthread_mutex_unlock(&mtx);
		return (NULL);
	}


	if(desired_event!=NULL) {

		// checks if event triggers any mechanisms
		for(i = 0; i < mechanism_table->size_table; i++){

			entry = mechanism_table->hash[i];

			// If mechanism is active

			if(entry){

				p_mechanism = (mechanism_t *)entry->value;
				trigger = p_mechanism->trigger;

				if (p_mechanism->is_active && trigger) {

					// Compare trigger event with requested event
					if ( A_is_refinement_of_B (desired_event, trigger) ) {

						// check if condition is satisfied if this event takes place
						desired_event->desired = TRUE;
						condition = update_past_monitor_by_id(p_mechanism->id, desired_event);

						if (condition) {
							plog(LOG_INFO, "  Condition TRUE -> Executing mechanism id=[%s]", p_mechanism->id);

							// the condition is satisfied, we need to see if action is allow
							// and in this case consider that the event really took place
							if (p_mechanism->actions->response==ALLOW) {
								desired_event->desired = FALSE;

								if (p_mechanism->actions->n_params > 0) {
									for(j = 0; j < p_mechanism->actions->n_params; j++) {
										for(k = 0; k < desired_event->n_params; k++) {
											if( strcmp(p_mechanism->actions->modify[j].param_name, desired_event->params[k].param_name)==0) {
												// match all modify parameters with the event parameters
												plog(LOG_DEBUG, " - Changing event parameter [%s] to [%s]", desired_event->params[k].param_name, p_mechanism->actions->modify[j].param_value);
												// free(desired_event->params[k].param_value);
												desired_event->params[k].param_value = strdup(p_mechanism->actions->modify[j].param_value);
												desired_event->params[k].param_type = strdup(p_mechanism->actions->modify[j].param_type);
											}
										}
									}
								}
								update_all_past_monitors(desired_event);
							}
							// return actions
							return p_mechanism->actions;
							break;
						} else {
							plog(LOG_INFO, "  Condition FALSE for mechanism id=[%s]", p_mechanism->id);
						}
					}

				}
			}
		}
	} else {
		// Received a NULL event
		// This will not happen, never!
		// checks if event triggers any mechanisms
		plog(LOG_TRACE, "   Searching for mechanisms with NULL trigger");
		for(i = 0; i < mechanism_table->size_table; i++){
			entry = mechanism_table->hash[i];
			if(entry){
				p_mechanism = (mechanism_t *)entry->value;
				if (p_mechanism->is_active) {
					//if mechanism is active, compare trigger event with requested event
					if(!p_mechanism->trigger){
						plog(LOG_TRACE, "  Found NULL trigger mechanism id=[%s]", p_mechanism->id);
						// Test mechanism condition
						// TODO: check condition
						// update_past_monitor(p_mechanism->id, NULL)
						if (TRUE) {
							return p_mechanism->actions;
						}
					} else {
						plog(LOG_TRACE, "  Mechanism [%s] does not match", p_mechanism->id);
					}
				}
			}
		}
		return NULL;
	}
	// pthread_mutex_unlock(&mtx);
	// No mechanism was triggered,
	// so update all monitors with event
	desired_event->desired = FALSE;
	update_all_past_monitors(desired_event);
	return NULL;
}

// Internal functions
mechanism_actions_t* execute_mechanism(char *id, event_t* desired_event) {
	/*
  entry_t *entry;
  mechanism_actions_t* response;
  int i,j;

 // log(LOG_DEBUG,"mechanism triggered");    

  //locates mechanism
  entry = m_lookup(mechanism_table,id);
  if(entry){
    response = (mechanism_actions_t *)malloc(sizeof(mechanism_actions_t));
    response->response = p_mechanism->actions.response;
    response->delay = p_mechanism->actions.delay;

    if((p_mechanism->actions.modify)&&((response->response == MODIFY)||(response->response == MODIFYANDDELAY))){
      response->n_params = p_mechanism->actions.n_params;
      response->modify = (param_t *)malloc(sizeof(param_t)*p_mechanism->actions.n_params);
      for(i = 0; i < response->n_params; i++){
        response->modify[i].param_name = (char *)malloc(strlen(p_mechanism->actions.modify[i].param_name));
        strcpy(response->modify[i].param_name,p_mechanism->actions.modify[i].param_name);
        response->modify[i].param_value = (char *)malloc(strlen(p_mechanism->actions.modify[i].param_value));
        strcpy(response->modify[i].param_value,p_mechanism->actions.modify[i].param_value);
      }
    }
    if(p_mechanism->actions.n_actions > 0){
      response->execute = (action_t *)malloc(sizeof(action_t)*p_mechanism->actions.n_actions);
      for(i = 0; i < p_mechanism->actions.n_actions; i++){   //runs through action list
        if(!p_mechanism->actions.execute[i].component){
          //action must be executed in the monitor            
          if(!strcmp(p_mechanism->actions.execute[i].id,"print")){
            if(p_mechanism->actions.execute[i].params[0].param_value)
              printf("%s\n",p_mechanism->actions.execute[i].params[0].param_value);
          }
          else if (!strcmp(p_mechanism->actions.execute[i].id,"delete")){
            if(p_mechanism->actions.execute[i].params[0].param_value)
              remove(p_mechanism->actions.execute[i].params[0].param_value);
          }
        }
        else {
          //action will not be executed in the monitor
          response->execute[response->n_actions].id = (char*)malloc(sizeof(p_mechanism->actions.execute[i].id));
          strcpy(response->execute[response->n_actions].id,p_mechanism->actions.execute[i].id);
          response->execute[response->n_actions].type = p_mechanism->actions.execute[i].type;
          response->execute[response->n_actions].component = p_mechanism->actions.execute[i].component;
          response->execute[response->n_actions].n_params = p_mechanism->actions.execute[i].n_params;
          //copy action parameters
          if(p_mechanism->actions.execute[i].n_params > 0){
            response->execute[response->n_actions].params = (action_param_t *)malloc(sizeof(action_param_t) * p_mechanism->actions.execute[i].n_params);
            for(j = 0; j < p_mechanism->actions.execute[i].n_params; j++){
              response->execute[response->n_actions].params[j].param_name = (char *)malloc(strlen(p_mechanism->actions.execute[i].params[j].param_name));
              strcpy(response->execute[response->n_actions].params[j].param_name,p_mechanism->actions.execute[i].params[j].param_name);
              response->execute[response->n_actions].params[j].param_value = (char *)malloc(strlen(p_mechanism->actions.execute[i].params[j].param_value));
              strcpy(response->execute[i].params[j].param_value,p_mechanism->actions.execute[i].params[j].param_value);
              response->execute[response->n_actions].params[j].xpath = p_mechanism->actions.execute[i].params[j].xpath;
            } //end for
          } //end if
          response->n_actions++;    
        }
      }  
    }
    else {
      response->n_actions = 0;
      response->execute = NULL;  
    }

    return (response);
  } else {
    return (NULL);
   }
	 */
	return NULL;
}

void plog_actions(int log_level, mechanism_actions_t *r_actions) {
	int i, k;
	action_t *execute;
	param_t *params;
	switch (r_actions->response) {

	case ALLOW:
		plog(log_level, "  Response is ALLOW");
		if (r_actions->n_params == 0) {
			plog(log_level, "  - No parameters should be modified");
		} else {
			plog(log_level, "  - Parameters should be modified [%d]", r_actions->n_params);
			params = r_actions->modify;
			for (k=0 ; k < r_actions->n_params; k++) {
				plog(log_level, "    Parameter [%s]=[%s]", r_actions->modify[k].param_name, r_actions->modify[k].param_value);
			}
		}
		break;

	case INHIBIT:
		plog(log_level, "Response is INHIBIT");
		break;
	}
	if (r_actions->n_actions == 0) {
		plog(log_level, "  - No additional actions should be executed");
	} else {
		plog(log_level, "  - Additional actions to be executed:");
		execute = r_actions->execute;
		for (i=0 ; i < r_actions->n_actions; i++ ) {
			plog(log_level, "  - Action [%s]", execute[i].id);
			params = execute[i].params;
			for (k=0 ; k < execute[i].n_params; k++) {
				plog(log_level, "    Parameter [%s]=[%s]", params[k].param_name, params[k].param_value);
			}
		}

	}

}

void parseEvent(xmlNode * node1, event_t **a_event) {
	xmlNode * node2;
	event_t *event = (event_t*)malloc(sizeof(event_t));
	event->n_params=0;
	// plog(LOG_TRACE," Parsing event");
	for (node2 = node1->children; node2; node2 = node2->next) {
		if (strcmp(node2->name, "id")==0) {
			event->event_name = getText(node2);
			// plog(LOG_TRACE,"  Event is [%s]", event->event_name);
		}
	}
	event->n_params = parseParams(node1, &(event->params));
	*a_event = event;
}

int parseParams(xmlNode * node, param_t **p_params) {
	xmlNode * node1;
	param_t *params;
	xmlChar *value;
	char* tmpval;
	int n_params=0, i=0;
	for (node1 = node->children; node1; node1 = node1->next) {
		if (strcmp(node1->name, "parameter")==0) {
			n_params++;
		}
	}
	params = NULL;
	// plog(LOG_TRACE,"  Number of params %d", n_params);
	if (n_params > 0) {
		i=0;
		params = (param_t *)malloc(sizeof(param_t) * n_params);
		for (node1 = node->children; node1; node1 = node1->next) {
			if (strcmp(node1->name, "parameter")==0) {
				xmlAttrPtr propPtr;
				propPtr = node1->properties;
				params[i].param_type = NULL;
				while( propPtr ) {
					value = xmlGetProp(node1, propPtr->name);
					// plog(LOG_TRACE," Param value = %s", value);
					if (strcmp(propPtr->name, "name") == 0) {
						params[i].param_name = strdup(value);
					} else if (strcmp(propPtr->name, "value")==0) {
						params[i].param_value = strdup(value);
					} else if (strcmp(propPtr->name, "type")==0) {
						params[i].param_type = strdup(value);
					}
					xmlFree(value);
					propPtr = propPtr->next;
				}

				if (params[i].param_type == NULL) {
					params[i].param_type = strdup("contUsage");
				}
				if (strstr(params[i].param_type,"dataUsage") == params[i].param_type) {
					if (strcmp(params[i].param_type,"dataUsage")==0) {
						tmpval=IF_initCont(params[i].param_value);

						unsigned char* new_type = calloc(100, sizeof(char));
						snprintf(new_type,100,"dataUsage[%s]",tmpval);

						params[i].param_type = strdup(new_type);
						xmlSetProp(node1, "type", new_type);

						params[i].param_value = strdup(tmpval);
						xmlSetProp(node1, "value", tmpval);

						mapDataToMechanism(tmpval, currently_parsed_mechanism);

						free(new_type);
					}
					else {
						tmpval = calloc(100, sizeof(char));
						sscanf(params[i].param_type,"dataUsage[%[^]s]]",tmpval);

						params[i].param_value = strdup(tmpval);
						xmlSetProp(node1, "value", tmpval);


						mapDataToMechanism(tmpval, currently_parsed_mechanism);
					}
				}
				//plog(LOG_TRACE," Counting params %s %i", node1->name, i);
				// plog(LOG_TRACE,"    %s=[%s]", params[i].param_name, params[i].param_value);
				i++;
			}
		}
	}
	*p_params = params;
	return n_params;
}


void parseFormula(xmlNode * node, char *condition) {
	xmlNode * node1;
	xmlChar * value;
	event_t *event;
	int i;
	int flag;
	if (node==NULL) return;
	if (strcmp(node->name, "text")==0 || strcmp(node->name, "comment")==0) return;

	if (
			strcmp(node->name, "true")==0 ||
			strcmp(node->name, "false")==0
	) {
		// Operators that we should only print the nale
		sprintf(condition+strlen(condition), "%s", node->name);

	} else if (strcmp(node->name, "XPathEval")==0) {
		// XPathEval
		sprintf(condition+strlen(condition), "XPathEval(%s)", getText(node));

	} else if (strcmp(node->name, "event")==0) {
		// Event Eall/Efst
		parseEvent(node, &event);
		sprintf(condition+strlen(condition), " Eall (%s)",event->event_name);
		if (event->n_params > 0) {
			sprintf(condition+strlen(condition), ", {");
			for(i=0; i < event->n_params; i++) {
				sprintf(condition+strlen(condition), "(%s, %s)%c",event->params[i].param_name, event->params[i].param_value,(i<event->n_params -1)?',':'}');
			}
			//sprintf(condition+strlen(condition), "}");
		}
		sprintf(condition+strlen(condition), ") ");

	} else if (
			strcmp(node->name, "not")==0 ||
			strcmp(node->name, "always")==0
	) {
		sprintf(condition+strlen(condition), "%s(", node->name);
		for (node1 = node->children; node1; node1 = node1->next) {
			parseFormula(node1, condition);
		}
		sprintf(condition+strlen(condition), ")");

	} else if (
			strcmp(node->name, "and")==0 ||
			strcmp(node->name, "implies")==0 ||
			strcmp(node->name, "or")==0
	) {
		// All the operators with 2 operands
		flag = 0;
		for (node1 = node->children; node1; node1 = node1->next) {
			// printf("%s ", node1->name);
			if (strcmp(node1->name, "text")!=0 && strcmp(node1->name, "comment")!=0) {
				if (flag==0) {
					sprintf(condition+strlen(condition), "(");
					parseFormula(node1, condition);
					flag=1;
				} else {
					sprintf(condition+strlen(condition), " %s ", node->name);
					parseFormula(node1, condition);
					sprintf(condition+strlen(condition), ")");
				}
			}
		}
	} else if (strcmp(node->name, "since")==0) {
		// All the operators with 2 operands
		flag = 0;
		for (node1 = node->children; node1; node1 = node1->next) {
			// printf("%s ", node1->name);
			if (strcmp(node1->name, "text")!=0 && strcmp(node1->name, "comment")!=0) {
				if (flag==0) {
					sprintf(condition+strlen(condition), "since (");
					parseFormula(node1, condition);
					flag=1;
				} else {
					sprintf(condition+strlen(condition), ", ");
					parseFormula(node1, condition);
					sprintf(condition+strlen(condition), ")");
				}
			}
		}
	} else if (
			strcmp(node->name, "before")==0 ||
			strcmp(node->name, "during")==0 ||
			strcmp(node->name, "within")==0
	) {
		sprintf(condition+strlen(condition), "%s(", node->name);
		value = xmlGetProp(node, "timeInterval");
		sprintf(condition+strlen(condition), "%s, ", value);
		xmlFree(value);
		for (node1 = node->children; node1; node1 = node1->next) {
			parseFormula(node1, condition);
		}
		sprintf(condition+strlen(condition), ")");

	} else if (strcmp(node->name, "replim")==0) {

		sprintf(condition+strlen(condition), "%s(", node->name);
		value = xmlGetProp(node, "timeInterval");
		sprintf(condition+strlen(condition), "%s, ", value);
		xmlFree(value);
		value = xmlGetProp(node, "lowerLimit");
		sprintf(condition+strlen(condition), "%s, ", value);
		xmlFree(value);
		value = xmlGetProp(node, "upperLimit");
		sprintf(condition+strlen(condition), "%s, ", value);
		xmlFree(value);
		for (node1 = node->children; node1; node1 = node1->next) {
			parseFormula(node1, condition);
		}
		sprintf(condition+strlen(condition), ")");

	} else if (strcmp(node->name, "repmax")==0) {

		sprintf(condition+strlen(condition), "%s(", node->name);
		value = xmlGetProp(node, "limit");
		sprintf(condition+strlen(condition), "%s, ", value);
		xmlFree(value);
		for (node1 = node->children; node1; node1 = node1->next) {
			parseFormula(node1, condition);
		}
		sprintf(condition+strlen(condition), ")");


	} else if (strcmp(node->name, "repsince")==0) {
		// All the operators with 2 operands
		flag = 0;
		for (node1 = node->children; node1; node1 = node1->next) {
			// printf("%s ", node1->name);
			if (strcmp(node1->name, "text")!=0) {
				if (flag==0) {
					sprintf(condition+strlen(condition), "%s (", node->name);
					value = xmlGetProp(node, "upperLimit");
					sprintf(condition+strlen(condition), "%s, ", value);
					xmlFree(value);
					parseFormula(node1, condition);
					flag=1;
				} else {
					sprintf(condition+strlen(condition), ", ");
					parseFormula(node1, condition);
					sprintf(condition+strlen(condition), ")");
				}
			}
		}
		// sprintf(condition+strlen(condition), ")");
		// } else {
		// } else if (strcmp(node->name, "PermitOnlyEventName")==0) {
		// } else if (strcmp(node->name, "PermitOnlyEventParam")==0) {
	} else if (strcmp(node->name, "denyc")==0){
		sprintf(condition+strlen(condition), "%s()", node->name);
	} else if (strcmp(node->name, "denyd")==0){
		sprintf(condition+strlen(condition), "%s()", node->name);
	} else if (strcmp(node->name, "limit")==0){
		sprintf(condition+strlen(condition), "%s()", node->name);
	}

}

void parseControlMechanism(xmlNode * node0, char **r_id, event_t ** r_event, char *condition, mechanism_actions_t **r_actions) {
	currently_parsed_mechanism = node0;

	xmlNode *node1;
	xmlNode *node2;
	xmlNode *node3;
	xmlNode *node4;
	xmlChar *value;
	char *id;
	event_t *event;
	mechanism_actions_t *actions;
	int k;

	for (node1 = node0->children; node1; node1 = node1->next) {
		if (strcmp(node1->name, "id")==0) {
			id = getText(node1);
			plog(LOG_DEBUG,"- Parsing control mechanism [%s]", id);

		} else if (strcmp(node1->name, "triggerEvent")==0) {
			parseEvent(node1, &event);
			plog(LOG_TRACE,"  trigger=[%s]", event->event_name );
		} else if (strcmp(node1->name, "condition")==0) {
			// condition = getText(node1);
			// TODO: Max size of the condition is hard coded 2048
			// and this is a security vulnerability
			condition[0] = '\0';
			for (node2 = node1->children; node2; node2 = node2->next) {
				parseFormula(node2, condition);
			}
			plog(LOG_TRACE,"  condition=[%s]", condition );
		} else if (strcmp(node1->name, "actions")==0) {

			actions = (mechanism_actions_t *)malloc(sizeof(mechanism_actions_t));
			// No delay
			actions->delay = 0;
			// No parameters to be modified
			actions->n_params = 0;
			actions->modify = NULL;
			// Zero actions to be executed
			actions->n_actions = 0;
			actions->execute = NULL;

			for (node2 = node1->children; node2; node2 = node2->next) {

				if (strcmp(node2->name, "allow")==0) {
					actions->response = ALLOW;
					plog(LOG_TRACE,"  action=[ALLOW]");

					// DELAY default parameter
					actions->delay = 0;

					// MODIFY default parameters
					actions->n_params = 0;
					actions->modify = NULL;

					for (node3 = node2->children; node3; node3 = node3->next) {
						if (strcmp(node3->name, "delay")==0) {
							value = xmlGetProp(node3, "duration");
							actions->delay = atoi(value);
							xmlFree(value);
							plog(LOG_TRACE,"  action=[DELAY] duration=[%d]", actions->delay);
						}
						if (strcmp(node3->name, "modify")==0) {
							actions->n_params = parseParams(node3, &actions->modify);
							plog(LOG_TRACE,"  action=[MODIFY] n_params=[%d]", actions->n_params);
						}
					}

				} else if (strcmp(node2->name, "inhibit")==0) {
					actions->response = INHIBIT;
					plog(LOG_TRACE,"  action=[INHIBIT]");


				} else if (strcmp(node2->name, "execute")==0) {

					// EXECUTE default is zero
					actions->n_actions = 0;

					// we first iterate to count the actions
					for (node3 = node2->children; node3; node3 = node3->next) {
						if (strcmp(node3->name, "action")==0) {
							actions->n_actions++;
						}
					}

					plog(LOG_TRACE,"  action=[EXECUTE] n_actions=[%d]", actions->n_actions);

					// then we allocate memory
					actions->execute = (action_t *)malloc(sizeof(action_t) * actions->n_actions);

					k=0;
					for (node3 = node2->children; node3; node3 = node3->next) {
						if (strcmp(node3->name, "action")==0) {
							for (node4 = node3->children; node4; node4 = node4->next) {
								if (strcmp(node4->name, "id")==0) {
									actions->execute[k].id = getText(node4);
								}
							}
							plog(LOG_TRACE,"  execute[%d]=[%s]", k, actions->execute[k].id);
							actions->execute[k].n_params = parseParams(node3, &actions->execute[k].params);
							k++;
						}
					}
				}
			}
		} 
	}
	*r_id = id;
	*r_event = event;
	*r_actions = actions;

	currently_parsed_mechanism = NULL;
}


void mapDataToMechanism(char *data, xmlNode *mechanism) {
	if (data == NULL || mechanism == NULL) {
		return;
	}

	size_t size;
	PblSet *associatedMechanisms;

	if (mechanism != NULL) {
		if (pblMapContainsKey(mapDataToSetOfMechanisms,data,strlen(data))) {
			associatedMechanisms = *((PblSet **) pblMapGet(mapDataToSetOfMechanisms,data,strlen(data),&size));
		}
		else {
			associatedMechanisms = pblSetNewHashSet();
		}
		pblSetAdd(associatedMechanisms, mechanism);
		pblMapAdd(mapDataToSetOfMechanisms, data,strlen(data),&associatedMechanisms,sizeof(PblSet*));
	}
}


/**
 * Returns the set of mechanisms that is mapped to the specified data.
 * @PARAM		data			the data to get the mechanisms for
 * @RETURN		!NULL			the set of mechanisms
 * 				NULL			if there is no such set
 */
PblSet *getMechanismsForData(char *data) {
	size_t size;

	if (data == NULL) {
		return NULL;
	}

	if (pblMapContainsKey(mapDataToSetOfMechanisms,data,strlen(data))) {
		return *((PblSet **) pblMapGet(mapDataToSetOfMechanisms,data,strlen(data),&size));
	}

	return NULL;
}



/**
 * Returns the array of mechanisms (in XML format as char*) for the specified data and returns the number
 * of mechanisms in size.
 * @PARAM			data		the data to get the mechanisms for
 * @OUT				size		the number of mechanisms
 * @RETURN			!NULL		the array of mechanisms; the array is of length size
 * 					NULL		if there is no such mechanism
 */
unsigned char **getMechanismsForDataAsXML(char *data, size_t *size) {
	int loop;
	int newsize;
	unsigned char **allBuffers = NULL;
	unsigned char **tmpAllBuffers = NULL;
	xmlBufferPtr buf;

	PblSet *mechanisms = getMechanismsForData(data);
	*size = 0;

	if (mechanisms != NULL) {
		// loop over the mechanisms of the data
		for (loop = 0; loop < pblSetSize(mechanisms); loop++) {
			// get the mechanism
			xmlNode *mech = pblSetGet(mechanisms, loop);
			if (mech !=NULL) {
				// make it a char* and copy it to the resulting array
				buf = xmlBufferCreate();
				xmlNodeDump(buf, mech_doc, mech, 1, 0);
				newsize = *size + 1;
				if ((tmpAllBuffers = realloc(allBuffers, newsize * sizeof(xmlBufferPtr))) != NULL) {
					*size = newsize;
					allBuffers = tmpAllBuffers;
					allBuffers[*size-1] = (unsigned char *) xmlBufferContent(buf);
				}

				free(buf);
			}
		}
	}

	return allBuffers;
}


/*
unsigned char **getMechanismsForDataListAsXML(list_of_data_ptr lod, size_t *size) {
	list_of_data_ptr tmplod = lod;
	size_t mech_len;
	int newsize;
	int i;

	*size = 0;

	xmlBufferPtr *mechanisms;
	unsigned char **allMechanisms = NULL;
	unsigned char **tmpAllMechanisms;

	while (tmplod) {
		mechanisms = getMechanismsForDataAsXML(tmplod->data->data_ID, &mech_len);

		if (mech_len > 0) {
			newsize = *size + mech_len;
			if ((tmpAllMechanisms = realloc(allMechanisms, newsize * sizeof(char*))) != NULL) {
				*size = newsize;
				allMechanisms = tmpAllMechanisms;

				for (i = mech_len ; i > 0; i--) {
					allMechanisms[*size - i] = xmlBufferContent(mechanisms[mech_len - i]);
				}
			}
		}

		free(mechanisms);


		tmplod = tmplod->next;
	}

	return allMechanisms;
}*/



int parse_and_add_mechanism(xmlNodePtr node) {
	char *id;
	event_t *event;
	char condition[2048] = "";
	mechanism_actions_t *actions;

	parseControlMechanism(node, &id, &event, condition, &actions);

	// 2000000 as in syscall_handler
	if(add_mechanism(id, event, condition, actions, 2000000)!=R_ERROR) {
		activate_mechanism(id);
		return 0;
	} else {
		plog(LOG_DEBUG,"Error adding mechanism '%s' NOT added", id);
		return -1;
	}
}
