/**
 * @file event.c
 * @brief Implementation for event processing
 *
 * creating, deleting event objects \n
 * handling event parameters \n
 * handling XML information for appropriate events
 *
 * @author Ricardo Neisse, Cornelius Moucha
 **/
#include "esfevent.h"
#include "log_esfevent_c.h"

// unique id for events
unsigned int id=0;

/**
 * Function used internally to free an event parameter
 */
void event_param_free(gpointer data, gpointer user_data)
{
  event_param_ptr event_param=(event_param_ptr)data;
  if(event_param == NULL)
  {
    log_error("Trying to free a NULL event parameter, aborting...");
    return;
  } log_trace("Freeing event parameter [%s]=[%s]", event_param->param_desc->name, event_param->paramValue);
  //free(event_param->value);
  free(event_param);
}

/**
 * Creates a new event and allocate the memory
 *
 * @param action_desc The action description this event refers to
 * @param is_try      TRUE for a desidered or tentative event
 * @return            Pointer to the new created event structure
 **/
event_ptr event_new(action_desc_ptr action_desc, uint index, bool is_try)
{
  event_ptr event;
  // First validate all arguments
  if(action_desc == NULL)
  {
    log_error("NULL action description when creating event");
    return NULL;
  }
  if(!(index == IDX_START || index == IDX_ONGOING))
  {
    log_error("Event index must be IDX_START or IDX_ONGOING");
    return NULL;
  }
  if(!(is_try == TRUE || is_try == FALSE))
  {
    log_error("Is try must be TRUE or FALSE, returning NULL");
    return NULL;
  } 
  // All arguments are fine, create the event
  event = mem_alloc(sizeof(event_t));
  if (event == NULL) {
    log_error("Could not allocate memory for event");
    return NULL;
  }
  log_trace("Creating event action=[%s] index=[%s] is_try=[%s]", action_desc->action_name, event_index_str[index], bool_str[is_try]);
  // unique id counter
  event->id = id++;
  event->action_desc = action_desc;
  event->is_try = is_try;
  event->index = index;
  //event->timestamp = timestamp_new();
  event->params = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) event_param_free);
  event->n_references = 0;
  event->event_trace = NULL;
  event->gindex = 0;
  event->xmlDoc=NULL;
  log_debug("Event id=[%d] [%s][%s][%s] created", event->id, action_desc->action_name, event_index_str[index], bool_str[is_try]);
  return event;
}

/**
 * Searchs and event and returns the event with the respective name
 */
event_param_ptr event_find_param_by_name(event_ptr event, char *param_desc_name)
{
  event_param_ptr event_param;
  if(param_desc_name == NULL)
  {
    log_error("Trying to find an event parameter with NULL parameter description name, returning NULL");
    return NULL;
  }
  event_param =(event_param_ptr)g_hash_table_lookup(event->params, param_desc_name);
  if(event_param == NULL)
    log_debug("Event parameter [%s] not found, returning NULL", param_desc_name);
  else log_debug("Event parameter [%s] found", param_desc_name);
  return event_param;
}

//unsigned char *event_get_param_value_by_name(event_ptr event, char *param_desc_name)
event_paramType_ptr event_get_param_value_by_name(event_ptr event, char *param_desc_name)
{
  event_param_ptr curParam;
  if (param_desc_name == NULL)
  {
    log_error("Trying to find an event parameter with NULL parameter description name, returning NULL");
    return NULL;
  }
  curParam=(event_param_ptr) g_hash_table_lookup(event->params, param_desc_name);
  if(curParam == NULL)
    log_debug("Event parameter [%s] not found, returning NULL", param_desc_name);
  else
  {
    log_debug("Event parameter [%s] found", param_desc_name);
    return curParam->paramValue;
  }
  return NULL;
}

unsigned char *event_get_param_value_by_name_string(event_ptr event, char *param_desc_name)
{
  event_param_ptr curParam;
  if (param_desc_name == NULL)
  {
    log_error("Trying to find an event parameter with NULL parameter description name, returning NULL");
    return NULL;
  }
  curParam=(event_param_ptr) g_hash_table_lookup(event->params, param_desc_name);
  if(curParam == NULL)
    log_debug("Event parameter [%s] not found, returning NULL", param_desc_name);
  else
  {
    log_debug("Event parameter [%s] found", param_desc_name);
    if(curParam->paramType!=PARAM_STRING) {log_warn("Wrong parameter type: expected STRING, found=[%s]", paramDataTypeStr[curParam->paramType]); return NULL;}
    return curParam->paramValue->paramString;
  }
  return NULL;
}

int event_get_param_value_by_name_int(event_ptr event, char *param_desc_name)
{
  event_param_ptr curParam;
  if (param_desc_name == NULL)
  {
    log_error("Trying to find an event parameter with NULL parameter description name, returning NULL");
    return -1;
  }
  curParam=(event_param_ptr) g_hash_table_lookup(event->params, param_desc_name);
  if(curParam == NULL)
	  log_debug("Event parameter [%s] not found, returning NULL", param_desc_name);
  else
  {
    log_debug("Event parameter [%s] found", param_desc_name);
    if(curParam->paramType!=PARAM_INT) {log_warn("Wrong parameter type: expected INT, found=[%s]", paramDataTypeStr[curParam->paramType]); return 0;}
    return curParam->paramValue->paramInt;
  }
  return 0;
}

