/**
 * @file  mechanism.c
 * @brief Implementation of mechanisms, i.e. the main part of an OSL policy
 *
 * @see mechanism.h, mechanism_s
 *
 * @author cornelius moucha
 **/

#include "mechanism.h"
#include "log_mechanism_pef.h"

extern pdp_ptr pdp;

/******************************************************************************
 *  Mechanism implementation
******************************************************************************/
mechanism_ptr mechanismNew(const char *name, const char *ns)
{
  checkNullPtr(name, "Error: NULL parameter for [%s]", __func__);
  checkNullPtr(ns,   "Error: NULL parameter for [%s]", __func__);

  mechanism_ptr nmech=memAlloc(sizeof(mechanism_t));
  checkNullPtr(nmech, "Could not allocate memory for mechanism");

  nmech->mechName=memCalloc(strlen(name)+1, sizeof(char));
  checkNull(nmech->mechName, NULL, {free(nmech);}, "Could not allocate memory for mechanism name");
  strncpy(nmech->mechName, name, strlen(name)+1);

  const char *lns=(ns!=NULL ? ns : noNamespace);
  nmech->ns=memCalloc(strlen(lns)+1, sizeof(char));
  checkNull(nmech->ns, NULL, {free(nmech);}, "Could not allocate memory for namespace");
  strncpy(nmech->ns, lns, strlen(lns)+1);

  nmech->startTime=timestampNew();
  nmech->timestepSize=0;
  nmech->lastUpdate=timestampNew();
  nmech->timestep=0;
  nmech->trigger=NULL;
  nmech->condition=NULL;
  nmech->authorizationActions=NULL;
  nmech->cntExecuteActions=0;
  nmech->executeActions=NULL;
  nmech->mechMutex=pefMutexNew(nmech->mechName);
  checkNullPtr(nmech->mechMutex, "Error initializing mechanism mutex!");
  log_trace("Successfully allocated mechanism \"%s\" in namespace \"%s\" (@ %llu)",nmech->mechName,nmech->ns, nmech->startTime->usec);
  return nmech;
}

/**
 * @brief mechanism free
 * Deallocates the memory of a mechanism
 *
 **/
unsigned int mechanismFree(mechanism_ptr mech)
{
  checkNullInt(mech, "Error: NULL parameter for [%s]", __func__);
  mech->mechMutex->lock(mech->mechMutex, INFINITE);
  log_debug("Stopping mechanism update thread");

  int ret=0;
  ret=pefThreadKill(mech->updateThread);
  if(ret!=0) log_warn("Cancelation error");
  ret=pefThreadFree(mech->updateThread);
  if(ret!=R_SUCCESS) log_warn("Error deallocating mechanism update thread.");

  if(mech->trigger!=NULL)
  {
    if(eventMatchFree(mech->trigger, mech)==R_ERROR) return R_ERROR;
    else log_debug("mechanism trigger successfully freed");
  }

  if(conditionFree(mech->condition, mech)==R_ERROR) return R_ERROR;
  else log_debug("mechanism condition successfully freed");
  log_trace("freeing mechanism_actions (authorization_actions)");
  if(authorizationActionFree(mech->authorizationActions)) return R_ERROR;

  log_trace("freeing executeActions (%d)", mech->cntExecuteActions);
  int i;
  for(i=0; i<mech->cntExecuteActions; i++)
    if(executeActionFree(mech->executeActions[i])==R_ERROR) return R_ERROR;
  log_trace("Finished freeing executeActions");
  if(mech->executeActions!=NULL) free(mech->executeActions);

  if(mech->mechFullname!=NULL) memFree(mech->mechFullname);
  memFree(mech->mechName);
  if(mech->ns!=NULL) memFree(mech->ns);
  log_trace("Finished freeing mechName and namespace");

  timestampFree(mech->startTime);
  timestampFree(mech->lastUpdate);
  log_trace("Finished freeing mechanism timesteps");

  mech->mechMutex->unlock(mech->mechMutex);
  pefMutexFree(mech->mechMutex);
  ret=0;

  free(mech);
  return R_SUCCESS;
}

