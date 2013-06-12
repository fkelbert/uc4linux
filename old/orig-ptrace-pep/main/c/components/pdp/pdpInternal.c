/**
 * @file  pdpInternal.c
 * @brief Implementation of the PDP (internal methods)
 *
 * @see pdp.h, pdpInternal.h
 *
 * @author cornelius moucha
**/

#include "pdpInternal.h"
#include "log_pdpInternal_pef.h"

extern pdp_ptr pdp;

// internal pdp methods
unsigned int loadPolicy(const char *xmlFilename, const char *mechanismName)
{
  checkNullInt(xmlFilename, "Error processing loadPolicy: NULL xmlFilename");
  xmlDocPtr    doc=NULL;
  xmlNodePtr   rootNode=NULL;

  log_debug("Loading mechanisms from file: %s (for mechanism=[%s])", xmlFilename, (mechanismName==NULL?"NULL":mechanismName));
  doc=loadXMLdoc(xmlFilename);
  if(doc==NULL) {log_error("Error loading mechanism file!"); return R_ERROR;}
  rootNode=xmlDocGetRootElement(doc);
  unsigned int ret=searchMechanismNodes(rootNode, mechanismName, xmlFilename, FALSE);
  xmlFreeDoc(doc);

  if(ret==R_ERROR) {log_error("Error loading policy from file [%s]", xmlFilename); return R_ERROR;}
  log_info("Successfully loaded mechanisms from file: %s",xmlFilename);
  return R_SUCCESS;
}

unsigned int loadPolicyString(const char *xml, size_t xmlSize, const char *ns, const char *mechanismName)
{
  checkNullInt(xml, "Error processing loadPolicyString: NULL xml-string");
  checkNullInt(ns,  "Error processing loadPolicyString: NULL namespace");
  xmlDocPtr    doc=NULL;
  xmlNodePtr   rootNode=NULL;

  doc=loadXMLmemory(xml, xmlSize, TRUE);
  if(doc==NULL) {log_error("Error loading mechanism file!"); return R_ERROR;}
  rootNode=xmlDocGetRootElement(doc);
  unsigned int ret=searchMechanismNodes(rootNode, mechanismName, ns, FALSE);
  xmlFreeDoc(doc);

  if(ret==R_ERROR) {log_error("Error loading policy from memory"); return R_ERROR;}
  log_info("Successfully loaded mechanisms from ns: %s",ns);
  return R_SUCCESS;
}

unsigned int contextDescriptions_parseXML(xmlNodePtr node)
{
  checkNullInt(node, "Cannot parse NULL xml-node");
  xmlNodePtr node0=node;
  for(; node0; node0=(node0 == node ? node0->children : node0->next))
  {
    if(xmlIsElement(node0) && xmlStrncasecmp(node0->name, "context", strlen(node0->name))==0)
    {
      xmlBufferPtr myxbp=xmlBufferCreate();
      xmlDocPtr tmpdoc = NULL;
      tmpdoc = xmlNewDoc((xmlChar*)"1.0");
      xmlDocSetRootElement(tmpdoc, xmlCopyNode(node0,1));
      xmlNodeDump(myxbp,tmpdoc,node0,1,1);

      char *context=strdup(xmlBufferContent(myxbp));
      char *node0id=xmlGetProp(node0, "id");
      char *contextName=strdup(node0id);
      xmlFree(node0id);
      xmlFreeDoc(tmpdoc);
      xmlBufferFree(myxbp);

      g_hash_table_insert(pdp->contextTable, contextName, context);
      log_info("Context [%s] successfully added to hashtable", contextName);
    }
  }
  return R_SUCCESS;
}