/**
 * Add a parameter to an event
 */
//unsigned int event_add_param(event_ptr event, action_param_desc_ptr param_desc, unsigned int type, char *value)
unsigned int event_add_param(event_ptr event, action_param_desc_ptr param_desc, unsigned int usageType, unsigned int paramType, event_paramType_ptr paramValue)
{
  // First validate all arguments
  if(event == NULL)
  {
    log_error("Trying to add parameter to NULL event, aborting");
    return R_ERROR;
  }
  if(param_desc == NULL)
  {
    log_error("Trying to add parameter to event(action=[%s]) with NULL parameter description, aborting",event->action_desc->action_name);
    exit(EXIT_FAILURE);
    return R_ERROR;
  }
  // TODO: update
  //if(paramValue == NULL)
  //{
  //  log_error("Trying to add parameter to event with NULL value, aborting");
  //  exit(EXIT_FAILURE);
  //  return R_ERROR;
  //}
  // TODO: do not allow duplicates, only one value per parameter
  // All arguments are fine, add value
  event_param_ptr event_param=(event_param_ptr)mem_alloc(sizeof(event_param_t));
  event_param->param_desc=param_desc;
  event_param->usageType=usageType;
  event_param->paramType=paramType;
  event_param->paramValue=paramValue;
  g_hash_table_insert(event->params, param_desc->name, event_param);
  log_trace("Adding parameter [%s][%p][type:%d] to event", param_desc->name, event_param->paramValue, event_param->paramType);
  return R_SUCCESS;
}

unsigned int    event_add_paramInt(event_ptr event, const unsigned char *paramName, unsigned int usageType, unsigned int value)
{
  event_paramType_ptr paramValue=(event_paramType_ptr)mem_alloc(sizeof(event_paramType_t));
  paramValue->paramInt=value;
  action_param_desc_ptr paramDesc=find_param_desc_by_name_or_insert(event->action_desc, paramName);
  event_add_param(event, paramDesc, usageType, PARAM_INT, paramValue);
}

unsigned int    event_add_paramString(event_ptr event, const unsigned char *paramName, unsigned int usageType, unsigned char *value)
{
  event_paramType_ptr paramValue=(event_paramType_ptr)mem_alloc(sizeof(event_paramType_t));
  paramValue->paramString=strdup(value);
  action_param_desc_ptr paramDesc=find_param_desc_by_name_or_insert(event->action_desc, paramName);
  event_add_param(event, paramDesc, usageType, PARAM_STRING, paramValue);
}

/**
 * Used internally to write information about the event param in the log 
 */
void event_param_log(gpointer key, gpointer value, gpointer user_data) {
  event_param_ptr event_param = (event_param_ptr) value;
  // TODO: update: prepare enum PARAM_TYPE_STR("%s","%d","%x","%p") and include in user_data dynamic selection
  printf(user_data ,event_param->param_desc->name);
  if(event_param->paramType==PARAM_STRING)       printf("[%s] (usageType: %s; dataType: %s)\n", event_param->paramValue->paramString,
                                                                                                paramUsageTypeStr[event_param->usageType],
                                                                                                paramDataTypeStr[event_param->paramType]);
  else if(event_param->paramType==PARAM_INT)     printf("[%d] (usageType: %s; dataType: %s)\n", event_param->paramValue->paramInt,
                                                                                                paramUsageTypeStr[event_param->usageType],
                                                                                                paramDataTypeStr[event_param->paramType]);
  else if(event_param->paramType==PARAM_ADDRESS) printf("[%x] (usageType: %s; dataType: %s)\n", event_param->paramValue->paramAddress,
                                                                                                paramUsageTypeStr[event_param->usageType],
                                                                                                paramDataTypeStr[event_param->paramType]);
  else printf("[%p] (type: %d)\n", event_param->paramValue->paramString, event_param->usageType);
}

/**
 * Write detailed information about the event in the log
 */
void event_log(char *msg, event_ptr event)
{
  printf("=== %s (event log) ===\n", msg);
  printf("    action: %s\n", event->action_desc->action_name);
  printf("       try: %s\n" ,bool_str[event->is_try]);
  printf("     index: %s\n" ,event_index_str[event->index]);
  g_hash_table_foreach(event->params, (GHFunc) event_param_log, " parameter: [%s]=");
  printf("======\n");
}


/**
 * parse XML-file and create corresponding event structure
 *
 * @param xml_doc pointer to XML document structure according libXML2
 * @return        pointer to the created event or NULL for failure
 **/
