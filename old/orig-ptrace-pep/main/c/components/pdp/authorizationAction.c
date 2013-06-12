/**
 * @file  authorizationAction.c
 * @brief Implementation of authorization actions, i.e. the decision of the PDP for incoming events
 *
 * @see authorizationAction.h, authorizationAction_s
 *
 * @todo Prepare static standard responses AUTHORIZATION_INHIBIT, AUTHORIZATION_ALLOW
 *
 * @author cornelius moucha
 **/

#include "authorizationAction.h"
#include "log_authorizationAction_pef.h"

/******************************************************************************
 *  AuthorizationAction implementation
******************************************************************************/
/// @todo fallback should be set to static standard AUTHORIZATION_INHIBIT
authorizationAction_ptr authorizationActionNew(const char *name, unsigned int response, unsigned int delay, const char *fallback)
{
  authorizationAction_ptr nauthAction=memAlloc(sizeof(authorizationAction_t));
  checkNullPtr(nauthAction, "Could not allocate memory for mechanism_actions");

  nauthAction->name=strdup(name);
  nauthAction->start=FALSE;
  nauthAction->response=response;
  nauthAction->delay=delay;
  nauthAction->cntParamsModify=0;
  nauthAction->paramsModify=NULL;
  if(fallback!=NULL) nauthAction->fallbackName=strdup(fallback);
  else nauthAction->fallbackName=NULL;
  nauthAction->cntExecuteActions=0;
  nauthAction->executeActions=NULL;
  log_debug("Successfully allocated new authorizationAction [%s] (response: %d, delay: %d)", nauthAction->name, nauthAction->response, nauthAction->delay);
  return nauthAction;
}

unsigned int authorizationActionAddModifier(mechanism_ptr curMech, authorizationAction_ptr curMechAction, const char *name,
                                            const char *value, const char *isXPath)
{
  checkNullInt(curMech, "%s - curMech is NULL",__func__);
  checkNullInt(curMechAction, "%s - curMechAction is NULL",__func__);
  checkNullInt(name, "%s - name is NULL",__func__);
  checkNullInt(value, "%s - value is NULL",__func__);

  curMechAction->paramsModify=realloc(curMechAction->paramsModify,(curMechAction->cntParamsModify+1)*sizeof(parameterInstance_ptr));
  checkNullInt(curMechAction->paramsModify,"Could not allocate memory for new parameter");

  checkNullInt(curMech->trigger->matchAction, "%s - trigger action is NULL?!", __func__);
  actionDescription_ptr laction = curMech->trigger->matchAction;
  /// @todo change to insert=TRUE
  paramDescription_ptr lparam = actionDescFindParam(laction, name, FALSE);
  if(lparam==NULL)
  {
    log_trace("Mentioned parameter in modify not present in action_description -> adding it");
    if(actionDescAddParam(laction, (char*)name, "string")!=R_SUCCESS)
    {
      log_error("Parameter Description could not be added");
      return R_ERROR;
    }
    lparam = actionDescFindParam(laction, (char*)name, FALSE);
  }

  curMechAction->paramsModify[curMechAction->cntParamsModify]=parameterInstanceNew(lparam,value,isXPath);
  checkNullInt(curMechAction->paramsModify[curMechAction->cntParamsModify],"Could not add parameter instance to mechanism_actions");

  log_debug("Successfully added parameter description \"%s\" with value \"%s\" to mechanism_actions",name,value);
  curMechAction->cntParamsModify++;
  return R_SUCCESS;
}

unsigned int authorizationActionFree(authorizationAction_ptr authAction)
{
  if(authAction==NULL) return R_SUCCESS;
  unsigned int a;
  for(a=0; a<authAction->cntParamsModify; a++)
    if(parameterInstanceFree(authAction->paramsModify[a])) return R_ERROR;

  free(authAction->name);
  free(authAction->paramsModify);
  free(authAction);
  return R_SUCCESS;
}

unsigned long authorizationActionMemSize(const authorizationAction_ptr authAction)
{
  unsigned long size=sizeof(authorizationAction_ptr);
  size+=4*sizeof(unsigned int);
  //unsigned int a;
  //for(a=0; a<mechaction->n_params_to_modify; a++)
  //  size+=param_getMemSize(mechaction->params_to_modify[a]);
  //size+=sizeof(param_ptr*);
  //for(a=0; a<mechaction->n_actions_to_execute; a++)
  //  size+=actions_getMemSize(mechaction->actions_to_execute[a]);
  size+=sizeof(executeAction_ptr*);
  return size;
}

