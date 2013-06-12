/**
 * @file  pefEvent.c
 * @brief Implementation for event processing
 *
 * @see pefEvent.h, event_s
 *
 * @author cornelius moucha
 **/

#include "pefEvent.h"
#include "log_pefEvent_pef.h"

/// unique id for received events
unsigned long long id=0;

event_ptr eventNew(actionDescription_ptr actionDescription, bool isTry)
{
  checkNullPtr(actionDescription,"NULL action description when creating event");
  if(!(isTry == TRUE || isTry == FALSE)) {log_error("Is try must be TRUE or FALSE, returning NULL"); return NULL;}

  event_ptr event = memAlloc(sizeof(event_t));
  checkNullPtr(event, "Could not allocate memory for event");

  log_trace("Creating event action=[%s] isTry=[%s]", actionDescription->actionName, boolStr[isTry]);
  event->id=id++;
  event->actionDesc=actionDescription;
  event->isTry=isTry;
  //event->timestamp = timestamp_new();
  event->params = g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) eventParamFree);
  event->xmlDoc=NULL;
  log_trace("Event id=[%llu] [%s][%s] created", event->id, actionDescription->actionName, boolStr[isTry]);

  return event;
}

void eventLog(char *msg, event_ptr event)
{
  if(msg==NULL || event==NULL) {log_warn("Logging NULL event impossible."); return;}
  log_trace("=== %s (event log) ===", msg);
  log_trace("    action: %s", event->actionDesc->actionName);
  log_trace("       try: %s" ,boolStr[event->isTry]);
  g_hash_table_foreach(event->params, (GHFunc) eventParamLog, " parameter: [%s]=");
  log_trace("======");
}

unsigned int eventFree(event_ptr event)
{ /// @todo check
  checkNullInt(event, "Trying to free a NULL event, aborting...");
  log_trace("Freeing event referencing action=[%s]", event->actionDesc->actionName);
  // free timestamp
  //timestampFree(event->timestamp);

  g_hash_table_destroy(event->params);

  // free xmlDoc
  // commented due to SEGFAULT
  //if(event->xmlDoc!=NULL) xmlFreeDoc(event->xmlDoc);

  // double free???
  log_debug("Event id=[%d] deallocated", event->id);
  free(event);
  return R_SUCCESS;
}

/// @todo event_getMemSize not implemented
unsigned long eventGetMemsize(event_ptr event)
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

