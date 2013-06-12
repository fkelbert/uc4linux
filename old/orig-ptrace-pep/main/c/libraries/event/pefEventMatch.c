/**
 * @file    pefEventMatch.c
 * @brief   Implementation of event matching
 *
 * @see pefEventMatch.h, eventMatch_s
 *
 * @author cornelius moucha
 **/

#include "pefEventMatch.h"
#include "log_pefEventMatch_pef.h"

extern pdp_ptr pdp;

eventMatch_ptr eventMatchNew(actionDescription_ptr matchAction, bool matchTry)
{
  checkNullPtr(matchAction, "NULL action description when creating matching operator, returning NULL");

  if(!(matchTry == TRUE || matchTry == FALSE))
    {log_error("Match try must be TRUE or FALSE, returning NULL"); return NULL;}
  log_info("Creating event matching operator action=[%s] match_try=[%s]",
           matchAction->actionName, boolStr[matchTry]);

  eventMatch_ptr eventMatch;
  eventMatch=memAlloc(sizeof(eventMatch_t));
  checkNullPtr(eventMatch, "Could not allocate memory for event matching operator");

  eventMatch->matchAction=matchAction;
  eventMatch->matchTry=matchTry;
  eventMatch->matchParams=g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify) paramMatchFree);
  return eventMatch;
}

unsigned int eventMatchFree(eventMatch_ptr matchEvent, mechanism_ptr curMech)
{
  if(matchEvent==NULL) {log_error("Trying to free a NULL event matching operator, aborting..."); return R_ERROR;}

  log_debug("Freeing event matching operator action=[%s]", matchEvent->matchAction->actionName);

  // unsubscribe the current mechanism from list
  /// todo this removal from the list still required due to separate condition subscription list?!
  matchEvent->matchAction->mechanisms=g_slist_remove(matchEvent->matchAction->mechanisms, (mechanism_ptr)curMech);
  g_hash_table_destroy(matchEvent->matchParams);

  //unsubscribe from PEP
  #if PDP_PEPSUBSCRIPTION == 1
    if(g_slist_length(matchEvent->matchAction->mechanisms)==0)
    {
      unsigned int ret=R_ERROR;
      if(matchEvent->matchAction->pepInterface!=NULL)
        ret=matchEvent->matchAction->pepInterface->pepSubscribe(matchEvent->matchAction->pepInterface,
                                                                matchEvent->matchAction->actionName, 1);
      log_debug("PEP UNsubscription for event name=[%s]=>[%d]", matchEvent->matchAction->actionName, ret);
    }
  #endif

  memFree(matchEvent);
  log_debug("Event matching operator deallocated");
  return R_SUCCESS;
}

