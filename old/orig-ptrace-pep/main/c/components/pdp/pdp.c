/**
 * @file  pdp.c
 * @brief Implementation of the PDP
 *
 * Main file of PDP implementation
 * @see pdp.h
 *
 * @author cornelius moucha
**/

#include "pdp.h"
#include "log_pdp_pef.h"

pdp_ptr     pdp;

/**
 * Constructor for the PDP shared library\n
 * Invoked when the library is linked against the application at runtime\n
 * Prepares the mutex for the PDP and sets the function pointers for the PDP exported methods
**/
__attribute__((constructor)) static void pdpConst(void)
{
  printf("pdpConst processing...\n");
  pdp=memAlloc(sizeof(pdp_t));
  logInit();
  printf("Logger initialization finished...\n");
  pdp->mutex=pefMutexNew("pdpMutex");

  // only pdpStart method is reachable after construction
  pdp->start=pdpStart;
  // set other methods to stubs
  pdp->stop=stubStop;
  pdp->deployMechanism=stubDeployMechanism;
  pdp->deployPolicy=stubDeployPolicy;
  pdp->deployPolicyString=stubDeployPolicy;
  pdp->listDeployedMechanisms=stubListDeployedMechanisms;
  pdp->notifyEvent=stubNotifyEvent;
  pdp->notifyEventXML=stubNotifyEventXML;
  pdp->revokeMechanism=stubRevokeMechanism;
  pdp->revokePolicy=stubRevokePolicy;
  pdp->mechanismTable=NULL;
  pdp->actionDescStore=NULL;
  pdp->contextTable=NULL;
  pdp->registeredPEPs=NULL;
  pdp->registeredPXPs=NULL;

  xmlInitParser();
  printf("pdpConstructor successfully finished\n");
  fflush(stdout);
}

/**
 * Destructor for the PDP shared library\n
 * Invoked when the library is detached from the application at runtime\n
 * Deallocates the PDP mutex and final clean up of allocated memory, e.g. from libxml2.
**/
__attribute__((destructor)) static void pdpDest(void)
{
  printf("Trying to deallocate pdpMutex\n");
  fflush(stdout);

  pefMutexFree(pdp->mutex);
  free(pdp);

  logDestroy();

  // free persistent structures from libxml2
  printf("Cleaning up libxml2 related memory\n");
  xmlCleanupParser();
  printf("pdpDestructor finished\n");
}

bool pdpStart()
{
  pdp->mechanismTable=g_hash_table_new_full(g_str_hash, g_str_equal, NULL, (GDestroyNotify)mechanismFree);
  checkNullInt(pdp->mechanismTable, "Unable to create PDP hash table");

  pdp->contextTable=g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)free, (GDestroyNotify)free);
  checkNullInt(pdp->contextTable, "Unable to create hash table for contextDescriptions");

  pdp->registeredPEPs=g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)free, (GDestroyNotify)pdpInterfaceFree);
  checkNullInt(pdp->registeredPEPs, "Unable to create hash table for registered PEPs");

  pdp->registeredPXPs=g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)free, (GDestroyNotify)pdpInterfaceFree);
  checkNullInt(pdp->registeredPXPs, "Unable to create hash table for registered PXPs");

  pdp->actionDescStore=actionDescriptionStoreNew();
  checkNullInt(pdp->actionDescStore, "Unable to create PDP action description store");

  if(pdpInterfacesInit()==R_ERROR) {log_error("Could not initialize PDP interfaces."); return R_ERROR;}
  log_debug("PDP interfaces initialized.");

  pdp->start=pdpStart;
  pdp->stop=pdpStop;
  pdp->deployMechanism=pdpDeployMechanism;
  pdp->deployPolicy=pdpDeployPolicy;
  pdp->deployPolicyString=pdpDeployPolicyString;
  pdp->listDeployedMechanisms=pdpListDeployedMechanisms;
  pdp->notifyEvent=pdpNotifyEvent;
  pdp->notifyEventXML=pdpNotifyEventXML;
  pdp->revokeMechanism=pdpRevokeMechanism;
  pdp->revokePolicy=pdpRevokePolicy;

  log_info("PDP initialized");
  return R_SUCCESS;
}

