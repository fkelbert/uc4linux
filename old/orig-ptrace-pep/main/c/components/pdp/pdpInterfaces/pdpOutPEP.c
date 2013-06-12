/**
 * @file  pdpOutPEP.c
 * @brief Implementation of PDP OUT communication interface for subscribing to PEP
 *
 * @author cornelius moucha
 **/

#include "pdpOutPEP.h"
#include "log_pdpOutPEP_pef.h"

#ifdef PDP_JNI
  // PDP out interface: PEP subscription via JNI interface
  extern JavaVM *curjvm;
  unsigned int pepSubscribeJNI(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe)
  {
    if(curjvm==NULL)
    {
      log_error("ERROR! JVM is not set! Instantiating new JVM currently not supported.");
      return R_ERROR;
    }
    JNIEnv *lenv=NULL;
    (*curjvm)->AttachCurrentThread(curjvm, (void **)&lenv, NULL);

    jstring jniRequest=(*lenv)->NewStringUTF(lenv, name);
    int ret=(*lenv)->CallStaticIntMethod(lenv, linterface->classPEP, linterface->pepSubscriptionMethod, jniRequest, unsubscribe);
    /// @todo free jniRequest?
    log_debug("PEP returned for subscription=[%d]", ret);
    return ret;
  }
#endif

// PDP out interface: PEP subscription via Socket
unsigned int pepSubscribeSocket(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe)
{
  //log_error("PEP subscription via Socket not yet implemented");
  char *msg=(char*)memCalloc(strlen(name)+2,sizeof(char));
  snprintf(msg, strlen(name)+2, "%d%s", unsubscribe, name);
  char *response=linterface->socket->clientSend(linterface->socket, msg);
  log_debug("Received socket response for pepSubscribe=[%s]", response);
  free(msg);
  if(strncasecmp(response, "success", 7)==0) return R_SUCCESS;
  return R_ERROR;
}

// PDP out interface: PEP subscription via XML-RPC
unsigned int pepSubscribeXMLrpc(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe)
{
  log_error("PEP subscription via XML-RPC not yet implemented");
  return R_ERROR;
}

// PDP out interface: stub method for uninitialized interface
unsigned int pepSubscribeStub(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe)
{
  log_warn("pepSubscribeStub...");
  return R_ERROR;
}