eventMatch_ptr eventMatchParseXML(actionDescStore_ptr actionDescStore, xmlNodePtr node)
{
  checkNullPtr(actionDescStore, "Failure parsing given event: action_desc_store is null!");
  checkNullPtr(node,            "Failure parsing given event: node is null!");

  if(xmlStrncasecmp(node->name, "trigger", 7) && xmlStrncasecmp(node->name, "eventMatch", 10))
  {
    log_error("Failure parsing given event: node name neither trigger nor eventMatch (name=[%s]\n!", node->name);
    return NULL;
  }

  xmlNode *node0;
  eventMatch_ptr nEvent=NULL;
  bool isTry;

  char *name, *cisTry;
  name=xmlGetProp(node,"action");
  checkNullPtr(name, "Failure parsing event: no referenced action found in mechanism xml!");

  cisTry=xmlGetProp(node, "isTry");
  isTry =(strncmp(cisTry == NULL ? "" : cisTry,"true",4)==0 ? 1 : 0);
  xmlFree(cisTry);

  actionDescription_ptr actionDescription=actionDescStore->find(actionDescStore, name, TRUE);

  nEvent=eventMatchNew(actionDescription, isTry);
  checkNullPtr(nEvent, "Failure parsing event: event_match_op could not be created!");

  node0=node;
  for( ; node0; node0=(node0==node ? node0->children : node0->next))
  {
    if(xmlIsElement(node0) && !xmlStrncasecmp(node0->name, "paramMatch", 12))
    {
      char *pname   =xmlGetProp(node0, "name");
      char *ptypeS  =xmlGetProp(node0, "type");
      char *pvalue  =xmlGetProp(node0, "value");
      char *cnegate =xmlGetProp(node0, "negate");
      unsigned int ptype=PARAM_STRING;
      bool negate=FALSE;
      if(cnegate!=NULL && (strncmp(cnegate,"true",4)==0 || strncmp(cnegate, "1", 1)==0)) negate=TRUE;
      xmlFree(cnegate);

      // KIT/TUM
      char *pqod = xmlGetProp(node0, "qod");
      char *initDataID = xmlGetProp(node0, "dataID");

      checkNullPtr(pname, "Failure parsing event: paramMatch without required field 'name'!");
      checkNullPtr(pvalue,"Failure parsing event: paramMatch without required field 'value'!");

      paramDescription_ptr lactionParamDesc=actionDescFindParam(actionDescription, pname, TRUE);
      if(paramValueFind(lactionParamDesc, pvalue)==NULL)
      {
        if(addParamValue(actionDescription, pname, pvalue)!=R_SUCCESS)
        {
          log_error("Failure parsing event: param value could not be inserted!");
          return NULL;
        }
      }

      // check parameter type
      if(ptypeS==NULL)
      { // 0 no parameter type given in policy; defaulting to PARAM_STRING
        log_info("no parameter type given in policy; defaulting to PARAM_SRING!");
      }
      else if(strncasecmp(ptypeS, "datausage", 9)==0)
      { // 1
        #ifndef PEF_LOGANDROID
          /// @todo parsing XML for dataUsage: update required...
          ptype=PARAM_DATAUSAGE;
          log_info(" * Parameter [%s] of trigger event [%s] is of type dataUsage",pname,name);

          log_info(" * Storing initial mapping...");
          if(initDataID!=NULL)
          {
            log_info(" * Found given dataID for initialRepresentation: [%s]", initDataID);
            pvalue=(char*)pipInitDataID("initialRepresentation", pvalue, initDataID);
          }
          else pvalue=(char*)pipInit("initialRepresentation", pvalue);
          //else pvalue=(char*)initialRepresentation(pvalue, pqod);

          // old KIT version
          //pvalue=(char*)IF_initCont(pvalue);
          // KIT: following two lines from new (111124) KIT-version!
          ////xmlSetProp(node0, "value", IF_initCont(pvalue));
          //pvalue=(char*)IF_initCont(pvalue, pqod);
        #endif
      }
      else if(strncasecmp(ptypeS, "xpath", 5)==0)
      { // 2
        log_debug("Event-Match parameter is of type: XPATH");
        ptype=PARAM_XPATH;
      }
      else if(strncasecmp(ptypeS, "re", 2)==0)
      { // 3
        log_debug("Event-Match parameter is of type: RegEx");
        ptype=PARAM_REGEX;
      }
      else if(strncasecmp(ptypeS, "context", 7)==0)
      { // 4
        log_debug("Event-Match parameter is of type: CONTEXT");
        ptype=PARAM_CONTEXT;

        // initialization for Context-PIP:
        if(strncasecmp(pname, "location", 8)==0)
        { // special handling (currently) for location in combination with Android
          log_info("trigger parameter is of type=[%s:%s] ==> activating locationService4Android", ptypeS, pname);
          char *retStr=pipInit("startLocationService","");
          log_info("received answer for PIP-request=[%s]", retStr);
          free(retStr);
        }
        else
        { // Generic context: I have to 'deploy' context-Description to PIP!
          log_debug("trigger parameter is of type=[%s:%s] ==> activating deployContext (%s)", ptypeS, pvalue, actionDescription->actionName);
          char *context=g_hash_table_lookup(pdp->contextTable, pvalue);
          if(context==NULL) log_error("ERROR: no context [%s] found in contextDescriptions!", pvalue);
          else
          {
            log_info("pipRequest=[deployContext,%s]", context);
            char *retStr=pipInit("deployContext", context);
            log_info("received answer for pip-request=[%s]", retStr);
            if(retStr!=NULL)
            {
              xmlFree(pvalue);
              pvalue=retStr;
              log_debug("Substituted context parameter value with ID from PIP=[%s]", pvalue);
            }
            else log_error("ERROR: could not substitute parameter value=[%s] with ID from PIP!", pvalue);
          }
        }
      }
      else if(strncasecmp(ptypeS, "binary", 6)==0)
      { // 5
        log_error("Binary parameter type not yet implemented!");
        return NULL;
      }
      else if(strncasecmp(ptypeS, "int", 2)==0)
      { // 6 pvalue should be stored as INT; done in event_match_add_param method
        ptype=PARAM_INT;
        log_debug("Event-Match parameter is of type: INT");
      }
      else if(strncasecmp(ptypeS, "long", 2)==0)
      { // 7 pvalue should be stored as LONG; done in event_match_add_param method
        ptype=PARAM_LONG;
        log_debug("Event-Match parameter is of type: LONG");
      }
      else if(strncasecmp(ptypeS, "bool", 2)==0)
      { // 8 pvalue should be stored as BOOL (or better as INT); done in event_match_add_param method
        ptype=PARAM_BOOL;
        log_debug("Event-Match parameter is of type: BOOL");
      }
      else if(strncasecmp(ptypeS, "stringarray", 2)==0)
      { // 9
        log_error("StringArray parameter type not yet implemented!");
        return NULL;
      }

      eventMatchAddParam(nEvent, lactionParamDesc, pvalue, ptype, negate);
      xmlFree(pname);
      xmlFree(ptypeS);

      if(pvalue!=NULL) free(pvalue);
    }
  }
  log_info("Successfully parsed XML and created eventMatch operator referencing action \"%s\"",nEvent->matchAction->actionName);

  xmlFree(name);
  return nEvent;
}