bool pdpStop()
{
  log_info("Stopping PDP...");
  pdp->mutex->lock(pdp->mutex, INFINITE);

  // Resetting pdp exported methods to stubs except of pdpStart, which should be still reachable
  pdp->start=pdpStart;
  pdp->stop=stubStop;
  pdp->deployMechanism=stubDeployMechanism;
  pdp->deployPolicy=stubDeployPolicy;
  pdp->deployPolicyString=stubDeployPolicy;
  pdp->listDeployedMechanisms=stubListDeployedMechanisms;
  pdp->notifyEvent=stubNotifyEvent;
  pdp->notifyEventXML=stubNotifyEventXML;
  pdp->revokeMechanism=stubRevokeMechanism;
  pdp->revokePolicy=stubRevokePolicy;

  // freeing stored mechanisms...
  log_debug("Revoking stored mechanisms...");
  g_hash_table_destroy(pdp->mechanismTable);

  // freeing action description store
  if(actionDescriptionStoreFree(pdp->actionDescStore)==R_ERROR) log_error("Error freeing action description store...");
  g_hash_table_destroy(pdp->contextTable);

  log_debug("Deallocating hashtable for registered PEPs");
  g_hash_table_destroy(pdp->registeredPEPs);
  log_debug("Deallocating hashtable for registered PXPs");
  g_hash_table_destroy(pdp->registeredPXPs);

  pdpInterfacesStop();

  pdp->mutex->unlock(pdp->mutex);
  log_info("PDP stopped...");
  return R_SUCCESS;
}

// PDP notification event with string
/// @todo cleanup
unsigned int eventrecvcnt=0;
char *pdpNotifyEventXML(const char *eventDoc)
{
  if(eventDoc==NULL) {log_error("Error: Received a NULL event for notification?!"); return NULL;}
  log_trace("receiving event (%d): [%s]", eventrecvcnt, eventDoc);
  // write event to file...
  /*
  char *filename=malloc(50*sizeof(char));
  snprintf(filename, 50, "/mnt/sdcard/pdpout/event%d.xml", eventrecvcnt);
  FILE *file=fopen(filename, "w+");
  fprintf(file, "%s", eventDoc);
  fclose(file);
  */

  eventrecvcnt++;
  xmlDocPtr doc=loadXMLmemory(eventDoc, strlen(eventDoc), FALSE);
  if(doc==NULL)
  {
    log_error("%s - Failure reading event!", __func__);
    char *errorResponse=malloc(20*sizeof(char));
    snprintf(errorResponse, 20, "<error/>");
    return errorResponse;
  }
  else
  {
    event_ptr levent=eventParseXML(pdp->actionDescStore, doc);
    if(levent==NULL)
    {
      log_error("Event parsing returned NULL-event");
      return "error";
    }

    #if PDP_PIPSOCKET == 0
      #warning "how is IF_update handled in Win32??"
      /// @todo IF_update should be directly handled by PEP!
      if(levent!=NULL)
      {
        log_trace("updating information flow");
        IF_update(levent);
      }
    #endif

    notifyResponse_ptr response=pdpNotifyEvent(levent);
    /// @todo xmlFreeDoc(doc) sometimes causes segfault if CDATA-node is contained in xml-document
    xmlFreeDoc(doc);
    levent->xmlDoc=NULL;  // otherwise double free in event_free
    char *serializedReponse=notifyResponseSerialize(response);
    notifyResponseFree(response); // including eventFree
    return serializedReponse;
  }
  return "error";
}

// PDP notification event with internal event structure
/// @todo return standard response depending on configuration in case of error!
notifyResponse_ptr pdpNotifyEvent(event_ptr levent)
{
  checkNullPtr(levent, "Got NULL-event as notification?!");

  log_debug("Searching for subscribed condition nodes for event=[%s]; subscribed nodes=[%d]",
      levent->actionDesc->actionName, g_slist_length(levent->actionDesc->conditions));

  g_slist_foreach(levent->actionDesc->conditions, (GFunc)conditionTriggerEvent, levent);


  log_debug("Searching for triggered mechanism for event=[%s]; subscribed mechanisms=[%d]", levent->actionDesc->actionName, g_slist_length(levent->actionDesc->mechanisms));
  notifyResponse_ptr response=notifyResponseNew(levent);
  if(response==NULL) {log_error("Error preparing new response for notification."); return NULL;}
  g_slist_foreach(levent->actionDesc->mechanisms, (GFunc)mechanismTriggerEvent, response);
  if(response->authorizationAction!=NULL) notifyResponseLog(response);
  return response;
}

