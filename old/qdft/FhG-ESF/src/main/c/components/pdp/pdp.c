#include "pdp.h"
#include "log_pdp_c.h"

GHashTable *pdp_table;

// Update thread synchronization mutex
pthread_mutex_t *pdp_mtx;

#ifdef _JNI_PDP_
JNIEXPORT jint JNICALL Java_de_fhg_iese_esf_pdp_PolicyDecisionPointNative_start(JNIEnv *env, jobject jobj)
{return pdpStart();}
JNIEXPORT jint JNICALL Java_de_fhg_iese_esf_pdp_PolicyDecisionPointNative_stop(JNIEnv *env, jobject jobj)
{return pdpStop();}

JNIEXPORT jstring JNICALL Java_de_fhg_iese_esf_pdp_PolicyDecisionPointNative_deployMechanism(JNIEnv *env, jobject jobj, jstring jMechanism_doc_path)
{
  jboolean iscopy;
  const char *mechanismDocPath=(*env)->GetStringUTFChars(env, jMechanism_doc_path, &iscopy);
  unsigned int ret=pdpDeployMechanism(mechanismDocPath);
  (*env)->ReleaseStringUTFChars(env, jMechanism_doc_path, mechanismDocPath);
  jstring response=(*env)->NewStringUTF(env, returnStr[ret]);
  return response;
}

JNIEXPORT jstring JNICALL Java_de_fhg_iese_esf_pdp_PolicyDecisionPointNative_notifyEventXML(JNIEnv *env, jobject jobj, jstring jeventDoc)
{
  jboolean iscopy;
  const char *eventDoc = (*env)->GetStringUTFChars(env, jeventDoc, &iscopy);
  unsigned char *ret=pdpNotifyEventXML(eventDoc);
  (*env)->ReleaseStringUTFChars(env, jeventDoc, eventDoc);
  jstring response=(*env)->NewStringUTF(env, ret);
  free(ret);
  return response;
}
#endif

bool pdpStart(unsigned long xml_port, unsigned long sock_port)
{
  pdp_mtx=mem_alloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(pdp_mtx, NULL);

  pdp_table=NULL;
  pdp_action_desc_store=NULL;
  // TODO: use g_hash_table_new_full with destroy_func=mechanism_free
  //pdp_table=g_hash_table_new(g_str_hash, g_str_equal);
  pdp_table=g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)mechanism_free);
  check_null_int(pdp_table, "Unable to create PDP hash table");

  pdp_action_desc_store = action_desc_store_new();
  check_null_int(pdp_action_desc_store, "Unable to create PDP action description store");

  log_debug("Initializing IF tracking");
  data_flow_monitor = data_flow_monitor_new();
  data_flow_monitor_init(data_flow_monitor);
  dataCont_print(data_flow_monitor->map, -1);
  log_debug("IF tracking initialized");

  uint64_t thread_usleep = 200000;
  pdp_startUpdateThread(thread_usleep);

#if PDP_XMLRPC == 1
  // XML-RPC service
  log_info("/pdp %d creating", xml_port);
  esfxmlrpc_ptr pdpxmlrpc=initXmlRPC("/pdp", xml_port, &event_handler);
  log_info("/pdp %d created: %x", xml_port, pdpxmlrpc);
#endif

#if PDP_SOCKET == 1
  esfsocket_ptr lsocket=NULL;
  if( (lsocket=initTCPSocket(sock_port, &pdpSocketHandler))==NULL )
  {
    log_error("Socket could not be initialized!");
    return 1;
  }
#endif

  log_info("PDP initialized");
  return R_SUCCESS;
}

bool pdpStop()
{ // @todo stopping PDP not implemented
  log_info("Stopping pdp...");
  // freeing stored mechanisms...
  return R_SUCCESS;
}

gboolean mechanismTriggerEvent(const unsigned char *name, mechanism_ptr mech, event_ptr levent)
{
  log_trace("%s - checking mechanism: %s", __func__, name);
  // TODO: checking mechanisms currently only looks in trigger!
  // should also look for event references in condition!! not yet implemented
  return event_matches(mech->trigger, levent, (char*)1);
}

unsigned char *pdpNotifyEventXML(const unsigned char *eventDoc)
{
  log_debug("receiving event: [%s]",eventDoc);

  xmlDocPtr doc = load_xml_memory(eventDoc, strlen(eventDoc));
  if(doc==NULL)
  {
    log_error("%s - Failure reading event!", __func__);
    return "error";
  }
  else
  {
	event_ptr levent=event_parse_xml(pdp_action_desc_store, doc);
    log_error("event parsed");
    if (levent!=NULL){
    	log_trace("updating information flow");
    	IF_update(levent);
    }
    notifyResponse_ptr response=pdpNotifyEvent(levent);
    log_error("notification finished");
    char *serializedReponse=notifyResponseSerialize(response);
    log_error("returning serialzed action");
    return serializedReponse;
  }
  return "error";
}