//event_ptr event_parse_xml(action_desc_store_ptr action_desc_store, xmlNodePtr node)
event_match_op_ptr event_match_parse_xml(action_desc_store_ptr action_desc_store, xmlNodePtr node)
{
	log_trace ("event_match_parse_xml");
  check_null_ptr(action_desc_store, "Failure parsing given event: action_desc_store is null!");
  check_null_ptr(node, "Failure parsing given event: node is null!");
  if( xmlStrncasecmp(node->name, "trigger", 7) && xmlStrncasecmp(node->name, "eventMatch", 10) )
  {
    log_error("Failure parsing given event: node name neither trigger nor eventMatch (name=[%s]\n!", node->name);
    return NULL;
  }

  xmlNode *node0;
  event_match_op_ptr nEvent = NULL;
  bool is_try;
  unsigned int index;

  char *name, *cisTry, *cindex;
  name=xmlGetProp(node,"action");
  check_null_ptr(name, "Failure parsing event: no referenced action found in mechanism xml!")

  cisTry=xmlGetProp(node, "isTry");
  cindex=xmlGetProp(node, "index");
  is_try=(strncmp(cisTry == NULL ? "" : cisTry,"true",4)==0 ? 1 : 0);
  index= (strncmp(cindex == NULL ? "" : cindex,"FIRST",4)==0 ? IDX_START : IDX_ONGOING);

  action_desc_ptr action_desc = find_action_desc_by_name(action_desc_store, name);
  if(action_desc==NULL)
  {
    // Default Value is ACTION_USAGE -> as defined in action schema
    action_desc = action_desc_new(name, ACTION_USAGE);
    if(add_action_desc(action_desc_store, action_desc)!=R_SUCCESS)
    {
      log_error("Failure parsing event: action_description could not be added to store!");
      return NULL;
    }
  }

  nEvent = event_match_op_new(action_desc, index, is_try);
  check_null_ptr(nEvent, "Failure parsing event: event_match_op could not be created!");

  node0=node;
  for( ; node0; node0=(node0==node ? node0->children : node0->next))
  {
    if(xmlIsElement(node0) && !xmlStrncasecmp(node0->name, "paramMatch", 12))
    {
      char *pname = xmlGetProp(node0, "name");
      char *ptype = xmlGetProp(node0, "type");
      char *pvalue= xmlGetProp(node0, "value");
      char *pIsXpath = xmlGetProp(node0, "isXPath");
      char *pqod = xmlGetProp(node0, "qod");
      check_null_ptr(pname, "Failure parsing event: paramMatch without required field name!");
      check_null_ptr(pvalue, "Failure parsing event: paramMatch without required field value!");

      action_param_desc_ptr lactionParamDesc = find_param_desc_by_name_or_insert(action_desc, pname);
      if(find_param_value(lactionParamDesc, pvalue)==NULL)
      {
        if(add_param_value(action_desc, pname, pvalue)!=R_SUCCESS)
        {
          log_error("Failure parsing event: param value could not be inserted!");
          return NULL;
        }
      }
      if(ptype!=NULL && strncasecmp(ptype, "data", 4)==0)
      {
        log_info(" * Parameter [%s] of trigger event [%s] is of type dataUsage",pname,name);
        log_info(" * Storing initial mapping...");
        //xmlSetProp(node0, "value", IF_initCont(pvalue));
        pvalue=(char*)IF_initCont(pvalue, pqod);
      }

      event_match_add_param(nEvent, lactionParamDesc, pvalue,
                            (ptype==NULL ? PARAM_CONTUSAGE : (strncasecmp(ptype, "cont", 4)==0 ? PARAM_CONTUSAGE : PARAM_DATAUSAGE)),
                            (pIsXpath==NULL) ? FALSE : (strncasecmp(pIsXpath, "true", 4)==0 ? TRUE : FALSE));
    }
  }
  log_info("Successfully parsed XML and created event_match_operator referencing action_desc \"%s\"",nEvent->match_action->action_name);
  //event_print(curEvent);
  // TODO free temporary variables like name, pname, ...
  return nEvent;
}

uint event_free(event_ptr event)
{
  if (event == NULL)
  {
    log_error("Trying to free a NULL event, aborting...");
    return R_ERROR;
  }
  log_debug("Freeing event action=[%s]", event->action_desc->action_name);
  g_hash_table_destroy(event->params);

  // free timestamp
  timestamp_free(event->timestamp);

  // double free???
  //mem_free(event);
  log_debug("Event id=[%d] deallocated", event->id);
  return R_SUCCESS;
}