unsigned int actionDescriptions_parseXML(xmlNodePtr node)
{
  checkNullInt(node, "Cannot parse NULL xml-node");
  xmlNodePtr node0=node;
  for(; node0; node0=(node0 == node ? node0->children : node0->next))
  {
    if(xmlIsElement(node0) && xmlStrncasecmp(node0->name, "actionDescription", strlen(node0->name))==0)
    {
      char *name=xmlGetProp(node0,"name");
      char *pepInterface=xmlGetProp(node0,"pep");
      if(name==NULL) {log_warn("ActionDescription missing required field: NAME!"); continue;}

      actionDescription_ptr action_desc = actionDescriptionFind(pdp->actionDescStore, name, TRUE);
      if(action_desc==NULL) {log_warn("Error inserting actionDescription to store!"); continue;}
      log_debug("ActionDescription [%s] successfully added to store", name);

      // searching for parameterDescriptions
      xmlNodePtr node1=node0;
      for( ; node1; node1=(node1==node0 ? node1->children : node1->next))
      {
        if(xmlIsElement(node1) && !xmlStrncasecmp(node1->name, "parameterDescription", 20))
        {
          char *pname = xmlGetProp(node1, "name");
          paramDescription_ptr lactionParamDesc = actionDescFindParam(action_desc, pname, TRUE);
          if(lactionParamDesc!=NULL)
          {
            log_debug("ParameterDescription [%s] for Action [%s] successfully added", pname, name);
            xmlNodePtr node2=node1;
            for( ; node2; node2=(node2==node1 ? node2->children : node2->next))
            {
              if(xmlIsElement(node2) && !xmlStrncasecmp(node2->name, "value", 5))
              {
                char *pvalue=NULL;
                if(node2->children->type==XML_TEXT_NODE) pvalue=node2->children->content;
                if(pvalue!=NULL && addParamValue(action_desc, pname, pvalue)!=R_SUCCESS)
                  log_warn("Failure adding value to parameterDescription");
              }   //if(xmlIsElement(node2) && !xmlStrncasecmp(node2->name, "value", 5))
            }
          } //if(lactionParamDesc!=NULL)
          else {log_warn("Error adding parameter description to action description"); continue;}
          xmlFree(pname);
        }
      } // iterating over parameterDescriptions

      // Processing interface if present in actionDescription
      if(pepInterface!=NULL)
      {
        log_trace("Processing given PEP interface [%s] for action [%s]", pepInterface, name);
        unsigned int ret=pdpRegisterAction(name, pepInterface);
        log_debug("Action registration for action [%s] via PEP interface [%s] => [%u]", name, pepInterface, ret);
        xmlFree(pepInterface);
      }
      else log_trace("NO PEP interface specified in policy for action [%s]...", name);
      xmlFree(name);
    }
  }
  actionDescriptionStoreLog(pdp->actionDescStore);
  return R_SUCCESS;
}