notifyResponse_ptr pdp_newNotifyResponse(event_ptr levent)
{
  notifyResponse_ptr nResponse=mem_alloc(sizeof(notifyResponse_t));
  check_null_ptr(nResponse, "Could not allocate memory for notification response");
  nResponse->event=levent;
  nResponse->authorizationAction=NULL;
  nResponse->n_actions_to_execute=0;
  nResponse->actions_to_execute=NULL;
  return nResponse;
}

notifyResponse_ptr pdpNotifyEvent(event_ptr levent)
{
  check_null_ptr(levent, "Got NULL-event as notification?!");
  notifyResponse_ptr response=pdp_newNotifyResponse(levent);
  g_slist_foreach(levent->action_desc->mechanisms, (GFunc)mechanismTriggerEvent2, response);
  if(response->authorizationAction!=NULL)
  {
    pdp_logNotifyResponse(response);
    return response;
  }
  else
  { // clean up response
    response->event=NULL;
    free(response);
    return NULL;
  }
}

void mechanismTriggerEvent2(mechanism_ptr mech, notifyResponse_ptr response)
{
  event_ptr levent=response->event;
  log_debug("%s - checking mech=[%s] for levent=[%s]", __func__, mech->mech_name ,levent->action_desc->action_name);
  if(event_matches(mech->trigger, levent, NULL))
  {
    log_info("%s - Found matching mechanism -> mechanism_name=[%s]", __func__, mech->mech_name);
    bool conditionValue=update_formula(mech, mech->condition, levent);
    conditionValue=conditionValue && stateFormula();
    if(conditionValue)
    { // condition is satisfied; so update other mechanisms and return authorizationAction and executes
      log_debug("condition is satisfied, so I have to add mechanism-actions etc. to response!!!");

      if(response->authorizationAction==NULL) response->authorizationAction=mechanism_actions_new(ACTION_ALLOW, 0);
      if(pdp_addMechanismResponse(response, mech)==R_ERROR) log_error("preparing response failed!");
    }
    else log_debug("condition NOT satisfied, doing nothing...");
  }
}

void pdp_logNotifyResponse(notifyResponse_ptr response)
{
  log_warn("notifyEvent response: ");
  log_warn("   response: [%s]", response->authorizationAction->response ? "ALLOW" : "INHIBIT");
  log_warn("   delay:    [%llu]", response->authorizationAction->delay);
  log_warn("   modifies: [%u]", response->authorizationAction->n_params_to_modify);
  log_warn("   executes: [%u]", response->n_actions_to_execute);
  unsigned int a;
  for(a=0; a<response->n_actions_to_execute; a++)
    log_warn("      exec action: [%s]", response->actions_to_execute[a]->action->action_name);
}

bool pdp_addMechanismResponse(notifyResponse_ptr response, mechanism_ptr mech)
{
  log_warn("adding mechanism=[%s] to response", mech->mech_name);
  if(response->authorizationAction->response!=ACTION_INHIBIT)
  {
    if(mech->actions->response==ACTION_INHIBIT)
    {
      log_warn("updating ACTION_ALLOW to ACTION_INHIBIT");
      response->authorizationAction->response=ACTION_INHIBIT;
    }
    else
    { // action WAS allowed and should be allowed; so update delay and modifiers...
      // set delay to max(response->delay, new_mech->delay)
      log_warn("keeping ACTION_ALLOW");
      if(mech->actions->delay > response->authorizationAction->delay)
      {
        log_warn("old delay=[%llu] updated to delay=[%llu]", response->authorizationAction->delay, mech->actions->delay);
        response->authorizationAction->delay=mech->actions->delay;
      }

      // update modifies
      // don't know how to handle conflicts... (modification of same parameter from different mechanisms)
      log_error("don't know what to do in case of conflicts...");
      // TODO: look for conflicts!
    }
  }
  else log_warn("response is already INHIBIT");

  // update executions (add executions from new mechanism)
  if(mech->n_actions_to_execute>0)
  {
    log_warn("copying execute-actions to response!");
    response->actions_to_execute=realloc(response->actions_to_execute,(response->n_actions_to_execute+mech->n_actions_to_execute)*sizeof(action_ptr));
    check_null_int(response->actions_to_execute, "Could not allocate memory for new action");

    unsigned int a;
    for(a=0; a<mech->n_actions_to_execute; a++)
    { // copy actions to execute from mechanism to response to avoid duplicating them (time consumption)
      // ==> should NOT be freed with freeing response!!!!
      response->actions_to_execute[response->n_actions_to_execute+a]=mech->actions_to_execute[a];
    }
    response->n_actions_to_execute+=mech->n_actions_to_execute;
  }
  return R_SUCCESS;
}

