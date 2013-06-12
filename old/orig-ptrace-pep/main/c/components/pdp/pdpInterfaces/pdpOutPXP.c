/**
 * @file  pdpOutPXP.c
 * @brief Implementation of PDP OUT communication interface for invoking PXP
 *
 * @author cornelius moucha
 **/

#include "pdpOutPXP.h"
#include "log_pdpOutPXP_pef.h"

#ifdef PDP_JNI
  // PDP out interface: PXP invocation via JNI interface
  extern JavaVM *curjvm;
  unsigned int pxpExecuteJNI(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params)
  {
    if(curjvm==NULL)
    {
      log_error("ERROR! JVM is not set! Instantiating new JVM currently not supported.");
      return R_ERROR;
    }
    JNIEnv *lenv=NULL;
    (*curjvm)->AttachCurrentThread(curjvm, (void **)&lenv, NULL);

    jobject paramList=(*lenv)->NewObject(lenv, jniArrayList.cls, jniArrayList.constructor);
    if(paramList==NULL) {log_error("ERROR: Could not create policyList object"); return R_ERROR;}
    int a;
    for(a=0; a<cntParams; a++)
    {
      jstring jname=(*lenv)->NewStringUTF(lenv, params[a]->paramDesc->name);
      jstring jvalue=(*lenv)->NewStringUTF(lenv, params[a]->value);
      jobject parameter=(*lenv)->NewObject(lenv, jniParam.cls, jniParam.constructor,
                                           jname, jvalue, 0);
      if(parameter==NULL) {log_error("ERROR: Could not create parameter object for modifiers"); return R_ERROR;}
      (*lenv)->CallVoidMethod(lenv, paramList, jniArrayList.add, parameter);
    }

    jstring jniRequest=(*lenv)->NewStringUTF(lenv, name);
    int ret=(*lenv)->CallStaticIntMethod(lenv, linterface->classPXP, linterface->pxpExecutionMethod, jniRequest, paramList);
    log_debug("PXP returned for execution=[%d]", ret);
    return ret;
  }
#endif

// PDP out interface: PXP invocation via Socket
unsigned int pxpExecuteSocket(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params)
{
  //log_error("PXP invocation via Socket not yet implemented");
  char *msg=(char*)memCalloc(strlen(name)+10,sizeof(char));
  snprintf(msg, strlen(name)+10, "%s#$#%d\r\n", name, cntParams);
  log_trace("sending to socket=[%s]", msg);
  char *response=linterface->socket->clientSend(linterface->socket, msg);
  log_debug("Received socket response for pepSubscribe=[%s]", response);
  free(msg);
  if(strncasecmp(response, "success", 7)==0) return R_SUCCESS;
  return R_ERROR;
}

// PDP out interface: PXP invocation via XML-RPC
unsigned int pxpExecuteXMLrpc(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params)
{
  log_error("PXP invocation via XML-RPC not yet implemented");
  return R_ERROR;
}

// PDP out interface: stub method for uninitialized interface
unsigned int pxpExecuteStub(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params)
{
  log_warn("pxpExecuteStub...");
  return R_ERROR;
}