// parsing xml of event received via notifyEvent (NOT parsing an eventMatch or similar trigger!)
event_ptr event_parse_xml(action_desc_store_ptr action_desc_store, xmlDocPtr xmlDoc)
{
	int moka=0;
  check_null_ptr(action_desc_store, "Failure parsing given event: action_desc_store is null!");
  check_null_ptr(xmlDoc, "Failure parsing given event: xmlDoc is null!");

  xmlNodePtr node = xmlDocGetRootElement(xmlDoc);
  event_ptr nEvent = NULL;

  xmlNodePtr node0;
  bool is_try;
  unsigned int index;

  char *name, *cisTry, *cindex;
  name=xmlGetProp(node,"action");

  log_trace("name=%s",name);
  check_null_ptr(name, "Failure parsing event: no referenced action found!")

  cisTry=xmlGetProp(node, "isTry");
  cindex=xmlGetProp(node, "index");
  is_try=(strncmp(cisTry == NULL ? "" : cisTry,"true",4)==0 ? 1 : 0);
  index= (strncmp(cindex == NULL ? "" : cindex,"FIRST",4)==0 ? IDX_START : IDX_ONGOING);

  action_desc_ptr action_desc = find_action_desc_by_name_or_insert(action_desc_store, name);


  if(action_desc==NULL)
  { // should NEVER be the case -> why should we receive an event we haven't subscribed to???
    log_warn("no action found");
    return NULL;
  }
  // free temporary variables
  free(name);
  free(cisTry);
  free(cindex);

  nEvent = event_new(action_desc, index, is_try);
  check_null_ptr(nEvent, "Failure parsing event: event_match_op could not be created!");
  nEvent->xmlDoc = xmlDoc;

  node0=node;
  for(; node0; node0=(node0==node ? node0->children : node0->next))
  {
    if(xmlIsElement(node0) && !xmlStrncasecmp(node0->name, "parameter", 9))
    {
      char *pname = xmlGetProp(node0, "name");
      char *pvalue= xmlGetProp(node0, "value");
      check_null_ptr(pname, "Failure parsing event: parameter without required field name!");
      check_null_ptr(pvalue, "Failure parsing event: parameter without required field value!");

      action_param_desc_ptr param_desc = find_param_desc_by_name(action_desc, pname);
      if(param_desc==NULL)
      {
        if(add_param_desc(action_desc, pname, "string")!=R_SUCCESS)
        {
          log_error("Failure parsing event: param description name could not be inserted!");
          return NULL;
        }
        if(add_param_value(action_desc, pname, pvalue)!=R_SUCCESS)
        {
          log_error("Failure parsing event: param value could not be inserted!");
          return NULL;
        }
        param_desc = find_param_desc_by_name(action_desc, pname);
      }
      // TODO: optimize (add paramdesc is not necessary any more!
      // TODO: distinguish between param types (PARAM_STRING, PARAM_INT, ...)
      event_add_paramString(nEvent, pname, PARAM_CONTUSAGE, pvalue);

      // free temporary variables
      free(pname);
      free(pvalue);
    }
  }

  // parsing timestamp
  nEvent->timestamp = timestamp_new();
  /* deactivated, not yet fully implemented
  xmlChar *cTimeStamp = xmlGetProp(node, "timestamp");
  if(cTimeStamp!=NULL)
  {
    log_trace("Found timestamp: %s", cTimeStamp);
    // strptime
    // @todo need to parse string manually to convert to timestamp_ptr ?!
  }*/
  log_debug("Successfully parsed XML and created event referencing action \"%s\"",nEvent->action_desc->action_name);
  //event_print(curEvent);
  return nEvent;
}




/**
 * Function used internally to free an event parameter matching operator
 */
void param_match_free(gpointer data, gpointer user_data) {
  param_match_op_ptr param_match = (param_match_op_ptr) data;
  if (param_match == NULL) {
    log_error("Trying to free a NULL event parameter matching operator, aborting...");
    return;
  }
  log_trace("Freeing event parameter matching [%s][%s][%s]", param_match->param_desc->name, param_match->value, bool_str[param_match->is_xpath]);
  free(param_match->value);
  free(param_match);
}

unsigned int event_match_op_free(event_match_op_ptr event_match) {
  if (event_match == NULL) {
    log_error("Trying to free a NULL event matching operator, aborting...");
    return R_ERROR;
  }
  log_debug("Freeing event matching operator action=[%s]", event_match->match_action->action_name);
  g_hash_table_destroy(event_match->match_params);
  mem_free(event_match);
  log_debug("Event matching operator deallocated");
  return R_SUCCESS;
}

event_match_op_ptr event_match_op_new(action_desc_ptr match_action, uint match_index, bool match_try) {
  event_match_op_ptr event_match;
  // First validate all arguments
  if (match_action == NULL) {
    log_error("NULL action description when creating matching operator, returning NULL");
    return NULL;
  }
  if (!(match_index == MATCH_FIRST || match_index == MATCH_ALL)) {
    log_error("Match index must be MATCH_FIRST or MATCH_ALL for matching operator, returning NULL");
    return NULL;
  }
  if (!(match_try == TRUE || match_try == FALSE)) {
    log_error("Match try must be TRUE or FALSE, returning NULL");
    return NULL;
  }
  log_info("Creating event matching operator action=[%s] match_index=[%s] match_try=[%s]",
           match_action->action_name, event_match_str[match_index], bool_str[match_try]);
  event_match = mem_alloc(sizeof(event_match_op_t));
  if (event_match == NULL) {
    log_error("Could not allocate memory for event matching operator");
    return NULL;
  }
  event_match->match_action = match_action;
  event_match->match_index = match_index;
  event_match->match_try = match_try;
  event_match->match_params = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) param_match_free);
  return event_match;  
}

uint event_match_add_param(event_match_op_ptr event_match, action_param_desc_ptr param_desc, char *value,
                           unsigned int type, bool is_xpath)
{
  // First validate all arguments
  if (event_match == NULL) {
    log_error("Trying to add parameter to NULL event matching operator, aborting");
    return R_ERROR;
  }
  if (param_desc == NULL) {
    log_error("Trying to add parameter to event matching operator with NULL parameter description, aborting");
    return R_ERROR;
  }
  if (value==NULL) {
    log_error("Trying to add parameter to event with NULL value, aborting");
    return R_ERROR;
  }
  if (!(is_xpath == TRUE || is_xpath == FALSE)) {
    log_error("Is_XPath must be TRUE or FALSE, returning NULL");
    return 0;
  }    
  param_match_op_ptr param_match = (param_match_op_ptr) mem_alloc(sizeof(param_match_op_t));
  param_match->param_desc = param_desc;
  param_match->value = strdup(value);
  param_match->type = type;               // ContUsage or DataUsage
  param_match->is_xpath = is_xpath;
  g_hash_table_insert(event_match->match_params, param_desc->name, param_match);
  log_trace("Adding parameter matching [%s][%s][%s][%s] to event",
            param_desc->name, value, param_desc->type,bool_str[is_xpath]);
  return R_SUCCESS;
}