unsigned int pdpDeployMechanism(const unsigned char *mechanismDocPath)
{
  log_trace("%s - %s", __func__, mechanismDocPath);
  unsigned int ret=load_mechanisms(mechanismDocPath);
  log_trace("%s - loading mechanism returned [%s]", __func__, returnStr[ret]);
  return ret;
}

unsigned int pdpRevokeMechanism(const unsigned char *mechName)
{
  if(mechName==NULL) return R_ERROR;
  log_info("Revoking mechanism=[%s]", mechName);
  if(g_hash_table_lookup(pdp_table, mechName)!=NULL)
  {
    pthread_mutex_lock(pdp_mtx);
    g_hash_table_remove(pdp_table, mechName);
    pthread_mutex_unlock(pdp_mtx);
    return R_SUCCESS;
  }
  else log_info("Mechanism doesn't exist!");
  return R_ERROR;
}

unsigned int pdpRevokePolicy(const unsigned char *mechanismDocPath)
{
  xmlDocPtr    doc=NULL;
  xmlNodePtr   rootNode=NULL;

  doc=loadXmlDoc(mechanismDocPath, schemaFilename);
  if(doc==NULL)
  {
    log_error("Error loading mechanism file!");
    return R_ERROR;
  }
  rootNode=xmlDocGetRootElement(doc);
  searchMechanismNodes(rootNode, TRUE);
  log_error("finished processing xml-nodes");
  xmlFreeDoc(doc);

  return R_SUCCESS;
}

#if PDP_XMLRPC == 1
// XML-RPC method handler
void event_handler(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data)
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
      //log_error("Deploying mechanism not implemented; require passing policy as string-parameter instead of filename!");
      char *policyString = NULL;
      if(!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &policyString))
      {
        type_error(msg, G_TYPE_STRING, params, 0);
        return;
      }

      log_trace("Mechanism received via XMLRPC :\n%s",policyString);

      xmlDocPtr doc = load_xml_memory(policyString, strlen(policyString));
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

#if PDP_SOCKET == 1
unsigned char *pdpSocketHandler(unsigned char *msg)
{
  log_error("socket handler invoked...");
  return "test back";
}
#endif // PDP_SOCKET == 1

unsigned int mechanism_add(xmlNodePtr node)
{
  mechanism_ptr curMechanism=mechanism_parse_xml(node);
  check_null_int(curMechanism, "Error adding mechanism!");

  if(g_hash_table_lookup(pdp_table, curMechanism->mech_name)==NULL)
  {
    pthread_mutex_lock(pdp_mtx);
    g_hash_table_insert(pdp_table, curMechanism->mech_name, curMechanism);
    pthread_mutex_unlock(pdp_mtx);
    log_info("Successfully added mechanism \"%s\" to hashtable",curMechanism->mech_name);
    return R_SUCCESS;
  }
  log_error("Mechanism [%s] already exists!", curMechanism->mech_name);
  return R_ERROR;
}

unsigned int load_mechanisms_xml(xmlNodePtr xml)
{
  xmlNodePtr node;
  node=xml->children;
  while(node!=NULL)
  {
    if(xmlIsElement(node) && (!xmlStrcmp(node->name,"detectiveMechanism") || !xmlStrcmp(node->name, "preventiveMechanism")) )
      if(mechanism_add(node)==R_ERROR) return R_ERROR;
    node=node->next;
  }
  return R_SUCCESS;
}

void searchMechanismNodes(xmlNodePtr rootNode, bool revoke)
{
  xmlNodePtr node=rootNode->children;
  while(node!=NULL)
  {
    if(xmlIsElement(node))
    {
      log_trace("NodeName: %s",node->name);
       if(!xmlStrcmp(node->name,"detectiveMechanism") || !xmlStrcmp(node->name, "preventiveMechanism"))
      {
        if(!revoke) mechanism_add(node);
        else{
        	pdpRevokeMechanism(xmlGetProp(node, "name"));
        }
      }
      else{
    	  searchMechanismNodes(node, revoke);
      }
    }
    node=node->next;
  }
}

