/**
 * @file  pdpCommInterfaces.c
 * @brief Implementation for out-going PDP communication to PIP
 *
 * schema: incoming interfaces: one file per interface type (e.g. one file for JNI, one for Socket, etc.)
 *         outgoing interfaces: one file per message type (e.g. PEP subscription, PIP init/eval, execute action, ...)
 *
 *
 * @author cornelius moucha
 **/

#include "pdpCommInterfaces.h"
#include "log_pdpCommInterfaces_pef.h"


#if PDP_XMLRPC == 1
  extern pefxmlrpc_ptr pdpInXMLRPC;
#endif
#if PDP_TCPSOCKET > 0
  extern pefSocket_ptr pdpInTCPsocket;
#endif
#if PDP_PIPSOCKET > 1
  extern pefSocket_ptr pdpOutPIPSocket;
#endif

extern JavaVM *curjvm;

unsigned int pdpInterfacesInit()
{
  // PDP input interface via TCP socket (PDP_TCPSOCKET => port)
  #if PDP_TCPSOCKET > 0
    //if((pdpInTCPsocket=pefsocketTCP_init(PDP_TCPSOCKET, &pdpSocketHandler))==NULL)
  if((pdpInTCPsocket=pefSocketServerNew(PEFSOCKET_TCP, NULL, PDP_TCPSOCKET, &pdpSocketHandler))==NULL)
    {
      log_error("PDP-TCPsocket (%d) could not be initialized!", PDP_TCPSOCKET);
      return R_ERROR;
    }
  #endif

  // PDP input interface via XML-RPC (PDP_XMLRPC => port)
  #if PDP_XMLRPC > 0
    if((pdpInXMLRPC=xmlrpcInit("/pdp", PDP_XMLRPC, &pdpXMLrpc_handler))==NULL)
    {
      log_error("XMLRPC-Interface could not be initialized!");
      return R_ERROR;
    }
  #endif

  #if PDP_PIPSOCKET == 0
    // local integrated IF-PIP (only for OpenBSD)
    log_debug("Initializing local IF tracking");
    data_flow_monitor = data_flow_monitor_new();
    data_flow_monitor_init(data_flow_monitor);
    dataCont_print(data_flow_monitor->map, -1);
    log_debug("Local IF tracking initialized");
  #endif

  // PDP output interface to PIP
  #if PDP_PIPSOCKET > 1
    if((pdpOutPIPSocket=pefsocketTCP_clientConnect("localhost", PDP_PIPSOCKET))==NULL )
    {
      log_error("PIP-Socket could not be initialized!");
      return R_ERROR;
    }
    log_debug("Successfully connected to pip-TCP-socket=[%d]", PDP_PIPSOCKET);
  #endif

  #if PDP_RMI > 0
    // testing java RMI interface
    if(initJVM(RMIJAR)==R_ERROR)
    {
      log_error("Could not initialize JVM! Aborting...");
      return R_ERROR;
    }
    JNIEnv *env=getJNIenvironment();

    jclass clsRMI=(*env)->FindClass(env, "de/fraunhofer/iese/pef/pdp/rmiServer/pdpRMIserver");
    if(clsRMI==NULL) {log_error("Could not find RMI-class."); return R_ERROR;}
    log_debug("pdpRMI class information retrieved");

    jmethodID rmiMainMathod=(*env)->GetStaticMethodID(env, clsRMI, "main", "([Ljava/lang/String;)V");
    if(rmiMainMathod==NULL) {log_error("Could not find main-method in pdpRMI-class."); return R_ERROR;}
    log_debug("RMI main method retrieved");

    char *portStr=memAlloc(12*sizeof(char));
    snprintf(portStr, 12, "%d", PDP_RMI);

    jstring jport=(*env)->NewStringUTF(env, portStr);
    if(jport==NULL) {log_warn("Error preparing argument list."); return R_ERROR;}
    jclass stringClass=(*env)->FindClass(env, "java/lang/String");
    if(stringClass==NULL) {log_warn("Error preparing argument list (String class not found)."); return R_ERROR;}
    jobjectArray args=(*env)->NewObjectArray(env, 1, stringClass, jport);
    if(args==NULL) {log_warn("Error creating argument list."); return R_ERROR;}

    (*env)->CallStaticVoidMethod(env, clsRMI, rmiMainMathod, args);

    log_info("RMI server instantiated");
  #endif

  pdpInterfaceStub=pdpInterfaceNew("stub", PDP_INTERFACE_NATIVE);
  return R_SUCCESS;
}