bool paramCompare(event_param_ptr event_param, param_match_op_ptr param_match)
{
  if(event_param->paramType==PARAM_STRING)
  {
    return (strcmp(event_param->paramValue->paramString, param_match->value) == 0);
  }
  else if(event_param->paramType==PARAM_INT)
  {
    // paramMatch doesn't support INT yet -> convert paramValue->int to string
    log_warn("x1a - converting int-param back to string for comparison");
    char *tmp=(char*)malloc(16*sizeof(char));
    snprintf(tmp, 16, "%d", event_param->paramValue->paramInt);
    return (strcmp(tmp, param_match->value) == 0);
  }
  else if(event_param->paramType==PARAM_ADDRESS)
  {
    log_error("not yet supported parameter type! aborting with FALSE");
    return FALSE;
  }
  else
  {
    log_error("unknown parameter type! aborting with FALSE");
    return FALSE;
  }
}

void iterator(gpointer key, gpointer value, gpointer user_data)
{
  struct event_match_eval *eval=(struct event_match_eval *)user_data;
  event_ptr event=eval->event;
  param_match_op_ptr param_match=(param_match_op_ptr)value;
  event_param_ptr event_param=event_find_param_by_name(event, param_match->param_desc->name);
  log_warn("comparing parameter=[%s]", param_match->param_desc->name);

  if(event_param!=NULL)
  { //if ((B->params[k].param_type == NULL) || (strcmp(B->params[k].param_type, "contUsage")) == 0)
    if(param_match->type==PARAM_CONTUSAGE)
    {
      // TODO: update!
      //if(strcmp(event_param->value, param_match->value) == 0)
      //if(strcmp(event_param->paramValue->paramString, param_match->value) == 0)
      if(paramCompare(event_param, param_match)==TRUE)
      {
        log_trace("Parameter match [%s]=[%s]",event_param->paramValue->paramString, param_match->value);
      }
      else
      {
        log_trace("Parameter does not match [%s]!=[%s]",event_param->paramValue->paramString, param_match->value);
        eval->result=FALSE;
      }
    }
    else if(DATA_FLOW_ENABLED == 1)
    { // TODO dataUsage-parameter are only matched if DATA_FLOW_ENABLED is enabled????
      char *tmpcont=NULL;
      list_of_data_ptr lod=NULL;
      //Let's check if data stored in file A is B
      //tmpcont=dataCont_getNaming(data_flow_monitor->map,"FNAME", event_param->value);
      if(event_param->paramType!=PARAM_STRING)
      {
        log_warn("trying to compare INT-PARAM as DATAUSAGE -> not yet supported");
        eval->result=FALSE;
        return;
      }
      tmpcont=dataCont_getNaming(data_flow_monitor->map,"FNAME", event_param->paramValue->paramString);
      if(tmpcont==NULL)
      { //no naming for A -> no data in the file
        eval->result=FALSE;
        //break;
      }

      lod=dataCont_getDataIn(data_flow_monitor->map, tmpcont);
      if (list_of_data_length(lod)==0)
      { // no data in file A
        eval->result=FALSE;
        //break;
      }

      if(list_of_data_find(lod, param_match->value)==0)
        //log_info("  [%s] \"refines\" [%s]", event_param->value, param_match->value);
        log_info("  [%s] \"refines\" [%s]", event_param->paramValue->paramString, param_match->value);
      else
      {
        //log_info("  [%s] doesn't \"refine\" [%s]", event_param->value, param_match->value);
        log_info("  [%s] doesn't \"refine\" [%s]", event_param->paramValue->paramString, param_match->value);
        eval->result=FALSE;
      }
    }
  }
  else log_warn("event_param is NULL! should be handled appropriately");
  return;
}

