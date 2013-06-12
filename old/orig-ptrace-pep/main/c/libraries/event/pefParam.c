/**
 * @file  pefParam.c
 * @brief Implementation for event parameter processing
 *
 * @see pefParam.h, event_s
 *
 * @author cornelius moucha
**/

#include "pefParam.h"
#include "log_pefParam_pef.h"

eventParam_ptr eventFindParam(event_ptr event, char *paramName)
{
  checkNullPtr(paramName, "Trying to find an event parameter with NULL parameter name, returning NULL");
  checkNullPtr(event, "Trying to find an event parameter in NULL event");

  eventParam_ptr event_param =(eventParam_ptr)g_hash_table_lookup(event->params, paramName);
  if(event_param == NULL) log_debug("Event parameter [%s] not found, returning NULL", paramName);
  else log_debug("Event parameter [%s] found", paramName);
  return event_param;
}

unsigned int eventAddParam(event_ptr event, paramDescription_ptr paramDesc, unsigned int paramType, eventParamValue_ptr paramValue)
{
  checkNullInt(event,     "Could not add parameter to NULL event");
  checkNullInt(paramDesc, "Adding parameter to event requires a parameter description (given NULL)");
  checkNullInt(paramValue,"Adding parameter to event requires a parameter value (given NULL)");




  eventParam_ptr eventParam=(eventParam_ptr)memAlloc(sizeof(eventParam_t));
  eventParam->paramDesc=paramDesc;
  eventParam->paramType=paramType;
  eventParam->paramValue=paramValue;



  g_hash_table_insert(event->params, paramDesc->name, eventParam);
  if(eventParam->paramType==PARAM_STRING)
    log_trace("Adding parameter [%s][%s][type:%d] to event", paramDesc->name, eventParam->paramValue->paramString, eventParam->paramType);
  else if(eventParam->paramType==PARAM_INT)
    log_trace("Adding parameter [%s][%d][type:%d] to event", paramDesc->name, eventParam->paramValue->paramInt, eventParam->paramType);
  else if(eventParam->paramType==PARAM_LONG)
    log_trace("Adding parameter [%s][%lld][type:%d] to event", paramDesc->name, eventParam->paramValue->paramLong, eventParam->paramType);
  else if(eventParam->paramType==PARAM_BINARY || eventParam->paramType==PARAM_STRINGARRAY)
    log_trace("Adding parameter [%s][XXX][type:%d] to event (not yet supported!)", paramDesc->name, eventParam->paramType);

  return R_SUCCESS;
}

unsigned int    eventAddParamInt(event_ptr event, const char *paramName, long value)
{
  checkNullInt(event,     "Could not add parameter to NULL event");
  checkNullInt(paramName, "Adding parameter to event requires a parameter name");

  eventParamValue_ptr paramValue=(eventParamValue_ptr)memAlloc(sizeof(eventParamValue_t));

  paramValue->paramInt=value;
  paramDescription_ptr paramDesc=actionDescFindParam(event->actionDesc, paramName, TRUE);

  eventAddParam(event, paramDesc, PARAM_INT, paramValue);
  return R_SUCCESS;
}

unsigned int    eventAddParamLong(event_ptr event, const char *paramName, int64_t value)
{
  checkNullInt(event,     "Could not add parameter to NULL event");
  checkNullInt(paramName, "Adding parameter to event requires a parameter name");


  eventParamValue_ptr paramValue=(eventParamValue_ptr)memAlloc(sizeof(eventParamValue_t));
  paramValue->paramLong=value;
  paramDescription_ptr paramDesc=actionDescFindParam(event->actionDesc, paramName, TRUE);
  eventAddParam(event, paramDesc, PARAM_LONG, paramValue);
  return R_SUCCESS;
}