unsigned int eventMatchAddParam(eventMatch_ptr eventMatch, paramDescription_ptr paramDesc, char *value, unsigned int type, bool negate)
{
  checkNullInt(eventMatch,"Trying to add parameter to NULL event matching operator, aborting");
  checkNullInt(paramDesc, "Trying to add parameter to event matching operator with NULL parameter description, aborting");
  checkNullInt(value,     "Trying to add parameter to event matching operator with NULL value, aborting");

  log_debug("Adding param [%s] to eventMatch", paramDesc->name);
  paramMatch_ptr paramMatch = (paramMatch_ptr)memAlloc(sizeof(paramMatch_t));
  paramMatch->paramDesc = paramDesc;

  paramMatch->type  =type;
  paramMatch->re    =NULL;
  paramMatch->negate=negate;

  eventParamValue_ptr paramValue=(eventParamValue_ptr)memAlloc(sizeof(eventParamValue_t));
  paramMatch->value=paramValue;

  if(type==PARAM_REGEX)
  { // value is regular expression, pre-compiling for performance reasons
    log_debug("Parameter value is regular expression => precompiling re=[%s]...", value);
    paramMatch->re=memAlloc(sizeof(regex_t));
    int val=regcomp(paramMatch->re, value, 0); /// @todo use REG_EXTENDED | REG_ICASE: to disable case sensitivity and use extended Posix regex-syntax!
    if(val!=0)
    {
      char buf[128];
      regerror(val, paramMatch->re, buf, sizeof(buf));
      log_error("Error pre-compiling regex: [%s]", buf);
      return R_ERROR;
    }
    else log_debug("Regular expression successfully compiled.");
  }

  // setting matching-function for parameter
  paramMatch->match=NULL;
  switch(type)
  {
    case PARAM_STRING:    paramMatch->match=paramMatch_compare;
                          paramMatch->value->paramString=strdup(value);
                          break;
    case PARAM_DATAUSAGE: paramMatch->match=paramMatch_dataUsage;
                          paramMatch->value->paramString=strdup(value);
                          break;
    case PARAM_XPATH:     paramMatch->match=paramMatch_xpath;
                          paramMatch->value->paramString=strdup(value);
                          break;
    case PARAM_REGEX:     paramMatch->match=paramMatch_re;
                          paramMatch->value->paramString=strdup(value);
                          break;
    case PARAM_CONTEXT:   paramMatch->match=paramMatch_context;
                          paramMatch->value->paramString=strdup(value);  // value is already prepared, i.e. initialized with PIP
                          break;
    case PARAM_INT:       paramMatch->match=paramMatch_int;
                          long intVal=strtol(value, NULL, 0);            // select base automatically
                          paramMatch->value->paramInt=intVal;
                          break;
    case PARAM_LONG:      paramMatch->match=paramMatch_int;
                          long long longVal=strtoll(value, NULL, 0);     // select base automatically
                          paramMatch->value->paramInt=longVal;
                          break;
    case PARAM_BOOL:      paramMatch->match=paramMatch_int;
                          unsigned int boolVal=0;                        // check for 0, 1, false, true
                          if(strncasecmp(value, "true", 4)==0) boolVal=TRUE;
                          else if(strncasecmp(value, "false", 5)) boolVal=FALSE;
                          else boolVal=strtol(value, NULL, 0);           // select base automatically
                          paramMatch->value->paramInt=boolVal;
                          break;

    case PARAM_BINARY:
    case PARAM_STRINGARRAY:
    //{
    //  log_error("Unsupported enginge for parameter matching! Defaulting to string-compare");
    //  return R_ERROR;
    //}
    default:
      type=PARAM_STRING;
      log_error("Unsupported enginge for parameter matching! Defaulting to string-compare");
      paramMatch->value->paramString=strdup(value);
      paramMatch->match=paramMatch_compare; break;
  }

  g_hash_table_insert(eventMatch->matchParams, paramDesc->name, paramMatch);
  log_trace("Adding parameter matching [%s][%s][%s][%s] to event", paramDesc->name, value, paramTypeStr[type], boolStr[paramMatch->negate]);
  return R_SUCCESS;
}