bool checkParameters(event_match_op_ptr event_match, event_ptr event)
{
  log_debug("%s - comparing trigger event (nParam=[%d]) with desired event(nParam=[%d])", __func__,
            g_hash_table_size(event_match->match_params), g_hash_table_size(event->params) );

  // using g_hash_table_foreach is not advisable, as i cannot stop searching if already the first param doesn't match
  GHashTableIter paramIterator;
  g_hash_table_iter_init(&paramIterator, event_match->match_params);

  // key not used...
  param_match_op_ptr paramMatch=NULL;
  gpointer key, value;
  while(g_hash_table_iter_next(&paramIterator, &key, &value))
  {
    param_match_op_ptr param_match=(param_match_op_ptr)value;
    event_param_ptr event_param=event_find_param_by_name(event, param_match->param_desc->name);
    if(event_param!=NULL)
    {
      if(param_match->type==PARAM_CONTUSAGE)
      {
        //if(strcmp(event_param->paramValue->paramString, param_match->value) == 0)
        if(paramCompare(event_param, param_match)==TRUE)
        {
          log_debug("Parameter match [%s]=[%s]",event_param->paramValue->paramString, param_match->value);
        }
        else
        {
          log_debug("Parameter does not match [%s]!=[%s]",event_param->paramValue->paramString, param_match->value);
          return FALSE;
        }
      }
      else if(DATA_FLOW_ENABLED == 1)
      { // TODO dataUsage-parameter are only matched if DATA_FLOW_ENABLED is enabled????
        char *tmpcont=NULL;
        list_of_data_ptr lod=NULL;
        //Let's check if data stored in file A is B
        //tmpcont=dataCont_getNaming(data_flow_monitor->map,"FNAME", event_param->value);
        if(event_param->paramType!=PARAM_STRING)
        {
          log_warn("trying to compare INT-PARAM as DATAUSAGE -> not yet supported");
          return FALSE;
        }
        tmpcont=dataCont_getNaming(data_flow_monitor->map,"FNAME", event_param->paramValue->paramString);
        if(tmpcont==NULL)
        { //no naming for A -> no data in the file
          return FALSE;
          //break;
        }

        lod=dataCont_getDataIn(data_flow_monitor->map, tmpcont);
        if (list_of_data_length(lod)==0)
        { // no data in file A
          //eval->result=FALSE;
          return FALSE;
          //break;
        }

        if(list_of_data_find(lod, param_match->value)==0)
          log_info("  [%s] \"refines\" [%s]", event_param->paramValue->paramString, param_match->value);
        else
        {
          log_info("  [%s] doesn't \"refine\" [%s]", event_param->paramValue->paramString, param_match->value);
          return FALSE;
        }
      }
      else return FALSE; //?????
    }
    else return FALSE;
  }
  return TRUE;
}

bool event_matches(event_match_op_ptr event_match, event_ptr event, char *xml_doc)
{
  // null checking for detective mechanisms (have no trigger)
  if(event_match == NULL) return FALSE;
  struct event_match_eval eval;
  eval.result=TRUE;
  eval.event=event;
  log_debug("Matching event[%d]", event->id);

  if(event_match->match_index == MATCH_ALL || (event_match->match_index== MATCH_FIRST && event->index == IDX_START))
  {
    log_trace("Event index matches operator [%s]~=[%s]", event_index_str[event->index], event_match_str[event_match->match_index] );
    if(event_match->match_try == event->is_try)
    {
      log_trace("Event is_try matches operator [%s]=[%s]", bool_str[event->is_try], bool_str[event_match->match_try] );
      if(strcmp(event->action_desc->action_name, event_match->match_action->action_name)== 0)
      {
        log_trace("Event action matches [%s]=[%s]", event->action_desc->action_name, event_match->match_action->action_name);
        // Checking parameters
        if(xml_doc!=NULL) g_hash_table_foreach(event_match->match_params, (GHFunc)iterator, &eval);
        else
        {
          bool result=checkParameters(event_match, event);
          if(result) eval.result=TRUE;
          else eval.result=FALSE;
        }
        // should be foreach of all params of DESIRED event!
      }
      else
      {
        log_trace("Event action does not match [%s]!=[%s]", event->action_desc->action_name, event_match->match_action->action_name);
        eval.result=FALSE;
      }
    }
    else
    {
      log_trace("Event is_try does not match operator [%s]!=[%s]", bool_str[event->is_try], bool_str[event_match->match_try] );
      eval.result=FALSE;
    }
  }
  else
  {
    log_trace("Event index does not match operator [%s]!~=[%s]", event_index_str[event->index], event_match_str[event_match->match_index] );
    eval.result=FALSE;
  }

  if(eval.result) log_debug("Event matches operator");
  else            log_debug("Event does not match operator");
  return eval.result;
}

/// @bug not implemented
unsigned long event_getMemSize(event_ptr event)
{
  unsigned long size=sizeof(event_ptr);
//  if(event==NULL) return size;
//  size+=(strlen(event->event_name)+1)*sizeof(unsigned char) + sizeof(unsigned char *);
//  size+=sizeof(bool);
//  size+=3*sizeof(unsigned int);
//
//  unsigned int a;
//  for(a=0; a<event->n_params; a++)
//    size+=param_getMemSize(event->params[a]);
//  size+=sizeof(param_ptr *);
//  size+=sizeof(uint64_t);
//  size+=xml_getMemSize(event->xml_doc);
  return size;
}



void event_createXMLdocParams(gpointer key, gpointer value, gpointer user_data)
{
  event_param_ptr event_param = (event_param_ptr) value;

  xmlNodePtr paramNode = NULL;
  event_param_ptr curParam = (event_param_ptr)value;
  paramNode = xmlNewChild( (xmlNodePtr)user_data, NULL, "parameter", NULL);
  xmlNewProp(paramNode, "name", curParam->param_desc->name);
  if(curParam->paramType==PARAM_STRING) xmlNewProp(paramNode, "value", curParam->paramValue->paramString);
  else
  {
    char *tmpString=malloc(16*sizeof(char));
    sprintf(tmpString,"[%d]", curParam->paramValue->paramInt);
    xmlNewProp(paramNode, "value", tmpString);
  }
  xmlNewProp(paramNode, "type", paramUsageTypeStr[curParam->usageType]);
}