uint64_t xmlParseTimestepSize(xmlNodePtr rootNode)
{
  if(rootNode==NULL) return 0;
  char *amountStr=xmlGetProp(rootNode,"amount"); // required to do separately to avoid memory leak due to libxml2...
  uint64_t interval=atoll(amountStr);
  xmlFree(amountStr);
  char *unit=xmlGetProp(rootNode, "unit");
  if(unit) interval*=getTimeUnitMultiplier(unit);
  log_trace("%s - timestepSize: %llu us",__func__, interval);
  xmlFree(unit);
  return interval;
}

/// @todo improvement: statical set e.g. sizeof(uint64_t) or often used sizes in base.h
///       (or look if compiler statically sets these values in asm)
unsigned long mechanismMemSize(mechanism_ptr mech)
{
  checkNullInt(mech, "Error: NULL parameter for [%s]", __func__);
  unsigned long size=sizeof(mechanism_ptr);
  size+=(strlen(mech->mechName)+1)*sizeof(char);
  size+=4*sizeof(uint64_t);   // usec_start, usec_last, usec_timestep, timestep
  //if(mech->trigger!=NULL)   size+=event_getMemSize(mech->trigger);
  if(mech->condition!=NULL) size+=conditionMemSize(mech->condition);
  if(mech->authorizationActions!=NULL)   size+=authorizationActionMemSize(mech->authorizationActions);
  return size;
}

void printMechNames(mechanism_ptr mech, void* a)
{
  if(mech==NULL) {log_error("Error: NULL parameter for [%s]", __func__); return;}
  log_debug("curmechPtr=[%p]",mech);
  log_debug("curmechName=[%s]",mech->mechName);
}

unsigned int mechanismAddExecuteAction(mechanism_ptr curMech, xmlNodePtr node, const char *name)
{ // node = XMLnode action -> increase list of executeAction_ptr's and parse parameters in action
  checkNullInt(curMech, "%s - curMech is NULL",__func__);
  checkNullInt(node,    "Error: NULL parameter for [%s]", __func__);
  checkNullInt(name,    "Error: NULL parameter for [%s]", __func__);

  curMech->executeActions=realloc(curMech->executeActions,(curMech->cntExecuteActions+1)*sizeof(executeAction_ptr));
  checkNullInt(curMech->executeActions, "Could not allocate memory for new action");

  executeAction_ptr nAction = executeActionNew(name);
  xmlNodePtr cur=node->children;
  while(cur!=NULL)
  {
    if(xmlIsElement(cur) && !xmlStrncasecmp(cur->name, "parameter",9))
    {
      char *lname=xmlGetProp(cur,"name");
      char *lvalue=xmlGetProp(cur,"value");
      //char *lisXPath=xmlGetProp(cur,"isXPath");
      char *ltype=xmlGetProp(cur, "type");
      executeActionAddParamInstance(nAction, lname, lvalue, ltype);
      xmlFree(lname);
      xmlFree(lvalue);
      xmlFree(ltype);
    }
    cur=cur->next;
  }

  curMech->executeActions[curMech->cntExecuteActions]=nAction;
  log_trace("Successfully added executeAction \"%s\" mechanism", name);
  curMech->cntExecuteActions++;
  return R_SUCCESS;
}