unsigned int interfaceDescriptions_parseXML(xmlNodePtr node)
{
  checkNullInt(node, "Cannot parse NULL xml-node");
  xmlNodePtr node0=node;
  unsigned int ret=0;
  unsigned int retCode=R_SUCCESS;
  for(; node0; node0=(node0 == node ? node0->children : node0->next))
  {
    if(xmlIsElement(node0) &&
       (xmlStrncasecmp(node0->name, "pep", strlen(node0->name))==0 || xmlStrncasecmp(node0->name, "pxp", strlen(node0->name))==0 ))
    {
      unsigned int peppxp=-1;
      if(xmlStrncasecmp(node0->name, "pep", strlen(node0->name))==0) peppxp=0;
      else peppxp=1;

      char *name=xmlGetProp(node0, "name");
      char *type=xmlGetProp(node0, "type");
      if(name==NULL || type==NULL) {log_error("Error processing interface description; required field is missing"); retCode=R_ERROR; continue;}
      log_trace("Processing [%s] interface description [%s]", (peppxp==0?"PEP":"PXP"), name);

      pdpInterface_ptr linterface=NULL;
      if(strncasecmp(type, "native", 6)==0)
      { // native interface cannot be specified in policy; require function pointer of handling method
        linterface=pdpInterfaceNew(name, PDP_INTERFACE_NATIVE);
        if(peppxp==0) ret=pdpRegisterPEP(name, linterface);
        else ret=pdpRegisterPXP(name, linterface);
        if(ret==R_ERROR)
        {
          log_debug("Registration failed; deallocating created interface");
          pdpInterfaceFree(linterface);
        }
        else log_debug("Finished [%s] registration with native interface for PEP [%s] => [%u]", (peppxp==0?"PEP":"PXP"), name, ret);
      }
      else if(strncasecmp(type, "jni", 3)==0)
      { // read className, methodName and methodSignature for interface preparation
        linterface=pdpInterfaceNew(name, PDP_INTERFACE_JNI);

        char *className=NULL;
        char *methodName=NULL;
        char *methodSignature=NULL;
        xmlNodePtr cur=node0->children;
        while(cur!=NULL)
        {
          if(!xmlStrncasecmp(cur->name,"parameter",xmlStrlen(cur->name)))
          {
            char *pname=xmlGetProp(cur, "name");
            char *pvalue=xmlGetProp(cur, "value");
            if(pname==NULL || pvalue==NULL) {log_error("Error processing interface description; required field is missing"); retCode=R_ERROR; continue;}

            if(strncasecmp(pname, "className", 9)==0) className=strdup(pvalue);
            else if(strncasecmp(pname, "methodName", 10)==0) methodName=strdup(pvalue);
            else if(strncasecmp(pname, "methodSignature", 15)==0) methodSignature=strdup(pvalue);
            xmlFree(pname);
            xmlFree(pvalue);
          }
          cur=cur->next;
        }
        if(className==NULL || methodName==NULL || methodSignature==NULL)
        {
          log_error("Error processing interface description; required field (className, methodName, methodSignature) is missing");
          if(className!=NULL) free(className);
          if(methodName!=NULL) free(methodName);
          if(methodSignature!=NULL) free(methodSignature);

          xmlFree(name);
          xmlFree(type);
          retCode=R_ERROR;
          continue;
        }

        JNIEnv *lenv=NULL;
        extern JavaVM *curjvm;
        if(curjvm==NULL)
        {
          log_error("ERROR! JVM is not set! Instantiating new JVM currently not supported.");
          xmlFree(name);
          xmlFree(type);
          free(className);
          free(methodName);
          free(methodSignature);
          pdpInterfaceFree(linterface);

          retCode=R_ERROR;
          continue;
        }

        (*curjvm)->AttachCurrentThread(curjvm, (void **)&lenv, NULL);
        jclass lclsPEPPXP=(*lenv)->FindClass(lenv, className);
        if(lclsPEPPXP==NULL) {log_error("ERROR: Could not find required classes for PEP/PXP registering!"); retCode=R_ERROR; continue;}

        if(peppxp==0)
        {
          linterface->classPEP=(*lenv)->NewGlobalRef(lenv, lclsPEPPXP);
          if(linterface->classPEP==NULL) {log_error("ERROR: Could not find required classes for PEP/PXP registering!"); retCode=R_ERROR; continue;}

          linterface->getInstance=(*lenv)->GetStaticMethodID(lenv, linterface->classPEP, "getInstance", "()Lde/fraunhofer/iese/pef/pep/IPolicyEnforcementPoint;");
          if(linterface->getInstance==NULL) {log_error("Error resolving methodID for getInstance in PEP class"); return JNI_ERR;}

          jobject lpepSingletonInstance=(jobject)(*lenv)->CallStaticObjectMethod(lenv, linterface->classPEP, linterface->getInstance);
          if(lpepSingletonInstance==NULL) {log_error("Error retrieving PEP instance object"); return JNI_ERR;}

          linterface->instance=(*lenv)->NewGlobalRef(lenv, lpepSingletonInstance);
          if(linterface->instance==NULL) {log_error("ERROR: Could not create global reference for PEP object!"); return JNI_ERR;}
          (*lenv)->DeleteLocalRef(lenv, lpepSingletonInstance);

          linterface->pepSubscriptionMethod=(*lenv)->GetMethodID(lenv, linterface->classPEP, methodName, methodSignature);
          if(linterface->pepSubscriptionMethod==NULL) {log_error("Error resolving methodID for PEP/PXP requests"); retCode=R_ERROR; continue;}
        }
        else
        {
          linterface->classPXP=(*lenv)->NewGlobalRef(lenv, lclsPEPPXP);
          if(linterface->classPXP==NULL) {log_error("ERROR: Could not find required classes for PEP/PXP registering!"); retCode=R_ERROR; continue;}

          linterface->getInstance=(*lenv)->GetStaticMethodID(lenv, jniPIP.cls, "getInstance", "()Lde/fraunhofer/iese/pef/pxp/IPolicyExecutionPoint;");
          if(linterface->getInstance==NULL) {log_error("Error resolving methodID for getInstance in PXP class"); return JNI_ERR;}

          jobject lpxpSingletonInstance=(jobject)(*lenv)->CallStaticObjectMethod(lenv, linterface->classPXP, linterface->getInstance);
          if(lpxpSingletonInstance==NULL) {log_error("Error retrieving PEP instance object"); return JNI_ERR;}

          linterface->instance=(*lenv)->NewGlobalRef(lenv, lpxpSingletonInstance);
          if(linterface->instance==NULL) {log_error("ERROR: Could not create global reference for PXP object!"); return JNI_ERR;}
          (*lenv)->DeleteLocalRef(lenv, lpxpSingletonInstance);

          linterface->pxpExecutionMethod=(*lenv)->GetStaticMethodID(lenv, linterface->classPXP, methodName, methodSignature);
          if(linterface->pxpExecutionMethod==NULL) {log_error("Error resolving methodID for PEP/PXP requests"); retCode=R_ERROR; continue;}
        }
        if(lclsPEPPXP!=NULL) (*lenv)->DeleteLocalRef(lenv, lclsPEPPXP);

        if(peppxp==0) ret=pdpRegisterPEP(name, linterface);
        else ret=pdpRegisterPXP(name, linterface);
        log_info("%s JNI registration for className=[%s], methodName=[%s] with signature=[%s] => [%u]", (peppxp==0?"PEP":"PXP"), className, methodName, methodSignature, ret);

        free(className);
        free(methodName);
        free(methodSignature);
      }
      else if(strncasecmp(type, "socket", 6)==0)
      {
        log_trace("Processing socket interface specification");
        linterface=pdpInterfaceNew(name, PDP_INTERFACE_SOCKET);

        // fetching socket parameter
        char *type=NULL;
        char *host=NULL;
        char *port=NULL;
        xmlNodePtr cur=node0->children;
        while(cur!=NULL)
        {
          if(!xmlStrncasecmp(cur->name,"parameter",xmlStrlen(cur->name)))
          {
            char *pname=xmlGetProp(cur, "name");
            char *pvalue=xmlGetProp(cur, "value");
            if(pname==NULL || pvalue==NULL) {log_error("Error processing interface description; required field is missing"); retCode=R_ERROR; continue;}

            if(strncasecmp(pname, "type", 4)==0) type=strdup(pvalue);
            else if(strncasecmp(pname, "host", 4)==0) host=strdup(pvalue);
            else if(strncasecmp(pname, "port", 4)==0) port=strdup(pvalue);
            xmlFree(pname);
            xmlFree(pvalue);
          }
          cur=cur->next;
        }
        if(type==NULL || host==NULL || port==NULL || strncasecmp(type, "TCP", 3)!=0)
        {
          log_error("Error processing interface description; required field (type, host, port) is missing or invalid");
          if(type!=NULL) free(type);
          if(host!=NULL) free(host);
          if(port!=NULL) free(port);

          xmlFree(name);
          xmlFree(type);
          retCode=R_ERROR;
          continue;
        }
        long porti=strtol(port, NULL, 10);
        log_trace("Found TCP socket specification: host=[%s], port=[%d]", host, porti);
        linterface->socket=pefSocketClientNew(PEFSOCKET_TCP, host, porti);
        if(linterface->socket==NULL)
        {
          log_error("Error initializing socket for interface");
          retCode=R_ERROR;
          /// @todo deallocation in case of error
          continue;
        }
        log_debug("Interface socket initialized");
        char *testResponse=linterface->socket->clientSend(linterface->socket, "testnachricht");
        log_debug("testresponse=[%s]", testResponse);

        if(peppxp==0) ret=pdpRegisterPEP(name, linterface);
        else ret=pdpRegisterPXP(name, linterface);
        if(ret==R_ERROR)
        {
          log_debug("Registration failed; deallocating created interface");
          pdpInterfaceFree(linterface);
        }
        else log_debug("Finished [%s] registration with socket interface for PEP [%s] => [%u]", (peppxp==0?"PEP":"PXP"), name, ret);
      }
      else if(strncasecmp(type, "xmlrpc", 6)==0)
      {
        log_error("Specified interface type [%s] not yet implemented...", type);
        retCode=R_ERROR;
        continue;
      }

      xmlFree(name);
      xmlFree(type);
    }
  }
  return retCode;
}

