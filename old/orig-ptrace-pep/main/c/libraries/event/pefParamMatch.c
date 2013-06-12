/**
 * @file    pefParamMatch.c
 * @brief   Implementation of parameter matching
 *
 * @see pefParamMatch.h
 *
 * @author cornelius moucha
 **/

#include "pefParamMatch.h"
#include "log_pefParamMatch_pef.h"


/// @todo introduce event->getParameterValue (which internally selects appropriate type for returning
///       (e.g. placeholder is returned as %s %d, ... in different method?)
bool matchEventParameters(eventMatch_ptr matchEvent, event_ptr event)
{
  if(matchEvent==NULL || event==NULL) {log_error("Cannot match NULL parameter; return FALSE"); return FALSE;}
  log_debug("%s - comparing trigger event (nParam=[%d]) with desired event(nParam=[%d])", __func__,
            g_hash_table_size(matchEvent->matchParams), g_hash_table_size(event->params) );

  // using g_hash_table_foreach is not advisable, as it cannot stop searching if already the first param doesn't match
  GHashTableIter paramIterator;
  g_hash_table_iter_init(&paramIterator, matchEvent->matchParams);

  paramMatch_ptr paramMatch=NULL;
  gpointer key, value;
  while(g_hash_table_iter_next(&paramIterator, &key, &value))
  {
    paramMatch=(paramMatch_ptr)value;
    log_debug("param [%s]: param value for matching, calling specific matching function...", paramMatch->paramDesc->name);
    int ret=paramMatch->match(event, paramMatch);

    if((paramMatch->negate^ret)==TRUE) log_debug("Parameter [%s] match", paramMatch->paramDesc->name);
    else
    {
      log_debug("Parameter [%s] does not match", paramMatch->paramDesc->name);
      return FALSE;
    }
  }
  return TRUE;
}

bool paramMatch_int(event_ptr curEvent, paramMatch_ptr matchParam)
{
  if(curEvent==NULL || matchParam==NULL) return FALSE;
  log_debug("Matching parameter using integer compare");
  eventParam_ptr eventParam=eventFindParam(curEvent, matchParam->paramDesc->name);
  if(eventParam==NULL) return FALSE;

  if(eventParam->paramType==PARAM_INT || eventParam->paramType==PARAM_BOOL)
  {
    // expect eventMatch-paramType to be INT or BOOL
    if(matchParam->type!=PARAM_INT && matchParam->type!=PARAM_BOOL) {log_warn("Incompatible parameter types for matching?!"); return FALSE;}
    log_debug("Comparing paramMatch-Integer [%d] with event-Integer [%d]", eventParam->paramValue->paramInt, matchParam->value->paramInt);
    return eventParam->paramValue->paramInt==matchParam->value->paramInt;
  }
  else if(eventParam->paramType==PARAM_LONG)
  {
    // expect eventMatch-paramType to be LONG
    if(matchParam->type!=PARAM_LONG) {log_warn("Incompatible parameter types for matching?!"); return FALSE;}
    log_debug("Comparing paramMatch-Long [%lld] with event-Long [%lld]", eventParam->paramValue->paramLong, matchParam->value->paramLong);
    return eventParam->paramValue->paramLong==matchParam->value->paramLong;
  }

  log_warn("unknown parameter type! aborting with FALSE");
  return FALSE;
}

bool paramMatch_context(event_ptr curEvent, paramMatch_ptr matchParam)
{
  if(curEvent==NULL || matchParam==NULL) return FALSE;
  log_debug("Matching context-parameter");

  log_debug("pipRequest: [%s,%s]", matchParam->paramDesc->name, matchParam->value->paramString);
  int ret=pipEval(matchParam->paramDesc->name, matchParam->value->paramString);
  log_debug("pipResponse=[%d]", ret);
  return ret;
}

#ifndef PEF_LOGANDROID
  bool paramMatch_dataUsage(event_ptr curEvent, paramMatch_ptr matchParam)
  {
    if(curEvent==NULL || matchParam==NULL) return FALSE;
    eventParam_ptr eventParam=eventFindParam(curEvent, matchParam->paramDesc->name);
    log_debug("Matching dataUsage-parameter");

    if(DATA_FLOW_ENABLED==1)
    { /// @todo dataUsage-parameter are only matched if DATA_FLOW_ENABLED is enabled? use existing stub-method!
      if(eventParam->paramType!=PARAM_STRING)
      {
        log_error("trying to compare INT-PARAM as DATAUSAGE -> not yet supported");
        return FALSE;
      }

      bool pipResponse=representationRefinesData(eventParam->paramValue->paramString, matchParam->value->paramString);
      log_warn("pip returned=[%s]",boolStr[pipResponse]);
      if(pipResponse==TRUE)
      {
        log_info("  [%s] \"refines\" [%s]", eventParam->paramValue->paramString, matchParam->value->paramString);
        return TRUE;
      }
      else
      {
        log_info("  [%s] doesn't \"refine\" [%s]", eventParam->paramValue->paramString, matchParam->value->paramString);
        return FALSE;
      }
    }
    return FALSE;
  }