unsigned int load_mechanisms(const unsigned char *xmlFilename)
{
  xmlDocPtr    doc=NULL;
  xmlNodePtr   rootNode=NULL;

  log_debug("Loading mechanisms from file: %s", xmlFilename);
  doc=loadXmlDoc(xmlFilename, schemaFilename);
  if(doc==NULL)
  {
    log_error("Error loading mechanism file!");
    return R_ERROR;
  }
  rootNode=xmlDocGetRootElement(doc);
  searchMechanismNodes(rootNode, FALSE);
  log_info("Successfully loaded mechanisms from file: %s",xmlFilename);
  xmlFreeDoc(doc);
  return R_SUCCESS;
}

/// NULL retrieve all references! -> return value: GList* of mechanism_ptr
void* lookup_mechanism(const unsigned char *id)
{
  if(id==NULL) return (mechanism_ptr)g_hash_table_get_values(pdp_table);
  else return (GList*)g_hash_table_lookup(pdp_table, id);
}

void pdp_startUpdateThread(uint64_t thread_usleep)
{
  pthread_t update_thread;
  log_debug("Starting pdp update thread usleep=%llu (%p)", thread_usleep, &thread_usleep);
  pthread_create(&update_thread,NULL,(void*)pdp_thread, &thread_usleep);
}

void pdp_thread(uint64_t *thread_usleep)
{
  // Generates periodically a NULL event to progress time step
  log_info("Started pdp update thread usleep=%llu (@ %p)", *thread_usleep, thread_usleep);
  while(1)
  {
    //rlognl(LOG_PDP_C_OUTPUT, LOG_TRACE, ".");
    g_hash_table_foreach(pdp_table, (GHFunc)&update_mechanism, NULL);

    // TODO: the sleep should be optimized considering
    // the average time it takes to update the monitors
    // and the time step specified. This is now manually
    // set to 5% of the timestep

    // sleep duration should be calculated depending on deployed mechanisms!
    // currently hardcoded to 1s sleeping
    usleep(999999);  //note: in windows usleep works only up to 999.999 microseconds sleeping!
    //sleep(1000);
    //usleep(1000000);
  }
}

bool update_mechanism(const unsigned char *id, mechanism_ptr mech, event_ptr event)
{
  uint64_t usec_cur, usec_elpsd_start, usec_elpsd_last;
  int64_t usec_diff;
  bool IS_NULL_EVENT=(event==NULL);

  // Elapsed time since monitor was updated
  usec_cur = usec_now();
  usec_elpsd_last = usec_cur - mech->timestamp_lastUpdate->usec;
  if(IS_NULL_EVENT)
  {
    usec_diff = usec_elpsd_last - mech->timestepSize;
    if(usec_diff<0)
    {
      // Aborting update because the time step has not passed yet
      log_trace("%s - [%s] Timestep difference %lld -> timestep has not yet passed", __func__, id, usec_diff);
      return FALSE;
    }
    log_debug("%s - [%s] Null-Event updating %llu. timestep at interval of %llu us", __func__, id, mech->timestep, mech->timestepSize);

    // @todo need update!!
    //usec_elpsd_start = usec_cur - mech->usec_start;
    usec_elpsd_start = usec_cur - mech->timestamp_start->usec;

    log_trace("  Start time                 %llu", mech->timestamp_start->usec);
    log_trace("  Last update time           %llu", mech->timestamp_lastUpdate->usec);

    // Correct time substracting possible delay in the execution
    // because the difference between the timestep and the last time
    // the mechanism was updated will not be exactly the timestep

    // @todo only usec-part of timestamp_ptr is updated here! should timeval also be updated?
    mech->timestamp_lastUpdate->usec = usec_cur - usec_diff;
    log_trace("  Corrected last update time %llu", mech->timestamp_lastUpdate->usec);
    log_trace("  Current time               %llu", usec_cur);
    log_trace("  Elapsed since start        %llu", usec_elpsd_start);
    log_trace("  Elapsed since last update  %llu", usec_elpsd_last);
    log_trace("  Timestep difference        %llu", usec_diff);

    if(usec_diff > mech->timestepSize)
    {
      log_warn("%s - [%s] Timestep difference is larger than mechanism's timestep size => we missed to evaluate one timestep!!", __func__, id);
      log_warn("--------------------------------------------------------------------------------------------------------------");
    }
  }

  // check condition formula
  bool mech_value=update_formula(mech, mech->condition, event);
  log_info("%s - [%s] event=[%u] timestep=[%llu] value=[%s]", __func__, id, !IS_NULL_EVENT, mech->timestep, bool_str[mech_value]);

  if(IS_NULL_EVENT)
  { // Reset event nodes for EALL, EFST, XPATH
    unsigned int a;
    for(a=0; a<mech->condition->n_nodes; a++)
    {
      if(mech->condition->nodes[a]->type==EVENTMATCH || mech->condition->nodes[a]->type==XPATH)
        mech->condition->nodes[a]->state->value=FALSE;
    }
    // Increment time step if the time step has passed
    mech->timestep++;
  }

  log_trace("%s - [%s] -----------------------------------------------------------", __func__, id);
  return mech_value;
}