unsigned int searchMechanismNodes(xmlNodePtr rootNode, const char *mechanismName, const char *ns, bool revoke)
{
  checkNullInt(rootNode, "Cannot parse NULL xml-node");
  xmlNodePtr node=rootNode->children;
  int ret=1;
  while(node!=NULL)
  {
    if(xmlIsElement(node))
    {
      if(!xmlStrcasecmp(node->name, "interfaceDescriptions"))
      {
        ret=interfaceDescriptions_parseXML(node);
        log_debug("Parsing interfaceDescriptions... [%s]",  returnStr[ret]);
      }
      if(!xmlStrcasecmp(node->name, "actionDescriptions"))
      {
        ret=actionDescriptions_parseXML(node);
        log_debug("Parsing actionDescriptions... [%s]",  returnStr[ret]);
      }
      else if(!xmlStrcasecmp(node->name, "contextDescriptions"))
      {
        ret=contextDescriptions_parseXML(node);
        log_debug("Parsing contextDescriptions... [%s]", returnStr[ret]);
      }
      else if(!xmlStrcmp(node->name,"detectiveMechanism") || !xmlStrcmp(node->name, "preventiveMechanism"))
      {
        if(!revoke)
        {
          if(mechanismName!=NULL)
          {
            xmlChar* curMechName=xmlGetProp(node, "name");
            if(curMechName!=NULL)
            {
              if(xmlStrcasecmp(mechanismName, curMechName)==0) ret=mechanismInstall(node, ns);
              else {node=node->next; xmlFree(curMechName); continue;}
              xmlFree(curMechName);
            }
          }
          else ret=mechanismInstall(node, ns);
        }
        else ret=pdpRevokeMechanism(xmlGetProp(node, "name"), ns);

        if(ret==R_ERROR)
        {
          log_error("Error processing policy request");
          return ret;
        }
      }
      else ret=searchMechanismNodes(node, mechanismName, ns, revoke);
    }
    node=node->next;
  }
  return ret;
}