mechanism_ptr mechanismParseXML(xmlNodePtr rootNode, const char *ns)
{
  checkNullPtr(rootNode, "Error: NULL parameter for [%s]", __func__);
  checkNullPtr(ns, "Error: NULL parameter for [%s]", __func__);

  log_trace("Mechanism Type: %s", rootNode->name);
  char *mechanismName=xmlGetProp(rootNode, "name");
  checkNullPtr(mechanismName,"Empty name for mechanism not allowed! Need a name (ID) for inserting into hash table.");

  // checking for existing mechanism; required to do it here as further processing might involve subscriptions etc.
  unsigned int mfnLength=strlen(mechanismName)+strlen(ns)+2;
  char *mechanismFullName=(char*)memCalloc(mfnLength, sizeof(char));
  snprintf(mechanismFullName, mfnLength, "%s#%s", ns, mechanismName);
  // ensure terminating null-terminator
  mechanismFullName[mfnLength-1]='\0';
  log_trace("MechanismFullName=[%s]", mechanismFullName);

  if(g_hash_table_lookup(pdp->mechanismTable, mechanismFullName)!=NULL)
  {
    log_error("Mechanism [%s] already installed!", mechanismFullName);
    free(mechanismName);
    free(mechanismFullName);
    return NULL;
  }

  mechanism_ptr nmech=mechanismNew(mechanismName, ns);
  free(mechanismName);
  nmech->mechFullname=mechanismFullName;
  nmech->timestepSize=xmlParseTimestepSize(xmlFindNode(rootNode, "timestep"));

  xmlNodePtr tmpNode=xmlFindNode(rootNode, "trigger");
  if(tmpNode!=NULL)
  {
    nmech->trigger=eventMatchParseXML(pdp->actionDescStore,tmpNode);

    // subscribe to PEP for trigger event:
    #if PDP_PEPSUBSCRIPTION == 1
      unsigned int ret=nmech->trigger->matchAction->pepInterface->pepSubscribe(nmech->trigger->matchAction->pepInterface,
                                                                               nmech->trigger->matchAction->actionName, 0);
      log_debug("PEP subscription for event name=[%s]=>[%d]", nmech->trigger->matchAction->actionName, ret);
      #if ABORT_SUBSCRIPTION_ERROR == 1
        if(ret==R_ERROR)
        { /// @todo clean after error
          log_error("PEP subscription for trigger event failed => aborting...");
          return NULL;
        }
      #endif
    #endif

    // add mechanism to action entry action_description store (for later lookup for triggered mechanisms)
    nmech->trigger->matchAction->mechanisms=g_slist_append(nmech->trigger->matchAction->mechanisms, nmech);
  }
  else
  {
    if(!xmlStrncasecmp(rootNode->name,"preventiveMechanism",19))
    {
      log_error("NULL as trigger in preventiveMechanism NOT allowed!");
      /// @todo cleanup allocated stuff
      return NULL;
    }
    else nmech->trigger=NULL;
  }

  nmech->condition=conditionParseXML(xmlFindNode(rootNode, "condition"), nmech);

  // Parse authorizationActions
  nmech->authorizationActions=authorizationActionParseXML(nmech, rootNode);
  if(!xmlStrncasecmp(rootNode->name,"preventiveMechanism",19) && nmech->authorizationActions==NULL)
  {
    log_error("Error processing preventiveMechanism; require authorizationAction");
    return NULL;
  }

  executeActionParseXML(nmech,rootNode);
  log_trace("Successfully parsed mechanism %s (size: %ld bytes)", nmech->mechName, mechanismMemSize(nmech));
  mechanismLog(nmech);
  return nmech;
}

void mechanismLog(mechanism_ptr mech)
{
  if(mech==NULL) {log_error("Cannot log NULL-mechanism!"); return;}
  log_trace("Mechanism: ");
  log_trace("         name: %s",   mech->mechName);
  log_trace(" timestepSize: %llu", mech->timestepSize);
  log_trace("        start: %llu", mech->startTime->usec);
  log_trace("  last update: %llu", mech->lastUpdate->usec);
  log_trace(" cur timestep: %llu", mech->timestep);
}