#else
  // dataUsage compare method for Android
  bool paramMatch_dataUsage(event_ptr curEvent, paramMatch_ptr matchParam)
  {
    if(curEvent==NULL || matchParam==NULL) return FALSE;
    eventParam_ptr eventParam=eventFindParam(curEvent, matchParam->paramDesc->name);
    log_debug("Matching Android dataUsage eventMatch=[%s] against event=[%s]", matchParam->value->paramString,
              eventParam->paramValue->paramString);

    long matchValue;
    if(matchParam->type==PARAM_STRING) matchValue=strtol(matchParam->value->paramString, NULL, 10);
    else matchValue=matchParam->value->paramInt;

    long eventValue;
    if(matchParam->type==PARAM_STRING) eventValue=strtol(eventParam->paramValue->paramString, NULL, 10);
    else eventValue=eventParam->paramValue->paramInt;

    int ret=(matchValue & eventValue) == matchValue;
    log_debug("Matching result=[%d]", ret);

    return ret;
  }
#endif

bool paramMatch_re(event_ptr curEvent, paramMatch_ptr matchParam)
{
  if(curEvent==NULL || matchParam==NULL) return FALSE;
  eventParam_ptr eventParam=eventFindParam(curEvent, matchParam->paramDesc->name);
  if(eventParam==NULL) {log_error("ERROR: required parameter [%s] not found in event!", matchParam->paramDesc->name); return FALSE;}
  log_debug("Matching regular expression eventMatch=[%s] against event=[%s]", matchParam->value->paramString, eventParam->paramValue->paramString);

  // expect paramType to be STRING
  if(eventParam->paramType!=PARAM_STRING) {log_warn("Incompatible parameter types for matching?!"); return FALSE;}

  int val=regexec(matchParam->re, eventParam->paramValue->paramString, 0, NULL, 0);
  if(val==0) {log_debug("regex match event paramter"); return TRUE;}
  else if(val==REG_NOMATCH) {log_debug("regex does NOT match event parameter"); return FALSE;}

  char buf[128];
  regerror(val, matchParam->re, buf, sizeof(buf));
  log_error("Error executing regex=[%s]", buf);
  return FALSE;
}

bool paramMatch_xpath(event_ptr curEvent, paramMatch_ptr matchParam)
{
  if(curEvent==NULL || matchParam==NULL) return FALSE;
  log_debug("Matching xpath-parameter=[%s]", matchParam->value->paramString);

  //if(matchParam->type!=PARAM_XPATH) {log_warn("Incompatible parameter types for matching?! Expected XPATH, but found=[%s]", paramTypeStr[matchParam->type]); return FALSE;}
  // expect paramType to be STRING
  if(matchParam->type!=PARAM_STRING) {log_warn("Incompatible parameter types for matching?!"); return FALSE;}

  if(curEvent->xmlDoc==NULL) eventCreateXMLdoc(curEvent);
  int ret=xpathEval(curEvent->xmlDoc, (matchParam->value->paramString));
  log_debug("Result of xpath evaluation: [%d]", ret);
  return ret;
}


bool paramMatch_compare(event_ptr curEvent, paramMatch_ptr matchParam)
{
  if(curEvent==NULL || matchParam==NULL) return FALSE;
  log_debug("Matching parameter using common compare");
  /// @todo if trigger parameter (param_match-param_desc->name) not present in event -> ask PIP??
  eventParam_ptr eventParam=eventFindParam(curEvent, matchParam->paramDesc->name);
  if(eventParam==NULL) return FALSE;

  if(eventParam->paramType==PARAM_STRING)
    return (strcmp(eventParam->paramValue->paramString, matchParam->value->paramString) == 0);
  else
  {
    log_error("Incompatible parameter type! aborting with FALSE");
    return FALSE;
  }
}

void paramMatchFree(gpointer data, gpointer userData)
{
  paramMatch_ptr paramMatch=(paramMatch_ptr)data;
  if(paramMatch==NULL) {log_error("Trying to free a NULL event parameter matching operator, aborting..."); return;}

  if(paramMatch->type==PARAM_CONTEXT && strncasecmp(paramMatch->paramDesc->name, "location", 8)==0)
  {
    log_info("deregistering from locationService");
    char *ret=pipInit("stopLocationService","");
    log_info("received answer for pip-request=[%s]", ret);
    free(ret);
  }
  else if(paramMatch->type==PARAM_CONTEXT)
  {
    log_info("deregistering from context for UUID [%s]", paramMatch->value->paramString);
    char *ret=pipInit("removeContext", paramMatch->value->paramString);
    log_info("received answer for pip-request=[%s]", ret);
    free(ret);
  }
  else if(paramMatch->type==PARAM_REGEX && paramMatch->re!=NULL)
  {
    regfree(paramMatch->re);
    free(paramMatch->re);
  }

  log_trace("Freeing event parameter matching [%s]", paramMatch->paramDesc->name);

  // Freeing eventParamValue_ptr  value;
  switch(paramMatch->type)
  {
    case PARAM_STRING:
    case PARAM_DATAUSAGE:
    case PARAM_XPATH:
    case PARAM_REGEX:
    case PARAM_CONTEXT:   free(paramMatch->value->paramString); break;
    case PARAM_BINARY:
    case PARAM_STRINGARRAY:
      /// @todo binary and stringArray type not yet implemented!
      break;
    default: break;
  }
  memFree(paramMatch->value);
  memFree(paramMatch);
}