unsigned int mechanismInstall(xmlNodePtr node, const char *ns)
{
  mechanism_ptr curMechanism=mechanismParseXML(node, ns);
  checkNullInt(curMechanism, "Error adding mechanism");

  pdp->mutex->lock(pdp->mutex, INFINITE);
  g_hash_table_insert(pdp->mechanismTable, curMechanism->mechFullname, curMechanism);
  pdp->mutex->unlock(pdp->mutex);
  log_info("Successfully added mechanism \"%s\" to hashtable",curMechanism->mechName);

  log_debug("Starting update thread for mechanisms=[%s] with usleep=[%llu]", curMechanism->mechName, curMechanism->timestepSize);
  curMechanism->updateThread=pefThreadNew(mechanismUpdateThread, curMechanism);
  curMechanism->updateThread->start(curMechanism->updateThread);
  checkNullInt(curMechanism->updateThread, "Error creating update thread for mechanism");

  return R_SUCCESS;
}

void conditionTriggerEvent(operator_ptr curop, event_ptr levent)
{
  log_debug("%s - checking condition eventMatch=[%s] for levent=[%s]", __func__, ((eventMatch_ptr)curop->operands)->matchAction->actionName,
                                                                       levent->actionDesc->actionName);
  if(eventMatches(curop->operands, levent)==TRUE)
    curop->state->value=TRUE;
  log_trace(" evaluated EVENT    node => %u", curop->state->value);
}