unsigned int pdpDeployMechanism(const char *policyDocPath, const char *mechanismName)
{
  checkNullInt(policyDocPath, "Error: NULL parameter for [%s]", __func__);
  checkNullInt(mechanismName, "Error: NULL parameter for [%s]", __func__);
  log_trace("%s - %s", __func__, policyDocPath);
  unsigned int ret=loadPolicy(policyDocPath, mechanismName);
  log_trace("%s - loading mechanism returned [%s]", __func__, returnStr[ret]);
  return ret;
}

unsigned int pdpDeployPolicy(const char *policyDocPath)
{
  checkNullInt(policyDocPath, "Error: NULL parameter for [%s]", __func__);
  log_trace("%s - %s", __func__, policyDocPath);
  unsigned int ret=loadPolicy(policyDocPath, NULL);
  log_trace("%s - loading policy returned [%s]", __func__, returnStr[ret]);
  return ret;
}

unsigned int pdpDeployPolicyString(const char *policyString, const char *ns)
{
  checkNullInt(policyString, "Error: NULL parameter for [%s]", __func__);
  checkNullInt(ns,           "Error: NULL parameter for [%s]", __func__);
  log_trace("%s", __func__);
  unsigned int ret=loadPolicyString(policyString, strlen(policyString), ns, NULL);
  log_trace("%s - loading policy returned [%s]", __func__, returnStr[ret]);
  return ret;
}

unsigned int pdpDeployMechanismString(const char *policyString, const char *ns, const char *mechanismName)
{
  checkNullInt(policyString,  "Error: NULL parameter for [%s]", __func__);
  checkNullInt(ns,            "Error: NULL parameter for [%s]", __func__);
  checkNullInt(mechanismName, "Error: NULL parameter for [%s]", __func__);
  log_trace("%s", __func__);
  unsigned int ret=loadPolicyString(policyString, strlen(policyString), ns, mechanismName);
  log_trace("%s - loading policy returned [%s]", __func__, returnStr[ret]);
  return ret;
}


unsigned int pdpRevokeMechanism(const char *mechName, const char *ns)
{
  if(mechName==NULL) return R_ERROR;

  log_info("Revoking mechanism=[%s] in namespace=[%s]", mechName, ns);

  unsigned int mfnLength=strlen(mechName)+strlen(ns)+2;
  char *mechanismFullName=(char*)memCalloc(mfnLength, sizeof(char));
  snprintf(mechanismFullName, mfnLength, "%s#%s", ns, mechName);
  mechanismFullName[mfnLength-1]='\0'; // ensure terminating null-terminator
  log_trace("MechanismFullName for revoking=[%s]", mechanismFullName);

  if(g_hash_table_lookup(pdp->mechanismTable, mechanismFullName)!=NULL)
  {
    pdp->mutex->lock(pdp->mutex, INFINITE);
    g_hash_table_remove(pdp->mechanismTable, mechanismFullName);
    pdp->mutex->unlock(pdp->mutex);
    return R_SUCCESS;
  }
  else log_info("Mechanism doesn't exist in given namespace!");
  return R_ERROR;
}

unsigned int pdpRevokePolicy(const char *mechanismDocPath)
{
  checkNullInt(mechanismDocPath, "Error: NULL parameter for [%s]", __func__);
  xmlDocPtr    doc=NULL;
  xmlNodePtr   rootNode=NULL;

  doc=loadXMLdoc(mechanismDocPath);
  if(doc==NULL) {log_error("Error loading mechanism file!"); return R_ERROR;}
  rootNode=xmlDocGetRootElement(doc);
  unsigned int ret=searchMechanismNodes(rootNode, NULL, mechanismDocPath, TRUE);
  xmlFreeDoc(doc);
  if(ret==R_ERROR) {log_error("Error revoking policy"); return R_ERROR;}
  return R_SUCCESS;
}