char *notifyResponseSerialize(notifyResponse_ptr response)
{
  xmlDocPtr xmlDoc = NULL;
  xmlNodePtr responseNode = NULL, authorizationNode = NULL, authorizationResponseNode = NULL, actionNode = NULL, paramNode = NULL;
  xmlDoc = xmlNewDoc("1.0");
  responseNode = xmlNewNode(NULL, "notifyEventResponse");
  xmlDocSetRootElement(xmlDoc, responseNode);
  
  authorizationNode=xmlNewChild(responseNode, NULL, "authorizationAction", NULL);
  
  log_trace("deny_because_of_if: %d", get_deny_because_of_if());
  if (get_deny_because_of_if()) {
    log_trace("inhibit because of IF");
    authorizationResponseNode=xmlNewChild(authorizationNode, NULL, "inhibit", NULL);
  } else if(response!=NULL && response->authorizationAction!=NULL) {
    authorizationResponseNode=
      xmlNewChild(authorizationNode, NULL,
		  ((response->authorizationAction->response==0) ? "inhibit" : "allow"), NULL);
  } else {
      authorizationResponseNode =
	xmlNewChild(authorizationNode, NULL, DEFAULT_POLICY, NULL);
  }

  if(response && response->authorizationAction) {
    if (response->authorizationAction->delay!=0) {
      char *amountStr = mem_alloc(20 * sizeof(char));
      snprintf(amountStr, 20, "%llu", response->authorizationAction->delay);
      xmlNodePtr delayNode = xmlNewChild(authorizationResponseNode, NULL, "delay", NULL);
      xmlNewProp(delayNode, "amount", amountStr);
      xmlNewProp(delayNode, "unit", "MICROSECONDS");
      // @todo free amountStr?!
    }
    if(response->authorizationAction->n_params_to_modify>0) {
      xmlNodePtr modifyNode = NULL;
      modifyNode=xmlNewChild(authorizationResponseNode, NULL, "modify", NULL);
      unsigned int b;
      for(b=0; b<response->authorizationAction->n_params_to_modify; b++) {
	paramNode=xmlNewChild(modifyNode, NULL, "parameter", NULL);
	xmlNewProp(paramNode, "name", response->authorizationAction->params_to_modify[b]->param_desc->name);
	xmlNewProp(paramNode, "value", response->authorizationAction->params_to_modify[b]->value);
	xmlNewProp(paramNode, "isXPath", (response->authorizationAction->params_to_modify[b]->is_xpath ? "true" : "false"));
      }
    }
  }

  if(response!=NULL) {
    unsigned int a;
    for(a=0; a<response->n_actions_to_execute; a++) {
      actionNode=xmlNewChild(responseNode, NULL, "action", NULL);
      xmlNewProp(actionNode, "name", response->actions_to_execute[a]->action->action_name);
      unsigned int b;
      for(b=0; b<response->actions_to_execute[a]->n_params_to_action; b++) {
	paramNode=xmlNewChild(actionNode, NULL, "parameter", NULL);
	xmlNewProp(paramNode, "name", response->actions_to_execute[a]->params[b]->param_desc->name);
	xmlNewProp(paramNode, "value", response->actions_to_execute[a]->params[b]->value);
	xmlNewProp(paramNode, "isXPath", (response->actions_to_execute[a]->params[b]->is_xpath ? "true" : "false"));
      }
    }
  }

  xmlBufferPtr retbuffer = dumpXMLdoc(xmlDoc);
  // caller should take care of freeing memory!
  char *retVal=mem_alloc( (strlen(retbuffer->content)+1) * sizeof(unsigned char));
  memcpy(retVal, retbuffer->content, strlen(retbuffer->content)+1);
  
  reset_deny_because_of_if();
  // free temporary variables
  xmlFreeDoc(xmlDoc);
  xmlBufferFree(retbuffer);
  return retVal;
}