authorizationAction_ptr authorizationActionProcessXMLnode(mechanism_ptr curMech, const xmlNodePtr node)
{
  xmlNodePtr cur=node->children;
  authorizationAction_ptr curMechAuth=NULL;

  char *authActionName=xmlGetProp(node, "name");
  char *authActionStart=xmlGetProp(node, "start");
  char *authActionFallback=xmlGetProp(node, "fallback");

  while(cur!=NULL)
  {
    if(xmlIsElement(cur))
    {

      if(authActionName==NULL) {log_error("Authorization action requires name attribute!"); return NULL;}

      if(!xmlStrncasecmp(cur->name,"allow",5) || !xmlStrncasecmp(cur->name, "inhibit",7))
      { //PreventiveMechanismType
        unsigned int actiontype=(!xmlStrcmp(cur->name,"allow") ? ACTION_ALLOW : ACTION_INHIBIT);
        uint64_t delay=0;
        xmlNodePtr tmpNode=xmlFindNode(cur, "delay");
        if(tmpNode!=NULL)
        {
          delay=xmlParseTimestepSize(tmpNode);
          log_trace("Delay of AuthorizationAction: %llu",delay);
        }
        curMechAuth=authorizationActionNew(authActionName, actiontype, delay, authActionFallback);
        if(authActionStart!=NULL && !xmlStrncasecmp(authActionStart, "true", 4))
          curMechAuth->start=TRUE;
        xmlFree(authActionName);
        xmlFree(authActionStart);
        xmlFree(authActionFallback);

        if(!xmlStrncasecmp(cur->name, "allow",5) && (tmpNode=xmlFindNode(cur, "modify"))!=NULL)
        {
          xmlNodePtr lcur=tmpNode->children;
          while(lcur!=NULL)
          {
            if(xmlIsElement(lcur) && !xmlStrncasecmp(lcur->name, "parameter",9))
            {
              char *lname=xmlGetProp(lcur,"name"); // required to do separately to avoid memory leak due to libxml2
              char *lvalue=xmlGetProp(lcur,"value");
              char *lisXpath=xmlGetProp(lcur,"isXPath");
              authorizationActionAddModifier(curMech, curMechAuth, lname, lvalue, lisXpath);
              log_trace("Successfully parsed modify parameter: param_desc=[%s], value=[%s], isXPath=[%d]",
                          curMechAuth->paramsModify[curMechAuth->cntParamsModify-1]->paramDesc->name,
                          curMechAuth->paramsModify[curMechAuth->cntParamsModify-1]->value,
                          curMechAuth->paramsModify[curMechAuth->cntParamsModify-1]->isXPath);
              xmlFree(lname);
              xmlFree(lvalue);
              xmlFree(lisXpath);
            }

            lcur=lcur->next;
          }
        }

        if(!xmlStrncasecmp(cur->name, "allow",5))
        {
          tmpNode=cur->children;
          while(tmpNode!=NULL)
          {
            if(!xmlStrncasecmp(tmpNode->name, "executeAction", 13))
            {
              char *execName=xmlGetProp(tmpNode, "name");
              if(execName==NULL) {log_error("Execute action in authorizationAction requires field name!"); return NULL;}
              log_debug("Found an executeAction [%s] in allow node", execName);

              curMechAuth->executeActions=realloc(curMechAuth->executeActions,(curMechAuth->cntExecuteActions+1)*sizeof(executeAction_ptr));
              checkNullPtr(curMechAuth->executeActions, "Could not allocate memory for new executeAction");

              executeAction_ptr nexecAction=executeActionNew(execName);
              xmlNodePtr curParameterNodes=tmpNode->children;
              while(curParameterNodes!=NULL)
              {
                if(xmlIsElement(curParameterNodes) && !xmlStrncasecmp(curParameterNodes->name, "parameter",9))
                {
                  char *lname=xmlGetProp(curParameterNodes,"name");
                  char *lvalue=xmlGetProp(curParameterNodes,"value");
                  //char *lisXPath=xmlGetProp(curParameterNodes,"isXPath");
                  char *ltype=xmlGetProp(curParameterNodes, "type");
                  executeActionAddParamInstance(nexecAction, lname, lvalue, ltype);
                  xmlFree(lname);
                  xmlFree(lvalue);
                  xmlFree(ltype);
                }
                curParameterNodes=curParameterNodes->next;
              }

              char *pxpInterface=xmlGetProp(tmpNode, "pxp");
              // Processing interface if present in actionDescription
              if(pxpInterface!=NULL)
              {
                log_trace("Processing given PXP interface [%s] for executeAction [%s]", pxpInterface, execName);
                unsigned int ret=pdpRegisterExecutor(execName, pxpInterface);
                log_debug("Action registration for action [%s] via PEP interface [%s] => [%u]", execName, pxpInterface, ret);
                xmlFree(pxpInterface);
              }
              else log_warn("NO PEP interface specified for action [%s]...", execName);

              curMechAuth->executeActions[curMechAuth->cntExecuteActions]=nexecAction;
              log_trace("Successfully added executeAction \"%s\" to authorizationAction", execName);
              curMechAuth->cntExecuteActions++;
            }
            tmpNode=tmpNode->next;
          }
        }
        break;
      }
    }
    cur=cur->next;
  }
  return curMechAuth;
}

