/**
 * @file  executeAction.c
 * @brief Implementation of execution actions, i.e. additional actions which should be executed
 *
 * @see executeAction.h, executeAction_s
 *
 * @author cornelius moucha
 **/

#include "executeAction.h"
#include "log_executeAction_pef.h"

extern pdp_ptr pdp;

/******************************************************************************
 *  executeAction implementation
******************************************************************************/
executeAction_ptr executeActionNew(const char *name)
{
  checkNullPtr(name, "ExecuteAction requires field 'name'!");
  actionDescription_ptr laction = actionDescriptionFind(pdp->actionDescStore, name, FALSE);
  if(laction == NULL)
  {
    log_trace("Mentioned execute action not present in action description store -> adding it");
    laction = actionDescriptionNew((char*)name, ACTION_USAGE);
    if(pdp->actionDescStore->add(pdp->actionDescStore, laction)!=R_SUCCESS)
    {
      log_error("Failure parsing event: action_description could not be added to store!");
      return NULL;
    }
  }
  executeAction_ptr nAction=memAlloc(sizeof(executeAction_t));
  checkNullPtr(nAction,"Could not allocate memory for action");
  nAction->actionDesc=laction;
  nAction->cntParams=0;
  nAction->params=NULL;
  log_trace("%s - successfully created new action=[%s]", __func__, name);
  return nAction;
}

unsigned int executeActionFree(executeAction_ptr action)
{
  unsigned int a;
  for(a=0; a<action->cntParams; a++)
    if(parameterInstanceFree(action->params[a])) return R_ERROR;

  free(action->params);
  free(action);
  return R_SUCCESS;
}

unsigned int executeActionAddParamInstance(executeAction_ptr lAction, const char *name,
                                           const char *value, const char *type)
{
  checkNullInt(lAction, "%s - lAction is NULL",__func__);
  checkNullInt(name, "%s - name is NULL",__func__);
  checkNullInt(value, "%s - value is NULL",__func__);
  checkNullInt(lAction->actionDesc, "%s - referenced action is NULL?!", __func__);

  lAction->params=realloc(lAction->params,(lAction->cntParams+1)*sizeof(parameterInstance_ptr));
  checkNullInt(lAction->params,"Could not allocate memory for new parameter");

  /// @todo change to insert=TRUE
  paramDescription_ptr lparam = actionDescFindParam(lAction->actionDesc, (char*)name, FALSE);
  if(lparam==NULL)
  {
    if(actionDescAddParam(lAction->actionDesc, name, "string")!=R_SUCCESS)
    {log_error("Parameter Description could not be added"); return R_ERROR;}

    lparam = actionDescFindParam(lAction->actionDesc, name, FALSE);
  }

  lAction->params[lAction->cntParams]=parameterInstanceNew(lparam,value,type);
  checkNullInt(lAction->params[lAction->cntParams],"Could not add parameter instance to mechanism_actions");

  log_debug("Successfully added parameter instance \"%s\" with value \"%s\", type=[%d] to executeAction",
              lAction->params[lAction->cntParams]->paramDesc->name,
              lAction->params[lAction->cntParams]->value,
              lAction->params[lAction->cntParams]->type);
  lAction->cntParams++;
  return R_SUCCESS;
}

unsigned long executeActionMemSize(const executeAction_ptr action)
{ /// @todo not yet implemented
  unsigned long size=sizeof(executeAction_ptr);
  //size+=(strlen(action->name)+1)*sizeof(unsigned char);
  size+=sizeof(unsigned char*);
  size+=sizeof(unsigned int);
  //unsigned int a;
  //for(a=0; a<action->n_params; a++)
  //  size+=param_getMemSize(action->params[a]);
  return size;
}

unsigned int executeActionParseXML(mechanism_ptr curMech, const xmlNodePtr rootNode)
{ // node = XMLnode <action>
  checkNullInt(curMech, "%s - curMech is NULL", __func__);
  checkNullInt(rootNode, "%s - node is NULL", __func__);

  xmlNodePtr node=rootNode->children;
  while(node!=NULL)
  {
    if(xmlIsElement(node) && !xmlStrcmp(node->name, "executeAction"))
    {
      char *actionName=xmlGetProp(node, "name");
      mechanismAddExecuteAction(curMech, node, actionName);
      log_trace("Successfully parsed executeAction [%s]", actionName);

      char *pxpInterface=xmlGetProp(node, "pxp");
      // Processing interface if present in actionDescription
      if(pxpInterface!=NULL)
      {
        log_trace("Processing given PXP interface [%s] for action [%s]", pxpInterface, actionName);
        unsigned int ret=pdpRegisterExecutor(actionName, pxpInterface);
        log_debug("Action registration for action [%s] via PXP interface [%s] => [%u]", actionName, pxpInterface, ret);
        xmlFree(pxpInterface);
      }
      else log_trace("NO PXP interface specified for executeAction [%s]...", actionName);

      xmlFree(actionName);
    }
    node=node->next;
  }

  return R_SUCCESS;
}

/// @todo Optimization: Restrict xpath-type for parameterInstance to paramInstances WITHIN authorizationActions!
/// @todo Additional: xpath-type for parameterInstances also for modifiers?
parameterInstance_ptr parameterInstanceNew(paramDescription_ptr paramDesc, const char *value, const char *type)
{
  checkNullPtr(paramDesc, "%s - lparam is NULL", __func__);
  checkNullPtr(paramDesc, "%s - value is NULL", __func__);
  parameterInstance_ptr nparamInstance = memAlloc(sizeof(parameterInstance_t));
  nparamInstance->paramDesc=paramDesc;
  nparamInstance->value=strdup(value);
  nparamInstance->evalValue=NULL;
  if(type!=NULL)
  {
    if(strncasecmp(type, "xpath", 5)==0)
    {
      nparamInstance->type=PARAM_INSTANCE_XPATH;
      nparamInstance->evalValue=nparamInstance->value;
    }
    else
    {
      log_warn("Unsupported type for parameter instance specified [%s]!", type);
      nparamInstance->type=PARAM_INSTANCE_STRING;
    }
  }
  else nparamInstance->type=PARAM_INSTANCE_STRING;
  return nparamInstance;
}

unsigned int parameterInstanceFree(parameterInstance_ptr param)
{
  if(param->value!=NULL) free(param->value);
  if(param->evalValue!=NULL) free(param->evalValue);
  free(param);
  return R_SUCCESS;
}