unsigned int pdpInterfacesStop()
{
  #if PDP_XMLRPC > 0
    if(xmlrpcFree(pdpInXMLRPC)==R_ERROR) log_error("Stopping XML-RPC interface FAILED!");
  #endif

  #if PDP_TCPSOCKET > 0
    if(pefSocketFree(pdpInTCPsocket)==R_ERROR) log_error("Stopping PDP-TCPsocket FAILED!");
  #endif

  #if PDP_PIPSOCKET > 1
    if(pefsocketTCP_free(pdpOutPIPSocket)==R_ERROR) log_error("Stopping PIP-Socket FAILED!");
  #endif

  log_debug("Deallocating stub-interface");
  if(pdpInterfaceStub!=NULL)
  {
    log_debug("Deallocating interface=[%p] (%d)", pdpInterfaceStub->name, pdpInterfaceStub->type);
    log_debug("Deallocating interface=[%s]", pdpInterfaceStub->name);
    if(pdpInterfaceStub->name!=NULL) free(pdpInterfaceStub->name);
    free(pdpInterfaceStub);
  }

  /// @todo free RMI interface
  // trying to deallocate RMI JVM
  #if PDP_RMI > 0
    log_debug("Stopping RMI JVM if instantiated manually.");
    unsigned int ret=stopJVM();
  #endif

  return R_SUCCESS;
}

pdpInterface_ptr pdpInterfaceStub;

pdpInterface_ptr pdpInterfaceNew(const char *name, unsigned int type)
{
  pdpInterface_ptr ninterface=(pdpInterface_ptr)memAlloc(sizeof(pdpInterface_t));
  ninterface->type=type;
  if(name!=NULL) ninterface->name=strdup(name);
  else ninterface->name=NULL;
  ninterface->socket=NULL;
  //log_debug("Allocating interface: name=[%s], name=[%p], interface=[%p]", name, name, ninterface);

  ninterface->pepSubscribe=pepSubscribeStub;
  ninterface->pxpExecute=pxpExecuteStub;

  ninterface->classPEP=NULL;
  ninterface->classPXP=NULL;
  ninterface->pepSubscriptionMethod=0;
  ninterface->pxpExecutionMethod=0;

  log_trace("Created new pdpInterface [%p] with name=[%p] %s", ninterface, ninterface->name, ninterface->name);
  return ninterface;
}

unsigned int pdpInterfaceFree(pdpInterface_ptr linterface)
{
  if(linterface==NULL || linterface->name==NULL) {log_debug("Skipping already deallocated interface... [%p] (name=%p)", linterface, linterface->name); return R_SUCCESS;}
  if(linterface==pdpInterfaceStub) {log_debug("Skipping stub-interface for deallocation!"); return R_SUCCESS;}

  log_trace("Deallocating interface: name=[%s], name=[%p], interface=[%p]", linterface->name, linterface->name, linterface);
  if(linterface->name!=NULL)
  {
    free(linterface->name);
    linterface->name=NULL;
  }

  #ifdef PDP_JNI
    extern JavaVM *curjvm;
    JNIEnv *lenv=NULL;
    if(curjvm!=NULL)
    {
      (*curjvm)->AttachCurrentThread(curjvm, (void **)&lenv, NULL);

      log_debug("Deallocating global reference for PEP class");
      if(linterface->classPEP!=NULL) (*lenv)->DeleteGlobalRef(lenv, linterface->classPEP);
      log_debug("Deallocating global reference for PXP class");
      if(linterface->classPXP!=NULL) (*lenv)->DeleteGlobalRef(lenv, linterface->classPXP);
    }
  #endif

  if(linterface->socket!=NULL)
  {
    if(pefSocketFree(linterface->socket)==R_ERROR) log_error("Error deallocating interface socket!");
    else log_debug("Interface socket deallocated");
  }

  free(linterface);
  return R_SUCCESS;
}