authorizationAction_ptr authorizationActionParseXML(mechanism_ptr curMech, const xmlNodePtr node)
{ // node = XMLnode <XXXmechanism>
  checkNullPtr(node, "Failure parsing authorizationAction: NULL node!");

  GHashTable *authActions=g_hash_table_new(g_str_hash, g_str_equal);
  authorizationAction_ptr startAuthAction=NULL;
  authorizationAction_ptr curAuthAction=NULL;

  xmlNodePtr cur=node->children;
  unsigned int cnt=0;
  while(cur!=NULL)
  {
    if(!xmlStrncasecmp(cur->name,"authorizationAction", xmlStrlen(cur->name)))
    { // Process authorizationAction
      curAuthAction=authorizationActionProcessXMLnode(curMech, cur);
      if(curAuthAction==NULL) {log_error("Error processing authorizationAction!"); cur=cur->next; continue;}
      if(curAuthAction->start==TRUE) startAuthAction=curAuthAction;
      g_hash_table_insert(authActions, curAuthAction->name, curAuthAction);
      log_debug("Processed authorizationAction [%s] (start=[%u])", curAuthAction->name, curAuthAction->start);
      cnt++;
    }
    cur=cur->next;
  }
  if(cnt==0)
  {
    g_hash_table_destroy(authActions);
    return NULL;
  }
  if(cnt==1)
  {
    log_debug("Just one authorizationAction specified [%s]", curAuthAction->name);
    // nevertheless take implicit INHIBIT in case of error with this authorizationAction:
    authorizationAction_ptr inhibitAuth=authorizationActionNew(strdup("INHIBIT"), ACTION_INHIBIT, 0, NULL);
    curAuthAction->fallback=inhibitAuth;
    // just destroy hashtable without deallocating keys/values
    g_hash_table_destroy(authActions);
    return curAuthAction;
  }

  // Process hierarchy of authorizationActions
  if(startAuthAction==NULL && cnt>1)
    {log_warn("More than 1 authorizationAction specified and no one explicitly specified as starting point!"); return NULL;}

  authorizationAction_ptr nextAuth=startAuthAction;
  while(nextAuth!=NULL)
  {
    log_trace("Processing authorizationAction=[%s], fallback=[%p]", nextAuth->name, nextAuth->fallbackName);
    authorizationAction_ptr fallback=NULL;
    if(nextAuth->fallbackName!=NULL)
    {
      if(strncasecmp(nextAuth->fallbackName, "ALLOW", 5)==0)
      {
        log_trace("Found explicitly specified ALLOW fallback");
        authorizationAction_ptr allowAuth=authorizationActionNew(strdup("ALLOW"), ACTION_ALLOW, 0, NULL);
        nextAuth->fallback=allowAuth;
        free(nextAuth->fallbackName);
        break;
      }
      else if(strncasecmp(nextAuth->fallbackName, "INHIBIT", 7)==0)
      {
        log_trace("Found explicitly specified INHIBIT fallback");
        nextAuth->fallback=NULL;
        free(nextAuth->fallbackName);
        break;
      }

      log_trace("Searching for fallback authAction=[%s]", nextAuth->fallbackName);
      fallback=g_hash_table_lookup(authActions, nextAuth->fallbackName);
      if(fallback==NULL)
      {
        log_error("Specified fallback authorizationAction [%s] not found!", nextAuth->fallbackName);
        g_hash_table_destroy(authActions);
        return NULL;
      }
      log_debug("Fallback authAction [%s] found", fallback->name);
      // Remove entry from hashtable to avoid cycles
      g_hash_table_remove(authActions, nextAuth->fallbackName);
      free(nextAuth->fallbackName);
      nextAuth->fallback=fallback;
    }
    else
    {
      log_trace("NULL fallback found -> implicit INHIBIT");
      //nextAuth->fallback=NULL;
      authorizationAction_ptr inhibitAuth=authorizationActionNew(strdup("INHIBIT"), ACTION_INHIBIT, 0, NULL);
      nextAuth->fallback=inhibitAuth;
      break;
    }
    nextAuth=fallback;
  }
  g_hash_table_destroy(authActions);
  return startAuthAction;
}