xmlDocPtr mechanismSerializeToXML(const mechanism_ptr mech)
{
  checkNullPtr(mech, "Error: NULL parameter for [%s]", __func__);

  xmlDocPtr xmlDoc = NULL;
  xmlNodePtr responseNode = NULL, authorizationNode = NULL, authorizationResponseNode = NULL, actionNode = NULL, paramNode = NULL;
  xmlDoc = xmlNewDoc("1.0");
  responseNode = xmlNewNode(NULL, "notifyEventResponse");
  xmlDocSetRootElement(xmlDoc, responseNode);

  if(mech->authorizationActions!=NULL)
  {
    authorizationNode=xmlNewChild(responseNode, NULL, "authorizationAction", NULL);
    authorizationResponseNode=xmlNewChild(authorizationNode, NULL, (mech->authorizationActions->response==0 ? "inhibit" : "allow"), NULL);
    if(mech->authorizationActions->delay!=0)
    {
      char *amountStr = memAlloc(20 * sizeof(char));
      snprintf(amountStr, 20, "%llu", mech->authorizationActions->delay);
      xmlNodePtr delayNode = xmlNewChild(authorizationResponseNode, NULL, "delay", NULL);
      xmlNewProp(delayNode, "amount", amountStr);
      xmlNewProp(delayNode, "unit", "MICROSECONDS");
      free(amountStr);
    }
    if(mech->authorizationActions->cntParamsModify>0)
    {
      xmlNodePtr modifyNode = NULL;
      modifyNode=xmlNewChild(authorizationResponseNode, NULL, "modify", NULL);
      unsigned int b;
      for(b=0; b<mech->authorizationActions->cntParamsModify; b++)
      {
        paramNode=xmlNewChild(modifyNode, NULL, "parameter", NULL);
        xmlNewProp(paramNode, "name", mech->authorizationActions->paramsModify[b]->paramDesc->name);
        xmlNewProp(paramNode, "value", mech->authorizationActions->paramsModify[b]->value);
        xmlNewProp(paramNode, "isXPath", (mech->authorizationActions->paramsModify[b]->isXPath ? "true" : "false"));
      }
    }
  }

  unsigned int a;
  for(a=0; a<mech->cntExecuteActions; a++)
  {
    actionNode=xmlNewChild(responseNode, NULL, "action", NULL);
    xmlNewProp(actionNode, "name", mech->executeActions[a]->actionDesc->actionName);
    unsigned int b;
    for(b=0; b<mech->executeActions[a]->cntParams; b++)
    {
      paramNode=xmlNewChild(actionNode, NULL, "parameter", NULL);
      xmlNewProp(paramNode, "name", mech->executeActions[a]->params[b]->paramDesc->name);
      xmlNewProp(paramNode, "value", mech->executeActions[a]->params[b]->value);
      xmlNewProp(paramNode, "type", (mech->executeActions[a]->params[b]->type == PARAM_INSTANCE_XPATH? "xpath" : "string"));
    }
  }
  return xmlDoc;
}

// note: in mingw 'printf' seems to have a bug with printing %llu;
// result is something like 3210975795 instead of 1337262753421875
// usage in snprintf and printing buffer results in valid output!
// printf seems to have serious problems in mingw!
//printf("A Timestep difference        %lld => (%d)\n", usec_diff, (usec_diff<0));
// results in A Timestep difference        -1705017079 => (-2)
// but -2 is no comparison result!!
/// @todo use separate method for NULL-event updating!
bool mechanismUpdate(const char *mechName, mechanism_ptr mech, event_ptr event)
{
  //log_warn("[%u]-----------------------------------------------------------------------------------------------------------", updateCount++);
  uint64_t usecElapsedLastUpdate;
  int64_t  usecDifference;
  bool     IS_NULL_EVENT=(event==NULL);
  uint64_t now = usecNow();
  usecElapsedLastUpdate = now - mech->lastUpdate->usec;
  usecDifference = usecElapsedLastUpdate - mech->timestepSize;

  if(IS_NULL_EVENT)
  {
    usecDifference = usecElapsedLastUpdate - mech->timestepSize;
    if(usecDifference<((int64_t)0))
    { // Aborting update because the time step has not passed yet
      //log_trace("%s - [%s] Timestep remaining %lld -> timestep has not yet passed", __func__, mechName, usecDifference);
      //log_trace("##############################################################################################################");
      return FALSE;
    }
    log_debug("////////////////////////////////////////////////////////////////////////////////////////////////////////////");
    log_debug("%s - [%s] Null-Event updating %llu. timestep at interval of %llu us", __func__, mechName, mech->timestep, mech->timestepSize);

    //log_trace("  Start time                 %llu", mech->startTime->usec);
    //log_trace("  Last update time           %llu", mech->lastUpdate->usec);

    // Correct time substracting possible delay in the execution because the difference between the timestep and the last time
    // the mechanism was updated will not be exactly the timestep
    mech->lastUpdate->usec = now - usecDifference;
    //log_trace("  Corrected last update time %llu", mech->lastUpdate->usec);
    //log_trace("  Current time               %llu", now);
    //log_trace("  Elapsed since last update  %llu", usecElapsedLastUpdate);
    //log_trace("  Timestep difference        %llu", usecDifference);

    if(usecDifference > mech->timestepSize)
    {
      log_warn("%s - [%s] Timestep difference is larger than mechanism's timestep size => we missed to evaluate one timestep!!", __func__, mechName);
      log_warn("--------------------------------------------------------------------------------------------------------------");
    }
  }

  // check condition formula
  bool conditionValue=conditionUpdate(mech, mech->condition, event);
  log_info("%s - [%s] event=[%u] timestep=[%llu] value=[%s]", __func__, mechName, !IS_NULL_EVENT, mech->timestep, boolStr[conditionValue]);

  if(IS_NULL_EVENT)
  { // Reset event nodes for EALL, EFST, XPATH
    /// @todo use table or similar for improving performance of reseting condition nodes to state value FALSE
    unsigned int a;
    for(a=0; a<mech->condition->cntNodes; a++)
    {
      if(mech->condition->nodes[a]->type==EVENTMATCH || mech->condition->nodes[a]->type==XPATH)
        mech->condition->nodes[a]->state->value=FALSE;
    }
    mech->timestep++;
  }

  log_trace("%s - [%s] -----------------------------------------------------------", __func__, mechName);
  return conditionValue;
}