unsigned int event_createXMLdoc(event_ptr levent)
{
  xmlDocPtr xmldoc = NULL;
  xmlNodePtr eventNode = NULL;
  xmldoc = xmlNewDoc("1.0");
  eventNode = xmlNewNode(NULL, "event");
  xmlDocSetRootElement(xmldoc, eventNode);
  xmlNewProp(eventNode, "action", levent->action_desc->action_name);
  xmlNewProp(eventNode, "isTry", bool_str[levent->is_try]);
  xmlNewProp(eventNode, "index", event_index_str[levent->index]);

  g_hash_table_foreach(levent->params, (GHFunc)event_createXMLdocParams, eventNode);
  //dumpXMLdoc(xmldoc);
  levent->xmlDoc=xmldoc;
  return R_SUCCESS;
}




/* not used */

/*
// Event trace
event_trace_ptr event_trace_new() {
  event_trace_ptr event_trace;
  event_trace = mem_alloc(sizeof(event_trace_t));
  if (event_trace == NULL) {
    log_error("Could not allocate memory for event trace, returning NULL");
    return NULL;
  }
  // array of event pointers
  log_debug("Creating event trace");
  event_trace->events = g_array_new(FALSE, FALSE, sizeof(event_ptr));
  log_info("Event trace created");
  return event_trace;
}

uint event_trace_add(event_trace_ptr event_trace, event_ptr event) {
  if (event_trace == NULL) {
    log_error("Trying to add event to a NULL event trace, aborting...");
    return R_ERROR;
  }
  if (event == NULL) {
    log_error("Trying to add NULL event to event trace, aborting...");
    return R_ERROR;
  }
  log_debug("Adding event [%s] to trace", event->action_desc->action_name);
  g_array_append_val(event_trace->events, event);
  event->event_trace = event_trace;
  event->gindex = event_trace->events->len - 1;
  log_trace("Event added at gindex=[%d]", event->gindex);
  log_info("Event id=[%d] added to trace", event->id);
  return R_SUCCESS;
}

uint event_trace_remove(event_trace_ptr event_trace, event_ptr event) {
  event_ptr last_event;
  if (event_trace == NULL) {
    log_error("Trying to remove event from a NULL event trace, aborting...");
    return R_ERROR;
  }
  if (event == NULL) {
    log_error("Trying to remove NULL event from event trace, aborting...");
    return R_ERROR;
  }
  // Remove event
  log_debug("Removing event from trace gindex=[%d]", event->gindex);
  g_array_remove_index_fast(event_trace->events, event->gindex);
  // Update gindex of new event in the position of removed event if there is one
  if (event_trace->events->len!=0) {
    last_event = g_array_index(event_trace->events, event_ptr, event->gindex);
    if (last_event==NULL) {
      log_error("There is no event (NULL) to update gindex removed event", event->gindex);
    } else {
      log_trace("Udating gindex of event in the new position from [%d] to [%d]", last_event->gindex, event->gindex);
      last_event->gindex = event->gindex;
    }
  } else {
    log_trace("Last event in trace removed, event trace is empty", event->gindex);
  }
  log_debug("Event id=[%d] removed from trace", event->id);
  return R_SUCCESS;
}

uint event_trace_free(event_trace_ptr event_trace) {
  event_ptr event;
  int i;
  if (event_trace == NULL) {
    log_error("Trying to free a NULL event trace, aborting...");
    return R_ERROR;
  }
  log_debug("Freeing event trace");
  for (i = 0; i < event_trace->events->len; i++) {
   log_trace("Freeing event at position gindex=[%d]", i);
   event = g_array_index(event_trace->events, event_ptr, i);
   event_free(event);
  }
  g_array_free(event_trace->events, FALSE);
  mem_free(event_trace);
  log_debug("Event trace deallocated");
  return R_SUCCESS;
}


/**
 *
 * Create a new timestep window
 *
 * @param <name> <desc>
 * @return       <desc>
 *
timestep_window_ptr timestep_window_new() {
  timestep_window_ptr window;
  window = mem_alloc(sizeof(timestep_window_t));
  if (window == NULL) {
    log_error("Could not allocate memory for timestep window, returning NULL");
    return NULL;
  }
  log_debug("Creating empty timestep window");
  window->events = g_array_new(FALSE, FALSE, sizeof(event_ptr));
  return window;
}

void timestep_window_free(timestep_window_ptr window) {
  if (window == NULL) {
    log_error("Trying to free a NULL timestep window");
    return;
  }
  log_debug("Freeing timestep window");
  timestep_window_empty(window);
  g_array_free(window->events, FALSE);
  mem_free(window);
  log_debug("Timestep window deallocated");
}

/*
 * Create a new discrete trace with a max number of windows.
 * If max_n_windows is 0 it means we have a discrete trace with unlimited size.
 *
discrete_trace_ptr discrete_trace_new(unsigned int max_n_windows) {
  discrete_trace_ptr discrete_trace;
  timestep_window_ptr window;
  int i;
  discrete_trace = mem_alloc(sizeof(discrete_trace_t));
  if (discrete_trace == NULL) {
    log_error("Could not allocate memory for discrete trace, returning NULL");
    return NULL;
  }
  // array of event pointers
  log_debug("Creating discrete event trace, max windows [%d]", max_n_windows);
  discrete_trace->max_n_windows = max_n_windows;
  discrete_trace->windows = g_array_new(FALSE, FALSE, sizeof(timestep_window_ptr));
  if (max_n_windows != 0) {
    // Discrete trace has limited size, so we create all windows now
    for(i=0; i < max_n_windows; i++) {
      window = timestep_window_new();
      g_array_append_val(discrete_trace->windows, window);
    }
  } else {
    log_trace("Timestep windows will be created on demand");
    // Discrete trace has unlimited size, so we add windows on demand
  }
  discrete_trace->cur_window=0;
  log_info("Discrete event trace created, max_n_windows=[%d]", max_n_windows);
  return discrete_trace;
}

/**
 * Remove all events from a timestep window and decrement reference counter.
 * If references are zero, remove from the event trace (if any),
 * and free the event.
 *
uint timestep_window_empty(timestep_window_ptr window) {
  event_ptr event;
  int i;
  if (window == NULL) {
    log_error("Trying to empty NULL timestep window");
    return R_ERROR;
  }
  log_trace("Emptying timestep window");
  for(i=0; i < window->events->len; i++) {
    event = g_array_index(window->events, event_ptr, i);
    // TODO: remove from trace first!
    event->n_references--;
    log_debug("Removing event id=[%d] from timestep window, references=[%d]", event->id, event->n_references);
    g_array_remove_index_fast(window->events, i);
    if (event->n_references == 0) {
      // we are the last reference to the event, remove from trace and free it
      if (event->event_trace != NULL) {
        event_trace_remove(event->event_trace,event);
      }
      event_free(event);
    }
  }
  return R_SUCCESS;
}

/**
 * Add event to timestep window and increment reference counter.
 *
uint timestep_window_add_event(timestep_window_ptr window, event_ptr event) {
  if (window == NULL) {
    log_error("Trying add event to NULL timestep window");
    return R_ERROR;
  }
  event->n_references++;
  log_debug("Adding event to timestep window, references=[%d]", event->n_references);
  g_array_append_val(window->events, event);
  // incremenent event counter
  log_info("Event id=[%d] added to timestep window", event->id, event->action_desc->action_name);
  return R_SUCCESS;
}


/*
 * Adds a timestep window to the discrete trace.
 * - If trace has limited size, we empty the oldest
 *   timestep window and re-use it.
 * - If trace has unlimited size we simply add a new empty window to
 *   the trace and return it.
 *
timestep_window_ptr discrete_trace_next_window(discrete_trace_ptr discrete_trace) {
  timestep_window_ptr window;
  int next_window;
  if (discrete_trace == NULL) {
    log_error("Trying to add timestep window to NULL discrete trace, returning NULL");
    return NULL;
  }
  log_debug("Returning next timestep window from discrete trace");
  if (discrete_trace->max_n_windows != 0) {
    // Limited size
    // Next window is the oldest added
    next_window = discrete_trace->cur_window + 1;
    if (next_window == discrete_trace->max_n_windows) {
      next_window = 0;
    }
    discrete_trace->cur_window = next_window;
    window = g_array_index(discrete_trace->windows, timestep_window_ptr, next_window);
    log_trace("Empty oldest window [%d] and return it", next_window);
    // TODO: empty oldest window, decrement event reference counters,
    //       if zero reference counters also free the event and remove from trace.
    timestep_window_empty(window);
  } else {
    // Unlimited size
    window = timestep_window_new();
    g_array_append_val(discrete_trace->windows, window);
    discrete_trace->cur_window = discrete_trace->windows->len - 1;
    log_trace("Append new window to discrete trace length=[%d]", discrete_trace->windows->len);
  }
  return window;
}

/*
 * Retrieves a timestep window from the discrete trace.
 * The time_shif indicates the timestep window we want to retrieve.
 * - 0: last added timestep window
 * - 1: second last added
 * - 2: thrid last added
 * - N: (N+1) last added
 *
timestep_window_ptr dicrete_trace_get_window(discrete_trace_ptr discrete_trace, uint time_shift) {
  int index;
  if (discrete_trace->max_n_windows != 0) {
    // Limited size
    if (time_shift >= discrete_trace->max_n_windows) {
      log_error("Past timestep window not available in the history, returning NULL");
      return NULL;
    }
    index = discrete_trace->cur_window - time_shift;
    if (index < 0) {
      index = index + discrete_trace->max_n_windows;
    }
    log_trace("Returning timestep window at index [%d] time shift [%d]", index, time_shift);
    return g_array_index(discrete_trace->windows, timestep_window_ptr, index);
  } else {
    // Unlimited size
    index = discrete_trace->windows->len - 1 - time_shift;
    if (index < 0) {
      log_trace("Timestep window not available, returning NULL");
      return NULL;
    }
    log_trace("Returning timestep window at index [%d] time shift [%d]", index, time_shift);
    return g_array_index(discrete_trace->windows, timestep_window_ptr, index);
  }
}

uint discrete_trace_free(discrete_trace_ptr discrete_trace) {
  timestep_window_ptr window;
  int i;
  if (discrete_trace == NULL) {
    log_error("Trying to free a NULL discrete event trace, aborting...");
    return R_ERROR;
  }
  log_trace("Freeing discrete event trace");
  // Free all timestep windows
  for(i=0; i < discrete_trace->windows->len; i++) {
    window = g_array_index(discrete_trace->windows, timestep_window_ptr, i);
    timestep_window_free(window);
  }
  // Free discrete trace windows
  g_array_free(discrete_trace->windows, FALSE);
  mem_free(discrete_trace);
  log_debug("Discrete trace deallocated");
  return R_SUCCESS;
}

*/