event_ptr eventParseXML(actionDescStore_ptr actionDescStore, xmlDocPtr xmlDoc)
{
  checkNullPtr(actionDescStore, "Failure parsing given event: action_desc_store is null!");
  checkNullPtr(xmlDoc,          "Failure parsing given event: xmlDoc is null!");

  xmlNodePtr node = xmlDocGetRootElement(xmlDoc);
  event_ptr nEvent = NULL;
  xmlNodePtr node0;
  bool isTry;

  char *name, *cisTry;
  name=xmlGetProp(node,"action");

  // modification for AAL-IS: (<event TOPIC="..." instead of action="...")
  if(name==NULL) return eventParseXML_AAL(actionDescStore, xmlDoc);
  checkNullPtr(name, "Failure parsing event: no referenced action found!");

  cisTry=xmlGetProp(node, "isTry");
  isTry=(strncasecmp(cisTry == NULL ? "" : cisTry,"true",4)==0 ? 1 : 0);

  actionDescription_ptr actionDescription=actionDescStore->find(actionDescStore, name, TRUE);
  checkNullPtr(actionDescription, "Referenced action description not found in store and could not be created?!");

  // free temporary variables
  free(name);
  free(cisTry);

  nEvent=eventNew(actionDescription, isTry);
  checkNullPtr(nEvent, "Failure parsing event: event_match_op could not be created!");
  nEvent->xmlDoc=xmlDoc;

  node0=node;
  for(; node0; node0=(node0==node ? node0->children : node0->next))
  {
    if(xmlIsElement(node0) && !xmlStrncasecmp(node0->name, "parameter", 9))
    {
      char *pname =xmlGetProp(node0, "name");
      char *pvalue=xmlGetProp(node0, "value");
      char *ptypeS=xmlGetProp(node0, "type");

      /// @todo rework CDATA-processing
      if(pvalue==NULL)
      { // dirty hack for android & denis
        log_warn("pvalue is NULL!!!");

        xmlNodePtr nodex1=node0;
        for(; nodex1; nodex1=(nodex1==node0 ? nodex1->children : nodex1->next))
        {
          if(nodex1->type==XML_CDATA_SECTION_NODE)
          {
            pvalue=nodex1->content;
            log_warn("dirty hack: [%s]", pvalue);
          }
        }
      }
      checkNullPtr(pname,  "Failure parsing event: parameter without required field name!");
      checkNullPtr(pvalue, "Failure parsing event: parameter without required field value!");

      if(ptypeS==NULL)
      { // no parameter type given in policy; defaulting to PARAM_STRING // 0
        log_debug("no parameter type given for event; defaulting to PARAM_SRING!");
        eventAddParamString(nEvent, pname, pvalue);
      }
      else if(strncasecmp(ptypeS, "binary", 6)==0)
      { // 5
        log_error("Binary parameter type not yet implemented!");
        eventAddParamBinary(nEvent, pname, pvalue, 0);
      }
      else if(strncasecmp(ptypeS, "int", 2)==0)
      { // 6
        log_debug("Event parameter is of type: INT");
        long intVal=strtol(pvalue, NULL, 0);            // select base automatically
        eventAddParamInt(nEvent, pname, intVal);
      }
      else if(strncasecmp(ptypeS, "long", 2)==0)
      { // 7
        log_debug("Event parameter is of type: LONG");
        long long longVal=strtoll(pvalue, NULL, 0);     // select base automatically
        eventAddParamLong(nEvent, pname, longVal);
      }
      else if(strncasecmp(ptypeS, "bool", 2)==0)
      { // 8
        log_debug("Event parameter is of type: BOOL");
        unsigned int boolVal=0;                        // check for 0, 1, false, true
        if(strncasecmp(pvalue, "true", 4)==0) boolVal=TRUE;
        else if(strncasecmp(pvalue, "false", 5)) boolVal=FALSE;
        else boolVal=strtol(pvalue, NULL, 0);          // select base automatically
        eventAddParamInt(nEvent, pname, boolVal);
      }
      else if(strncasecmp(ptypeS, "stringarray", 2)==0)
      { // 9
        log_error("StringArray parameter type not yet implemented!");
        eventAddParamStringArray(nEvent, pname, pvalue);
      }
      else
      {
        log_error("Unsupported parameter type for event!");
        return NULL;
      }

      // free temporary variables
      free(pname);
      free(pvalue);
      free(ptypeS);
    }
  }

  // parsing event timestamp
  nEvent->timestamp = timestampNew();
  /* deactivated, not yet fully implemented
  xmlChar *cTimeStamp = xmlGetProp(node, "timestamp");
  if(cTimeStamp!=NULL)
  {
    log_trace("Found timestamp: %s", cTimeStamp);
    // strptime
    /// @todo need to parse string manually to convert to timestamp_ptr ?!
  }*/
  log_debug("Successfully parsed XML and created event referencing action \"%s\"",nEvent->actionDesc->actionName);
  return nEvent;
}

void eventCreateXMLparams(gpointer key, gpointer value, gpointer userData)
{
  xmlNodePtr paramNode = NULL;
  eventParam_ptr curParam = (eventParam_ptr)value;
  if(curParam==NULL) {log_error("Cannot create XML-document for NULL param!"); return;}
  paramNode = xmlNewChild( (xmlNodePtr)userData, NULL, "parameter", NULL);
  xmlNewProp(paramNode, "name", curParam->paramDesc->name);
  if(curParam->paramType==PARAM_STRING) xmlNewProp(paramNode, "value", curParam->paramValue->paramString);
  else if(curParam->paramType==PARAM_INT || curParam->paramType==PARAM_BOOL)
  {
    char *tmpString=malloc(16*sizeof(char));
    sprintf(tmpString,"[%ld]", curParam->paramValue->paramInt);
    xmlNewProp(paramNode, "value", tmpString);
    //free(tmpString);??
  }
  else if(curParam->paramType==PARAM_LONG)
  {
    char *tmpString=malloc(16*sizeof(char));
    sprintf(tmpString,"[%lld]", curParam->paramValue->paramLong);
    xmlNewProp(paramNode, "value", tmpString);
    //free(tmpString);??
  }
  else
  {
    log_error("Unsupported parameter type for event!");
    xmlNewProp(paramNode, "value", "UNSUPPORTED_VALUE");
  }
  xmlNewProp(paramNode, "type", paramTypeStr[curParam->paramType]);
}