#ifndef _WIN32
  void mechanismUpdaterExitHandler(int sig)
  {log_debug("exiting mechanism update thread..."); pthread_exit(0);}
#else
  void mechanismUpdaterExitHandler(int sig) {}
#endif

/// @todo locking mutex after sleep might result in race condition with locking mutex in mechanism_free
///       mechanism_free unlocks before destroying => locking here and continue with update -> segfault! => should be tested with sleeps
LPTHREAD_START_ROUTINE mechanismUpdateThread(void* mech)
{ // Generate periodically a NULL event to progress timestep
  mechanism_ptr curMech=(mechanism_ptr)mech;
  if(curMech==NULL) {log_error("Error: NULL parameter in [%s]", __func__); return NULL;}
  curMech->updateThread->prepareThreadKill(&mechanismUpdaterExitHandler);

  uint64_t sleepValue=div(curMech->timestepSize, 10).quot;
  log_info("Started mechanism update thread usleep=%llu us", sleepValue);
  sleepValue=curMech->updateThread->adjustSleepValue(curMech->updateThread, sleepValue);

  unsigned int executeReturns=0;
  while(1)
  {
    curMech->mechMutex->lock(curMech->mechMutex, INFINITE);
    bool mechValue=mechanismUpdate(curMech->mechName, curMech, NULL);
    if(mechValue==TRUE && curMech->cntExecuteActions>0)
    {
      log_trace("Executing specified executeActions (%d)...", curMech->cntExecuteActions);
      unsigned int a;
      for(a=0; a<curMech->cntExecuteActions; a++)
      {
        executeAction_ptr curExecAction=curMech->executeActions[a];
        log_trace("Executing [%s] interfaceMethod=[%p]", curExecAction->actionDesc->actionName,
                                                         curExecAction->actionDesc->pxpInterface->pxpExecute);
        executeReturns=curExecAction->actionDesc->pxpInterface->pxpExecute(curExecAction->actionDesc->pxpInterface,
            curExecAction->actionDesc->actionName, curExecAction->cntParams, curExecAction->params);

        log_trace("Executing [%s] returned [%s]", curExecAction->actionDesc->actionName, returnStr[executeReturns]);
      }
    }
    curMech->mechMutex->unlock(curMech->mechMutex);

    //#ifdef PTHREAD_CANCEL
    //  pthread_testcancel();
    //#endif

    /// @todo the sleep should be optimized considering the average time it takes to update the monitors
    ///       and the time step specified. This is now manually set to 10% of the mechanism's timestepSize
    curMech->updateThread->sleep(sleepValue);
  }
  //#ifdef PTHREAD_CANCEL
  //  pthread_cleanup_pop(0);
  //#endif
  return THREADRETURN;
}