char *pdpListDeployedMechanisms()
{
  char *policies=NULL;
  GList *polList=g_hash_table_get_values(pdp->mechanismTable);
  unsigned int curLength=0;
  int i;
  for(i=0; i<g_list_length(polList); i++)
  {
    char *curMechName=((mechanism_ptr)(g_list_nth(polList, i)->data))->mechName;
    int newLength=(curLength>0 ? (curLength+strlen(curMechName)+4) : strlen(curMechName)+1);
    policies=realloc(policies, newLength);
    if(curLength>0) policies=strcat(policies, "#$#");
    else policies[0]='\0';
    policies=strcat(policies, curMechName);
    curLength+=strlen(policies)+1;
  }
  g_list_free(polList);
  if(policies==NULL)
  {
    policies=memAlloc(12*sizeof(char));
    snprintf(policies, 12, "NO policies");
  }
  log_trace("List of deployed mechanisms=[%s]", policies);
  return policies;
}

unsigned int       pdpRegisterPEP(const char *name, pdpInterface_ptr linterface)
{
  checkNullInt(name,       "Error: NULL name for registering PEP");
  checkNullInt(linterface, "Error: NULL interface specification for registering PEP");

  pdpInterface_ptr sinterface=g_hash_table_lookup(pdp->registeredPEPs, name);
  if(sinterface==NULL || (linterface->type==PDP_INTERFACE_NATIVE))
  {
    switch(linterface->type)
    {
      case PDP_INTERFACE_NATIVE:
      { // handler should already be set by calling entity!
        if(linterface->pepSubscribe==NULL) {log_error("Require native method for native interface!"); return R_ERROR;}

        if(sinterface!=NULL && sinterface->pepSubscribe==pepSubscribeStub && linterface->pepSubscribe!=pepSubscribeStub)
        {
          log_debug("Received registration attempt for native interface; current is set to stub and new provides method!");
          sinterface->pepSubscribe=linterface->pepSubscribe;
        }
        else if(sinterface!=NULL)
          {log_debug("Received PEP registration with native interface, but PEP is already registered (not stub)!"); return R_ERROR;}
        break;
      }
      case PDP_INTERFACE_JNI:
      {
        linterface->pepSubscribe=pepSubscribeJNI;
        break;
      }
      case PDP_INTERFACE_SOCKET:
      {
        linterface->pepSubscribe=pepSubscribeSocket;
        break;
      }
      case PDP_INTERFACE_XMLRPC:
      {
        linterface->pepSubscribe=pepSubscribeXMLrpc;
        break;
      }
      default: log_error("Error: Unknown interface type"); return R_ERROR;
    }
    g_hash_table_insert(pdp->registeredPEPs, strdup(name), linterface);
  }
  else
  {
    log_warn("Received registration attempt for already existing PEP name; ignoring");
    return R_ERROR;
  }
  return R_SUCCESS;
}

unsigned int       pdpRegisterAction(const char *name, const char *pepID)
{
  checkNullInt(name,  "Require action name for registering action");
  checkNullInt(pepID, "Require pepID for registering action");
  pdpInterface_ptr pepInterface=g_hash_table_lookup(pdp->registeredPEPs, pepID);
  if(pepInterface!=NULL)
  {
    log_debug("Found a registered PEP for this ID");

    actionDescription_ptr actionDesc = actionDescriptionFind(pdp->actionDescStore, name, TRUE);
    if(actionDesc==NULL) {log_warn("Error inserting actionDescription to store!"); return R_ERROR;}
    log_debug("ActionDescription [%s] successfully found/added to store", name);

    if(actionDesc->pepInterface==NULL || actionDesc->pepInterface==pdpInterfaceStub)
      actionDesc->pepInterface=pepInterface;
    else
    {
      if(strncmp(pepID, pepInterface->name, strlen(pepID))==0)
      {
        log_trace("Trying to associate the action to the same PEP as it is already; skipping...");
        return R_SUCCESS;
      }
      log_warn("Action [%s] is already associated to another PEP. Multiple PEPs for one action are currently not supported.", name);
      return R_ERROR;
    }
    log_debug("Action [%s] successfully associated to PEP [%s]", name, pepID);
  }
  else
  {
    log_error("No PEP registered for the ID=[%s]", pepID);
    return R_ERROR;
  }
  return R_SUCCESS;
}