unsigned int    eventAddParamString(event_ptr event, const char *paramName, const char *value)
{
  checkNullInt(event, "Could not add parameter to NULL event");
  checkNullInt(paramName, "Adding parameter to event requires a parameter name");
  checkNullInt(value, "Adding parameter to event requires a parameter value");


  eventParamValue_ptr paramValue=(eventParamValue_ptr)memAlloc(sizeof(eventParamValue_t));
  paramValue->paramString=strdup(value);
  paramDescription_ptr paramDesc=actionDescFindParam(event->actionDesc, paramName, TRUE);
  eventAddParam(event, paramDesc, PARAM_STRING, paramValue);
  return R_SUCCESS;
}

unsigned int    eventAddParamBinary(event_ptr event, const char *paramName, const void *value, unsigned long size)
{
  checkNullInt(event, "Could not add parameter to NULL event");
  checkNullInt(paramName, "Adding parameter to event requires a parameter name");
  checkNullInt(value, "Adding parameter to event requires a parameter value");

  log_error("Binary data is not yet supported; returning R_ERROR");
  return R_ERROR;

  //event_paramType_ptr paramValue=(event_paramType_ptr)mem_alloc(sizeof(event_paramType_t));
  //void* paramBinaryValue=(void*)mem_calloc(size, sizeof(char));
  //memcpy(paramBinaryValue, value, size);
  //paramValue->paramBinary=paramBinaryValue;
  //paramDescription_ptr paramDesc=paramDescriptionFind(event->actionDesc, paramName, TRUE);
  //addParam(event, paramDesc, usageType, PARAM_STRING, paramValue); // size not yet considered!
  //return R_SUCCESS;
}

unsigned int    eventAddParamStringArray(event_ptr event, const char *paramName, const char *value)
{
  checkNullInt(event, "Could not add parameter to NULL event");
  checkNullInt(paramName, "Adding parameter to event requires a parameter name");
  checkNullInt(value, "Adding parameter to event requires a parameter value");

  log_error("String array data is not yet supported; returning R_ERROR");
  return R_ERROR;

  //event_paramType_ptr paramValue=(event_paramType_ptr)mem_alloc(sizeof(event_paramType_t));
  //paramValue->paramString=strdup(value);
  //paramDescription_ptr paramDesc=paramDescriptionFind(event->actionDesc, paramName, TRUE);
  //addParam(event, paramDesc, usageType, PARAM_STRING, paramValue);
  //return R_SUCCESS;
}

eventParamValue_ptr eventGetParamValue(event_ptr event, char *paramName)
{
  checkNullPtr(event, "Could not search for parameter in NULL event");
  checkNullPtr(paramName, "Could not search for parameter without name");

  eventParam_ptr curParam=(eventParam_ptr)g_hash_table_lookup(event->params, paramName);
  if(curParam == NULL)
    log_debug("Event parameter [%s] not found, returning NULL", paramName);
  else
  {
    log_debug("Event parameter [%s] found", paramName);
    return curParam->paramValue;
  }
  return NULL;
}

char *eventGetParamValueString(event_ptr event, char *paramName)
{
  checkNullPtr(event, "Could not search for parameter in NULL event");
  checkNullPtr(paramName, "Could not search for parameter without name");
  eventParam_ptr curParam=(eventParam_ptr)g_hash_table_lookup(event->params, paramName);
  if(curParam == NULL)
    log_debug("Event parameter [%s] not found, returning NULL", paramName);
  else
  {
    log_debug("Event parameter [%s] found", paramName);
    if(curParam->paramType!=PARAM_STRING) log_warn("Wrong parameter type: expected STRING, found=[%s]", paramTypeStr[curParam->paramType]);
    else return curParam->paramValue->paramString;
  }
  return NULL;
}

long eventGetParamValueInt(event_ptr event, char *paramName)
{
  if(event==NULL) {log_error("Could not search for parameter in NULL event"); return -1;}
  if(paramName==NULL) {log_error("Could not search for parameter without name"); return -1;}
  eventParam_ptr curParam=(eventParam_ptr) g_hash_table_lookup(event->params, paramName);
  if(curParam==NULL)
    log_debug("Event parameter [%s] not found, returning NULL", paramName);
  else
  {
    log_debug("Event parameter [%s] found", paramName);
    if(curParam->paramType!=PARAM_INT) log_warn("Wrong parameter type: expected INT, found=[%s]", paramTypeStr[curParam->paramType]);
    else return curParam->paramValue->paramInt;
  }
  return -1;
}