bool eventMatches(eventMatch_ptr matchEvent, event_ptr event)
{ // null checking for detective mechanisms, which have no trigger
  if(matchEvent==NULL || event==NULL) return FALSE;

  bool matchResult=TRUE;
  log_debug("Matching event [%d]", event->id);

  if(matchEvent->matchTry == event->isTry)
  {
    log_trace("Event isTry matches operator [%s]=[%s]", boolStr[event->isTry], boolStr[matchEvent->matchTry] );
    if(strcmp(event->actionDesc->actionName, matchEvent->matchAction->actionName)== 0)
    {
      log_trace("Event action matches [%s]=[%s]", event->actionDesc->actionName, matchEvent->matchAction->actionName);
      // Matching event parameters
      matchResult=matchEventParameters(matchEvent, event);
    }
    else
    {
      log_trace("Event action does not match [%s]!=[%s]", event->actionDesc->actionName, matchEvent->matchAction->actionName);
      matchResult=FALSE;
    }
  }
  else
  {
    log_trace("Event isTry does not match operator [%s]!=[%s]", boolStr[event->isTry], boolStr[matchEvent->matchTry] );
    matchResult=FALSE;
  }

  if(matchResult) log_debug("Event matches operator");
  else            log_debug("Event does not match operator");
  return matchResult;
}