void mechanismTriggerEvent(mechanism_ptr mech, notifyResponse_ptr response)
{
  event_ptr levent=response->event;
  log_debug("%s - checking mech=[%s] for levent=[%s]", __func__, mech->mechName, levent->actionDesc->actionName);

  mech->mechMutex->lock(mech->mechMutex, INFINITE);
  if(eventMatches(mech->trigger, levent))
  {
    log_info("%s - Found matching mechanism -> mechanism_name=[%s]", __func__, mech->mechName);
    if(response->authorizationAction==NULL)
      response->authorizationAction=authorizationActionNew(strdup("response"), ACTION_ALLOW, 0, NULL);

    bool conditionValue=conditionUpdate(mech, mech->condition, levent);
    if(conditionValue)
    { // condition is satisfied; so update other mechanisms and return authorizationAction and executes
      if(notifyResponseProcessMechanism(response, mech)==R_ERROR) log_error("preparing response failed!");
    }
    else log_trace("condition NOT satisfied, doing nothing...");
  }
  mech->mechMutex->unlock(mech->mechMutex);
}

// PDP response methods
// standard-response=ACTION_ALLOW w/ delay=0us
/// @todo prepare static standard response!
notifyResponse_ptr notifyResponseNew(event_ptr levent)
{
  notifyResponse_ptr nResponse=memAlloc(sizeof(notifyResponse_t));
  checkNullPtr(nResponse, "Could not allocate memory for notification response");
  nResponse->event=levent;
  nResponse->authorizationAction=NULL;
  return nResponse;
}

void notifyResponseLog(notifyResponse_ptr response)
{
  if(response==NULL) {log_warn("Cannot log NULL notifyResponse!"); return;}
  log_debug("notifyEvent response: ");
  log_debug("   response: [%s]",  response->authorizationAction->response ? "ALLOW" : "INHIBIT");
  log_debug("   delay:    [%llu]",response->authorizationAction->delay);
  log_debug("   modifiers:[%u]",  response->authorizationAction->cntParamsModify);
}

unsigned int notifyResponseAddModifier(mechanism_ptr curMech, authorizationAction_ptr curMechAction, const char *name,
                                       const char *value, const char *isXPath)
{
  checkNullInt(curMech,       "%s - curMech is NULL",__func__);
  checkNullInt(curMechAction, "%s - curMechAction is NULL",__func__);
  checkNullInt(name,          "%s - name is NULL",__func__);
  checkNullInt(value,         "%s - value is NULL",__func__);

  // reference to actions of trigger!
  curMechAction->paramsModify=realloc(curMechAction->paramsModify,(curMechAction->cntParamsModify+1)*sizeof(parameterInstance_ptr));
  checkNullInt(curMechAction->paramsModify,"Could not allocate memory for new parameter");

  actionDescription_ptr laction = curMech->trigger->matchAction;
  paramDescription_ptr lparam = actionDescFindParam(laction, name, FALSE);
  if(lparam == NULL)
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

  log_debug("Successfully added parameter description \"%s\" with value \"%s\" to response-modifiers",name,value);
  curMechAction->cntParamsModify++;
  return R_SUCCESS;
}