long long eventGetParamValueLong(event_ptr event, char *paramName)
{
  if(event==NULL) {log_error("Could not search for parameter in NULL event"); return LONG_MIN;}
  if(paramName==NULL) {log_error("Could not search for parameter without name"); return LONG_MIN;}
  eventParam_ptr curParam=(eventParam_ptr) g_hash_table_lookup(event->params, paramName);
  if(curParam==NULL)
    log_debug("Event parameter [%s] not found, returning NULL", paramName);
  else
  {
    log_debug("Event parameter [%s] found", paramName);
    if(curParam->paramType!=PARAM_LONG) log_warn("Wrong parameter type: expected LONG, found=[%s]", paramTypeStr[curParam->paramType]);
    else return curParam->paramValue->paramLong;
  }
  return LONG_MIN;
}

int eventGetParamValueBool(event_ptr event, char *paramName)
{
  if(event==NULL) {log_error("Could not search for parameter in NULL event"); return -1;}
  if(paramName==NULL) {log_error("Could not search for parameter without name"); return -1;}
  eventParam_ptr curParam=(eventParam_ptr) g_hash_table_lookup(event->params, paramName);
  if(curParam==NULL)
    log_debug("Event parameter [%s] not found, returning NULL", paramName);
  else
  {
    log_debug("Event parameter [%s] found", paramName);
    if(curParam->paramType!=PARAM_BOOL) log_warn("Wrong parameter type: expected BOOL, found=[%s]", paramTypeStr[curParam->paramType]);
    else return curParam->paramValue->paramInt;
  }
  return -1;
}

void eventParamLog(gpointer key, gpointer value, gpointer userData)
{
  eventParam_ptr eventParam=(eventParam_ptr)value;
  if(eventParam==NULL) {log_warn("Cannot log NULL-parameter"); return;}
  /// @todo update: prepare enum PARAM_TYPE_STR("%s","%d","%x","%p") and include in user_data dynamic selection
  log_trace(userData, eventParam->paramDesc->name);
  if(eventParam->paramType == PARAM_STRING)
    log_trace("     [%s] (dataType: %s)", eventParam->paramValue->paramString, paramTypeStr[eventParam->paramType]);
  else if(eventParam->paramType == PARAM_INT || eventParam->paramType==PARAM_BOOL)
    log_trace("     [%d] (dataType: %s)", eventParam->paramValue->paramInt, paramTypeStr[eventParam->paramType]);
  else if(eventParam->paramType == PARAM_LONG)
    log_trace("     [%lld] (dataType: %s)", eventParam->paramValue->paramLong, paramTypeStr[eventParam->paramType]);
  else log_trace("     [%p]\n", eventParam->paramValue->paramString);
}

/// @todo freeing not yet ported to new parameterType stuff!
void eventParamFree(gpointer data, gpointer userData)
{
  eventParam_ptr eventParam=(eventParam_ptr)data;
  if(eventParam==NULL) {log_error("Trying to free a NULL event parameter, aborting..."); return;}

  log_trace("Freeing event parameter [%s]", eventParam->paramDesc->name);
  if(eventParam->paramType==PARAM_STRING)
  {
    log_trace("Freeing parameter [%s][%s][type:%d]", eventParam->paramDesc->name, eventParam->paramValue->paramString, eventParam->paramType);
    free(eventParam->paramValue->paramString);
  }
  else if(eventParam->paramType==PARAM_INT)
    log_trace("Freeing parameter [%s][%d][type:%d]", eventParam->paramDesc->name, eventParam->paramValue->paramInt, eventParam->paramType);

  free(eventParam->paramValue);
  free(eventParam);
}