unsigned int eventCreateXMLdoc(event_ptr levent)
{
  checkNullInt(levent, "Could not create XML representation of NULL event");
  xmlDocPtr xmldoc = NULL;
  xmlNodePtr eventNode = NULL;
  xmldoc = xmlNewDoc("1.0");
  eventNode = xmlNewNode(NULL, "event");
  xmlDocSetRootElement(xmldoc, eventNode);
  xmlNewProp(eventNode, "action", levent->actionDesc->actionName);
  xmlNewProp(eventNode, "isTry", boolStr[levent->isTry]);

  g_hash_table_foreach(levent->params, (GHFunc)eventCreateXMLparams, eventNode);
  //dumpXMLdoc(xmldoc);
  levent->xmlDoc=xmldoc;
  return R_SUCCESS;
}



event_ptr eventParseXML_AAL(actionDescStore_ptr action_desc_store, xmlDocPtr xmlDoc)
{
  checkNullPtr(action_desc_store, "Failure parsing given event: action_desc_store is null!");
  checkNullPtr(xmlDoc, "Failure parsing given event: xmlDoc is null!");

  xmlNodePtr node = xmlDocGetRootElement(xmlDoc);
  event_ptr nEvent = NULL;
  xmlNodePtr node0;
  bool isTry;

  char *name, *cisTry;
  name=xmlGetProp(node,"topic");
  checkNullPtr(name, "Failure parsing event: no referenced action found!")

  cisTry=xmlGetProp(node, "isTry");
  isTry=(strncmp(cisTry == NULL ? "" : cisTry,"true",4)==0 ? 1 : 0);

  actionDescription_ptr actionDescription = actionDescriptionFind(action_desc_store, name, TRUE);
  checkNullPtr(actionDescription, "Referenced action description not found in store and could not be created?!");

  // free temporary variables
  free(name);
  free(cisTry);

  nEvent=eventNew(actionDescription, isTry);
  checkNullPtr(nEvent, "Failure parsing event: event_match_op could not be created!");
  nEvent->xmlDoc=xmlDoc;

  // parsing timestamp
  nEvent->timestamp = timestampNew();
  char *producer=xmlGetProp(node, "producer");
  if(producer!=NULL)
  {
    paramDescription_ptr param_desc = actionDescFindParam(actionDescription, "producer", TRUE);
    if(param_desc==NULL) {log_error("Error finding parameter description"); return NULL;}

    eventAddParamString(nEvent, "producer", producer);
    free(producer);
  }

  node0=node;
  for(; node0; node0=(node0==node ? node0->children : node0->next))
  {
    if(xmlIsElement(node0) && !xmlStrncasecmp(node0->name, "payload", 7))
    {
      log_trace("payload-node found in event");
      // retrieve content from <payload>-node
      char *payloadContent=NULL;
      if(node0->children->type==XML_TEXT_NODE) payloadContent=node0->children->content;

      log_trace("payload-content=[%s]", payloadContent);
      eventAddParamString(nEvent, "payload", payloadContent);
      break;
    }
  }

  /* deactivated, not yet fully implemented
  xmlChar *cTimeStamp = xmlGetProp(node, "timestamp");
  if(cTimeStamp!=NULL)
  {
    log_trace("Found timestamp: %s", cTimeStamp);
    // strptime
    /// @todo need to parse string manually to convert to timestamp_ptr ?!
  }*/
  log_debug("Successfully parsed XML and created event referencing action \"%s\"",nEvent->actionDesc->actionName);
  //event_print(curEvent);
  return nEvent;
}