unsigned int notifyResponseProcessMechanism(notifyResponse_ptr response, mechanism_ptr mech)
{
  if(response==NULL || mech==NULL) {log_error("[%s] - ERROR: Null parameter", __func__); return R_ERROR;}

  log_debug("Adding mechanism=[%s] to response", mech->mechName);

  // Start with mechanism's starting authorizationAction
  authorizationAction_ptr curAuthorization=mech->authorizationActions;

  // Trying to execute specified actions of new mechanism
  if(response->authorizationAction->response==ACTION_ALLOW)
  { // only ALLOW-Type can specify executeActions in authorizationAction
    // (inhibit can specify them out of authAction node, without difference)
    // if response already inhibit event, these actions should not be executed if they would result in allowing the event,
    // so check existing response first for ACTION_ALLOW!
    do
    {
      log_debug("Processing mechanism's authorizationAction [%s]", curAuthorization->name);
      unsigned int executeReturns=0;
      if(curAuthorization->cntExecuteActions>0)
      {
        log_trace("Executing specified executeActions (%d)...", curAuthorization->cntExecuteActions);
        unsigned int a;
        for(a=0; a<curAuthorization->cntExecuteActions; a++)
        {
          executeAction_ptr curExecAction=curAuthorization->executeActions[a];
          log_trace("Preparing execution");
          unsigned int b;
          for(b=0; b<curExecAction->cntParams; b++)
          {
            if(curExecAction->params[b]->type==PARAM_INSTANCE_XPATH)
            {
              log_trace("Evaluating given xpath expression for parameter instance...");
              if(response->event->xmlDoc==NULL) eventCreateXMLdoc(response->event);
              /// @todo deallocation of temporary result of xpathExec
              log_trace("Processing xpath=[%s] against evnt:", curExecAction->params[b]->evalValue);
              dumpXMLdoc(response->event->xmlDoc);
              char *retVal=xpathExec(response->event->xmlDoc, curExecAction->params[b]->evalValue);
              if(retVal!=NULL) curExecAction->params[b]->value=retVal;
            }
          }

          log_trace("Executing [%s](%d) interfaceMethod=[%p]", curExecAction->actionDesc->actionName,
                                                           curExecAction->cntParams,
                                                           curExecAction->actionDesc->pxpInterface->pxpExecute);

          executeReturns=curExecAction->actionDesc->pxpInterface->pxpExecute(curExecAction->actionDesc->pxpInterface,
              curExecAction->actionDesc->actionName, curExecAction->cntParams, curExecAction->params);

          log_trace("Executing [%s] returned [%s]", curExecAction->actionDesc->actionName, returnStr[executeReturns]);
          if(executeReturns==R_ERROR) break;
        }

        if(executeReturns==R_SUCCESS)
        {
          log_debug("All specified execution actions were executed successfully!");
          break;
        }
        else
        {
          log_debug("Error executing specified execution actions!!");
          log_debug("Trying to process fallback authorization action [%p]", curAuthorization->fallback);
          // fallback should never be NULL
          if(curAuthorization->fallback==NULL)
            {log_fatal("Specified fallback in authorizationAction [%s] is NULL", curAuthorization->name); return R_ERROR;}
          curAuthorization=curAuthorization->fallback;
        }
      }
      else
      {
        log_trace("Nothing to execute for this authorizationAction");
        break;
      }
    }
    while(1);

  }

  if(response->authorizationAction->response!=ACTION_INHIBIT)
  {
    if(curAuthorization->response==ACTION_INHIBIT)
    {
      log_trace("Updating ACTION_ALLOW to ACTION_INHIBIT");
      response->authorizationAction->response=ACTION_INHIBIT;
    }
    else
    { // action was allowed and should still be allowed; so update delay and modifiers...
      // set delay to max(response->delay, new_mech->delay)
      if(curAuthorization->delay > response->authorizationAction->delay)
      {
        log_trace("Previous delay=[%llu] updated to delay=[%llu]", response->authorizationAction->delay, curAuthorization->delay);
        response->authorizationAction->delay=curAuthorization->delay;
      }

      // update modifiers
      // don't know how to handle conflicts... (modification of same parameter from different mechanisms)
      /// @todo conflict handling for modifiers in notifyEvent response: currently modifiers are simply appended!
      if(curAuthorization->cntParamsModify>0)
      {
        unsigned int i;
        for(i=0;i<curAuthorization->cntParamsModify; i++)
        {
          int ret=notifyResponseAddModifier(mech, response->authorizationAction, curAuthorization->paramsModify[i]->paramDesc->name,
              curAuthorization->paramsModify[i]->value, boolStr[curAuthorization->paramsModify[i]->isXPath]);
          if(ret==R_ERROR) log_error("Error appending modifier for response!");
        }
      }
    }
  }
  else log_trace("Response is already INHIBIT");

  return R_SUCCESS;
}