unsigned int       pdpRegisterPXP(const char *name, pdpInterface_ptr linterface)
{
  checkNullInt(name,       "Error: NULL name for registering PXP");
  checkNullInt(linterface, "Error: NULL interface specification for registering PXP");

  pdpInterface_ptr sinterface=g_hash_table_lookup(pdp->registeredPXPs, name);
  if(sinterface==NULL || (linterface->type==PDP_INTERFACE_NATIVE))
  {
    switch(linterface->type)
    {
      case PDP_INTERFACE_NATIVE:
      { // handler should already be set by calling entity!
        if(linterface->pxpExecute==NULL) {log_error("Require native method for native interface!"); return R_ERROR;}
        if(sinterface!=NULL && sinterface->pxpExecute==pxpExecuteStub && linterface->pxpExecute!=pxpExecuteStub)
        {
          log_debug("Received registration attempt for native interface; current is set to stub and new provides method!");
          sinterface->pxpExecute=linterface->pxpExecute;
        }
        else if(sinterface!=NULL)
          {log_debug("Received PXP registration with native interface, but PXP is already registered (not stub)!"); return R_ERROR;}
        break;
      }
      case PDP_INTERFACE_JNI:
      {
        linterface->pxpExecute=pxpExecuteJNI;
        break;
      }
      case PDP_INTERFACE_SOCKET:
      {
        linterface->pxpExecute=pxpExecuteSocket;
        break;
      }
      case PDP_INTERFACE_XMLRPC:
      {
        linterface->pxpExecute=pxpExecuteXMLrpc;
        break;
      }
      default: log_error("Error: Unknown interface type"); return R_ERROR;
    }
    g_hash_table_insert(pdp->registeredPXPs, strdup(name), linterface);
  }
  else
  {
    log_warn("Received registration attempt for already existing PXP name; ignoring");
    return R_ERROR;
  }
  return R_SUCCESS;
}

unsigned int       pdpRegisterExecutor(const char *name, const char *pxpID)
{
  checkNullInt(name,  "Require action name for registering action");
  checkNullInt(pxpID, "Require pxpID for registering action");
  pdpInterface_ptr pxpInterface=g_hash_table_lookup(pdp->registeredPXPs, pxpID);
  if(pxpInterface!=NULL)
  {
    log_debug("Found a registered PXP for this ID");

    actionDescription_ptr actionDesc = actionDescriptionFind(pdp->actionDescStore, name, TRUE);
    if(actionDesc==NULL) {log_warn("Error inserting actionDescription to store!"); return R_ERROR;}
    log_trace("ActionDescription [%s] successfully found/added to store", name);

    if(actionDesc->pxpInterface==NULL || actionDesc->pxpInterface==pdpInterfaceStub)
      actionDesc->pxpInterface=pxpInterface;
    else
    {
      if(strncmp(pxpID, pxpInterface->name, strlen(pxpID))==0)
      {
        log_trace("Trying to associate the action to the same PXP as it is already; skipping...");
        return R_SUCCESS;
      }
      log_warn("Execution action [%s] is already associated to another PXP. Multiple PXPs for one action are currently not supported.",
                name);
      return R_ERROR;
    }
    log_debug("Execution action [%s] successfully associated to PXP [%s]", name, pxpID);
  }
  else {log_error("No PXP registered for the ID=[%s]", pxpID);}
  return R_SUCCESS;
}

// stub methods for uninitialized pdp
bool               stubStart() {return FALSE;}
bool               stubStop()  {return FALSE;}
unsigned int       stubRevokeMechanism(const char *mechName, const char *ns) {return R_ERROR;}
unsigned int       stubRevokePolicy(const char *policyDocPath) {return R_ERROR;}
unsigned int       stubDeployMechanism(const char *policyDocPath, const char *mechanismName) {return R_ERROR;}
unsigned int       stubDeployPolicy(const char *policyDocPath) {return R_ERROR;}
char*              stubListDeployedMechanisms()
{
  char *ret=malloc(5*sizeof(char));
  snprintf(ret, 5, "STUB");
  return ret;
}
char*              stubNotifyEventXML(const char *eventDoc) {return NULL;}
notifyResponse_ptr stubNotifyEvent(event_ptr levent) {return NULL;}