char *notifyResponseSerialize(notifyResponse_ptr response)
{
  xmlDocPtr xmlDoc = NULL;
  xmlNodePtr responseNode = NULL, authorizationNode = NULL, authorizationResponseNode = NULL, actionNode = NULL, paramNode = NULL;
  xmlDoc = xmlNewDoc("1.0");
  responseNode = xmlNewNode(NULL, "notifyEventResponse");
  xmlDocSetRootElement(xmlDoc, responseNode);

  if(response!=NULL)
  {
    if(response->authorizationAction!=NULL)
    {
      authorizationNode=xmlNewChild(responseNode, NULL, "authorizationAction", NULL);
      authorizationResponseNode=xmlNewChild(authorizationNode, NULL, (response->authorizationAction->response==0 ? "inhibit" : "allow"), NULL);
      if(response->authorizationAction->delay!=0)
      {
        char *amountStr = memAlloc(20 * sizeof(char));
        snprintf(amountStr, 20, "%llu", response->authorizationAction->delay);
        xmlNodePtr delayNode = xmlNewChild(authorizationResponseNode, NULL, "delay", NULL);
        xmlNewProp(delayNode, "amount", amountStr);
        xmlNewProp(delayNode, "unit", "MICROSECONDS");
      }

      if(response->authorizationAction->cntParamsModify>0)
      {
        xmlNodePtr modifyNode = NULL;
        modifyNode=xmlNewChild(authorizationResponseNode, NULL, "modify", NULL);
        unsigned int b;
        for(b=0; b<response->authorizationAction->cntParamsModify; b++)
        {
          paramNode=xmlNewChild(modifyNode, NULL, "parameter", NULL);
          xmlNewProp(paramNode, "name", response->authorizationAction->paramsModify[b]->paramDesc->name);
          xmlNewProp(paramNode, "value", response->authorizationAction->paramsModify[b]->value);
          xmlNewProp(paramNode, "isXPath", (response->authorizationAction->paramsModify[b]->isXPath ? "true" : "false"));
        }
      }
    }
  }

  xmlBufferPtr retbuffer = dumpXMLdoc(xmlDoc);
  // caller should take care of freeing memory for the returning string!
  char *retVal=memAlloc( (strlen(retbuffer->content)+1) * sizeof(unsigned char));
  memcpy(retVal, retbuffer->content, strlen(retbuffer->content)+1);

  // free temporary variables
  xmlFreeDoc(xmlDoc);
  xmlBufferFree(retbuffer);
  return retVal;
}

unsigned int notifyResponseFree(notifyResponse_ptr response)
{
  if(eventFree(response->event)==R_ERROR) {log_warn("Error in freeing temporary event memory"); return R_ERROR;}

  // free mechanism_actions in response; created during preparation
  if(authorizationActionFree(response->authorizationAction)==R_ERROR)
    {log_warn("Error in freeing temporary authorization-action for response"); return R_ERROR;}

  // the remaining variables are only references -> keeping
  free(response);
  return R_SUCCESS;
}

















