/**
 * @file  pdpInJNI.c
 * @brief Implementation for PDP communication interface using JNI
 *
 * Description of the communication interface using the Java Native Interface (JNI). These
 * methods are called directly from Java applications, they are not intended to be called from
 * native C code. The appropriate Java class should have the class name PolicyDecisionPoint in the
 * package de.fraunhofer.iese.pef.pdp. Otherwise Java will complain about linking problems with the
 * native shared library of the PDP.
 *
 * @see pdpjni.h
 *
 * @author cornelius moucha
 **/

#ifdef PDP_JNI

#include "pdpInJNI.h"
#include "log_pdpInJNI_pef.h"

extern pdp_ptr pdp;

extern JavaVM *curjvm;

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved)
{
  log_info("JNI_OnLoad started...");
  curjvm=jvm;

  JNIEnv *lenv=NULL;
  if ((*jvm)->GetEnv(jvm, (void **)&lenv, JNI_VERSION_1_6))
  {
    log_error("ERROR retrieving JNI environment from JVM");
    return JNI_ERR; // JNI version not supported
  }
  log_debug("JVM reference retrieved.");

  jclass lclsPDPinstance=(*lenv)->FindClass(lenv, "de/fraunhofer/iese/pef/pdp/PolicyDecisionPoint");
  jclass lclsDecision=(*lenv)->FindClass(lenv, "de/fraunhofer/iese/pef/pdp/internal/Decision");
  jclass lclsExecuteAction=(*lenv)->FindClass(lenv, "de/fraunhofer/iese/pef/pdp/internal/ExecuteAction");
  jclass lclsAuthorizationAction=(*lenv)->FindClass(lenv, "de/fraunhofer/iese/pef/pdp/internal/AuthorizationAction");
  jclass lclsEvent=(*lenv)->FindClass(lenv, "de/fraunhofer/iese/pef/pdp/internal/Event");
  jclass lclsParameter=(*lenv)->FindClass(lenv, "de/fraunhofer/iese/pef/pdp/internal/Param");

  jclass lclsArrayList=(*lenv)->FindClass(lenv, "java/util/ArrayList");
  jclass lclsInteger=(*lenv)->FindClass(lenv, "java/lang/Integer");
  jclass lclsBoolean=(*lenv)->FindClass(lenv, "java/lang/Boolean");
  jclass lclsByteArray=(*lenv)->FindClass(lenv, "[B");


  if(lclsPDPinstance==NULL || lclsDecision==NULL || lclsExecuteAction==NULL || lclsAuthorizationAction==NULL ||
     lclsEvent==NULL || lclsParameter==NULL || lclsArrayList==NULL || lclsInteger==NULL || lclsBoolean==NULL || lclsByteArray==NULL)
    {log_error("ERROR: Could not find required classes for initializing JNI environment!"); return JNI_ERR;}

  // creating global references of classes
  jniPDP.cls=(*lenv)->NewGlobalRef(lenv, lclsPDPinstance);
  jniDecision.cls=(*lenv)->NewGlobalRef(lenv, lclsDecision);
  jniExecuteAction.cls=(*lenv)->NewGlobalRef(lenv, lclsExecuteAction);
  jniAuthAction.cls=(*lenv)->NewGlobalRef(lenv, lclsAuthorizationAction);
  jniParam.cls=(*lenv)->NewGlobalRef(lenv, lclsParameter);
  jniArrayList.cls=(*lenv)->NewGlobalRef(lenv, lclsArrayList);
  jniEvent.cls=(*lenv)->NewGlobalRef(lenv, lclsEvent);
  jniInteger.cls=(*lenv)->NewGlobalRef(lenv, lclsInteger);
  jniBoolean.cls=(*lenv)->NewGlobalRef(lenv, lclsBoolean);
  jniByteArray.cls=(*lenv)->NewGlobalRef(lenv, lclsByteArray);

  if(jniPDP.cls==NULL || jniDecision.cls==NULL || jniExecuteAction.cls==NULL || jniAuthAction.cls==NULL ||
     jniEvent.cls==NULL || jniParam.cls==NULL || jniArrayList.cls==NULL || jniInteger.cls==NULL || jniBoolean.cls==NULL || jniByteArray.cls==NULL)
    {log_error("ERROR: Could not create global references for java classes!"); return JNI_ERR;}

  // delete local class references
  (*lenv)->DeleteLocalRef(lenv, lclsPDPinstance);
  (*lenv)->DeleteLocalRef(lenv, lclsDecision);
  (*lenv)->DeleteLocalRef(lenv, lclsExecuteAction);
  (*lenv)->DeleteLocalRef(lenv, lclsAuthorizationAction);
  (*lenv)->DeleteLocalRef(lenv, lclsParameter);
  (*lenv)->DeleteLocalRef(lenv, lclsArrayList);
  (*lenv)->DeleteLocalRef(lenv, lclsEvent);
  (*lenv)->DeleteLocalRef(lenv, lclsInteger);
  (*lenv)->DeleteLocalRef(lenv, lclsBoolean);
  (*lenv)->DeleteLocalRef(lenv, lclsByteArray);
  log_debug("JNI initialization: java class initialization finished...");

  jniDecision.constructor=(*lenv)->GetMethodID(lenv, jniDecision.cls, "<init>", "()V");
  jniDecision.addExecuteAction=(*lenv)->GetMethodID(lenv, jniDecision.cls, "addExecuteAction", "(Lde/fraunhofer/iese/pef/pdp/internal/ExecuteAction;)V");
  jniDecision.setAuthAction=(*lenv)->GetMethodID(lenv, jniDecision.cls, "setAuthorizationAction", "(Lde/fraunhofer/iese/pef/pdp/internal/AuthorizationAction;)V");

  jniExecuteAction.constructor=(*lenv)->GetMethodID(lenv, jniExecuteAction.cls, "<init>", "()V");
  jniExecuteAction.addParameter=(*lenv)->GetMethodID(lenv, jniExecuteAction.cls, "addParameter", "(Lde/fraunhofer/iese/pef/pdp/internal/Param;)V");
  jniExecuteAction.setName=(*lenv)->GetMethodID(lenv, jniExecuteAction.cls, "setName", "(Ljava/lang/String;)V");

  jniAuthAction.constructor=(*lenv)->GetMethodID(lenv, jniAuthAction.cls, "<init>", "()V");
  jniAuthAction.setType=(*lenv)->GetMethodID(lenv, jniAuthAction.cls, "setType", "(Z)V");
  jniAuthAction.setName=(*lenv)->GetMethodID(lenv, jniAuthAction.cls, "setName", "(Ljava/lang/String;)V");
  jniAuthAction.setDelay=(*lenv)->GetMethodID(lenv, jniAuthAction.cls, "setDelay", "(I)V");
  jniAuthAction.addModifier=(*lenv)->GetMethodID(lenv, jniAuthAction.cls, "addModifier", "(Lde/fraunhofer/iese/pef/pdp/internal/Param;)V");

  jniEvent.getEventName=(*lenv)->GetMethodID(lenv, jniEvent.cls, "getEventAction", "()Ljava/lang/String;");
  jniEvent.isTryEvent=(*lenv)->GetMethodID(lenv, jniEvent.cls, "isTryEvent", "()Z");
  jniEvent.getTimestamp=(*lenv)->GetMethodID(lenv, jniEvent.cls, "getTimestamp", "()J");
  jniEvent.getParams=(*lenv)->GetMethodID(lenv, jniEvent.cls, "getParams", "()Ljava/util/ArrayList;");

  jniArrayList.constructor=(*lenv)->GetMethodID(lenv, jniArrayList.cls, "<init>", "()V");
  jniArrayList.size=(*lenv)->GetMethodID(lenv, jniArrayList.cls, "size", "()I");
  jniArrayList.get=(*lenv)->GetMethodID(lenv, jniArrayList.cls, "get", "(I)Ljava/lang/Object;");
  jniArrayList.add=(*lenv)->GetMethodID(lenv, jniArrayList.cls, "add", "(Ljava/lang/Object;)Z");

  jniParam.constructor=(*lenv)->GetMethodID(lenv, jniParam.cls, "<init>", "(Ljava/lang/String;Ljava/lang/Object;I)V");
  jniParam.getName=(*lenv)->GetMethodID(lenv, jniParam.cls, "getName", "()Ljava/lang/String;");
  jniParam.getType=(*lenv)->GetMethodID(lenv, jniParam.cls, "getType", "()I");
  jniParam.getValue=(*lenv)->GetMethodID(lenv, jniParam.cls, "getValue", "()Ljava/lang/Object;");

  jniInteger.longValue=(*lenv)->GetMethodID(lenv, jniInteger.cls, "longValue", "()J");
  jniInteger.intValue=(*lenv)->GetMethodID(lenv, jniInteger.cls, "intValue", "()I");
  jniBoolean.booleanValue=(*lenv)->GetMethodID(lenv, jniBoolean.cls, "booleanValue", "()Z");

  if(jniDecision.constructor==NULL || jniDecision.addExecuteAction==NULL || jniDecision.setAuthAction==NULL ||
     jniExecuteAction.constructor==NULL || jniExecuteAction.addParameter==NULL || jniExecuteAction.setName==NULL ||
     jniAuthAction.constructor==NULL || jniAuthAction.setType==NULL || jniAuthAction.setName==NULL ||
     jniAuthAction.setDelay==NULL || jniAuthAction.addModifier==NULL ||
     jniEvent.getEventName==NULL || jniEvent.isTryEvent==NULL || jniEvent.getTimestamp==NULL || jniEvent.getParams==NULL ||
     jniArrayList.constructor==NULL || jniArrayList.size==NULL || jniArrayList.get==NULL || jniArrayList.add==NULL ||
     jniParam.constructor==NULL || jniParam.getName==NULL || jniParam.getType==NULL || jniParam.getType==NULL || jniParam.getValue==NULL ||
     jniInteger.longValue==NULL || jniInteger.intValue==NULL || jniBoolean.booleanValue==NULL )
    {log_error("ERROR: Could not find required methods IDs"); return JNI_ERR;}

  log_debug("JNI initialization: java method initialization finished...");

  // preparing standard decision objects
  jfieldID responseAllowID=(*lenv)->GetStaticFieldID(lenv, jniDecision.cls, "RESPONSE_ALLOW", "Lde/fraunhofer/iese/pef/pdp/internal/Decision;");
  jobject lresponseAllow=(*lenv)->GetStaticObjectField(lenv, jniDecision.cls, responseAllowID);

  jfieldID responseInhibitID=(*lenv)->GetStaticFieldID(lenv, jniDecision.cls, "RESPONSE_INHIBIT", "Lde/fraunhofer/iese/pef/pdp/internal/Decision;");
  jobject lresponseInhibit=(*lenv)->GetStaticObjectField(lenv, jniDecision.cls, responseInhibitID);

  jniDecision.allow=(*lenv)->NewGlobalRef(lenv, lresponseAllow);
  jniDecision.inhibit=(*lenv)->NewGlobalRef(lenv, lresponseInhibit);

  if(responseAllowID==NULL || jniDecision.allow==NULL || responseInhibitID==NULL || jniDecision.inhibit==NULL)
  {
    log_error("Error retrieving static decision response objects");
    return JNI_ERR;
  }
  (*lenv)->DeleteLocalRef(lenv, lresponseAllow);
  (*lenv)->DeleteLocalRef(lenv, lresponseInhibit);
  log_info("Static decision response objects retrieved");

  // requesting singleton instance of PolicyDecisionPoint
  //jniPDP.getInstance=(*lenv)->GetStaticMethodID(lenv, jniPDP.cls, "getInstance", "()Lde/fraunhofer/iese/pef/pdp/IPolicyDecisionPoint;");
  jniPDP.getInstance=(*lenv)->GetStaticMethodID(lenv, jniPDP.cls, "getInstance", "()Lde/fraunhofer/iese/pef/pdp/internal/IPolicyDecisionPoint;");
  if(jniPDP.getInstance==NULL) {log_error("Error resolving methodID for getDefault in PDP class"); return JNI_ERR;}

  jobject lpdpSingletonInstance=(jobject)(*lenv)->CallStaticObjectMethod(lenv, jniPDP.cls, jniPDP.getInstance);
  if(lpdpSingletonInstance==NULL) {log_error("Error retrieving PDP instance object"); return JNI_ERR;}
  jniPDP.instance=(*lenv)->NewGlobalRef(lenv, lpdpSingletonInstance);
  if(jniPDP.instance==NULL) {log_error("ERROR: Could not create global reference for PDP object!"); return JNI_ERR;}
  (*lenv)->DeleteLocalRef(lenv, lpdpSingletonInstance);

  // Initializing PIP connection (if set to JNI)
  #if PDP_PIPSOCKET == 1
    log_debug("Initializing PIP connection via JNI");
    // todo should be done via registry and interfaceDescription!
    //jclass lclsPIPinstance=(*lenv)->FindClass(lenv, "de/fraunhofer/iese/pef/pdp/example/pipExample");
    jclass lclsPIPinstance=(*lenv)->FindClass(lenv, "main/PIPHandler");
    if(lclsPIPinstance==NULL)
      {log_error("ERROR: Could not find required classes for initializing PIP connection!"); return JNI_ERR;}

    // creating global references of classes
    jniPIP.cls=(*lenv)->NewGlobalRef(lenv, lclsPIPinstance);
    (*lenv)->DeleteLocalRef(lenv, lclsPIPinstance);

    jniPIP.getInstance=(*lenv)->GetStaticMethodID(lenv, jniPIP.cls, "getInstance", "()Lhelper/IPIPCommunication;");
    if(jniPIP.getInstance==NULL) {log_error("Error resolving methodID for getDefault in PIP class"); return JNI_ERR;}

    jobject lpipSingletonInstance=(jobject)(*lenv)->CallStaticObjectMethod(lenv, jniPIP.cls, jniPIP.getInstance);
    if(lpipSingletonInstance==NULL) {log_error("Error retrieving PDP instance object"); return JNI_ERR;}
    jniPIP.instance=(*lenv)->NewGlobalRef(lenv, lpipSingletonInstance);
    if(jniPIP.instance==NULL) {log_error("ERROR: Could not create global reference for PIP object!"); return JNI_ERR;}
    (*lenv)->DeleteLocalRef(lenv, lpipSingletonInstance);

    jniPIP.handlePIPeval=(*lenv)->GetMethodID(lenv, jniPIP.cls, "eval", "(Ljava/lang/String;Ljava/lang/String;)I");
    jniPIP.handlePIPinit=(*lenv)->GetMethodID(lenv, jniPIP.cls, "init", "(Ljava/lang/String;)Ljava/lang/String;");
    jniPIP.handlePIPinitDataID=(*lenv)->GetMethodID(lenv, jniPIP.cls, "init", "(Ljava/lang/String;Ljava/lang/String;)Ljava/lang/String;");

    if(jniPIP.handlePIPeval==NULL || jniPIP.handlePIPinit==NULL || jniPIP.handlePIPinitDataID==NULL)
      {log_error("ERROR: Could not find required methods IDs for PIP!"); return JNI_ERR;}

    log_debug("PIP initialization finished");

  #endif

  log_info("JNI initialization finished");
  return JNI_VERSION_1_6;
}


JNIEXPORT void JNICALL JNI_OnUnload(JavaVM *jvm, void *reserved)
{ // this method seems to be not called?
  printf("bliblablub...\n\n");
  fflush(stdout);

  log_info("JNI deallocation started...\n\n");
  JNIEnv *lenv=NULL;
  if ((*jvm)->GetEnv(jvm, (void **)&lenv, JNI_VERSION_1_6))
  {
    log_error("ERROR retrieving JNI environment from JVM");
    return;
  }

  (*lenv)->DeleteGlobalRef(lenv, jniDecision.allow);
  (*lenv)->DeleteGlobalRef(lenv, jniDecision.inhibit);
  (*lenv)->DeleteGlobalRef(lenv, jniPDP.cls);
  (*lenv)->DeleteGlobalRef(lenv, jniDecision.cls);
  (*lenv)->DeleteGlobalRef(lenv, jniExecuteAction.cls);
  (*lenv)->DeleteGlobalRef(lenv, jniAuthAction.cls);
  (*lenv)->DeleteGlobalRef(lenv, jniParam.cls);
  (*lenv)->DeleteGlobalRef(lenv, jniArrayList.cls);
  (*lenv)->DeleteGlobalRef(lenv, jniEvent.cls);
  (*lenv)->DeleteGlobalRef(lenv, jniInteger.cls);
  (*lenv)->DeleteGlobalRef(lenv, jniBoolean.cls);
  (*lenv)->DeleteGlobalRef(lenv, jniByteArray.cls);

  log_info("JNI_OnUNLoad finished...");
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpStart(JNIEnv *env, jobject jobj)
{
  if(curjvm==NULL)
  {
    log_warn("Trying to retrieve JVM instance; should be set in JNI_onLoad!");
    if( (*env)->GetJavaVM(env, &curjvm) < 0)
    {
      log_error("Could not retrieve reference to current java VM!");
      return R_ERROR;
    }
    log_info("JVM reference successfully stored.");
    if(JNI_OnLoad(curjvm, NULL)<0) {log_error("Error initializing JNI environment!"); return R_ERROR;}
  }
  return pdp->start();
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpStop(JNIEnv *env, jobject jobj)
{
  return pdp->stop();
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpDeployPolicy(JNIEnv *env, jobject jobj, jstring jPolicyDocPath)
{
  const char *mechanismDocPath=(*env)->GetStringUTFChars(env, jPolicyDocPath, 0);
  unsigned int ret=pdp->deployPolicy(mechanismDocPath);
  (*env)->ReleaseStringUTFChars(env, jPolicyDocPath, mechanismDocPath);
  return ret;
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpDeployPolicyString(JNIEnv *env, jobject jobj, jstring jPolicyString)
{
  const char *policyString=(*env)->GetStringUTFChars(env, jPolicyString, 0);
  //const char *policyNamespace=(*env)->GetStringUTFChars(env, jPolicyDocPath, 0);
  unsigned int ret=pdp->deployPolicyString(policyString);
  (*env)->ReleaseStringUTFChars(env, jPolicyString, policyString);
  return ret;
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpDeployMechanism(JNIEnv *env, jobject jobj,
      jstring jPolicyDocPath, jstring jMechName)
{
  const char *policyDocPath=(*env)->GetStringUTFChars(env, jPolicyDocPath, 0);
  const char *mechName=(*env)->GetStringUTFChars(env, jMechName, 0);
  unsigned int ret=pdp->deployMechanism(policyDocPath, mechName);
  (*env)->ReleaseStringUTFChars(env, jPolicyDocPath, policyDocPath);
  (*env)->ReleaseStringUTFChars(env, jMechName, mechName);
  return ret;
}

JNIEXPORT jstring JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_listDeployedMechanisms(JNIEnv *env, jobject jobj)
{
  log_info("listDeployedMechanisms...");
  char *ret=NULL;
  ret=pdp->listDeployedMechanisms();
  jstring response=(*env)->NewStringUTF(env, ret);
  log_info("listDeployedMechanisms finished...");
  // free of ret results in free(): invalid next size (fast): 0x6ea34568 *** ???
  /// @todo: test in testpdp! working in testjni
  //free(ret);
  return response;
}

JNIEXPORT jobject JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_listDeployedMechanismsJNI(JNIEnv *env, jobject jobj)
{
  jobject policyList=(*env)->NewObject(env, jniArrayList.cls, jniArrayList.constructor);
  if(policyList==NULL) {log_error("ERROR: Could not create policyList object"); return NULL;}

  GHashTableIter mechIterator;
  gpointer key, value;
  g_hash_table_iter_init(&mechIterator, pdp->mechanismTable);

  while(g_hash_table_iter_next(&mechIterator, &key, &value))
  {
    jstring jmechName=(*env)->NewStringUTF(env, key);
    (*env)->CallVoidMethod(env, policyList, jniArrayList.add, jmechName);
    // ReleaseString => corrupted double-linked list: 0x6f1d7118
    //(*env)->ReleaseStringUTFChars(env, jmechName, key);
  }
  return policyList;
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpRevokeMechanism(JNIEnv *env, jobject jobj, jstring jMechanismName, jstring jns)
{
  const char *mechanismName=(*env)->GetStringUTFChars(env, jMechanismName, 0);
  const char *ns=(*env)->GetStringUTFChars(env, jns, 0);
  unsigned int ret=pdp->revokeMechanism(mechanismName, ns);
  (*env)->ReleaseStringUTFChars(env, jMechanismName, mechanismName);
  (*env)->ReleaseStringUTFChars(env, jns, ns);
  return ret;
}

JNIEXPORT jstring JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpNotifyEventXML(JNIEnv *env, jobject jobj, jstring jeventDoc)
{
  const char *eventDoc = (*env)->GetStringUTFChars(env, jeventDoc, 0);
  char *ret=pdp->notifyEventXML(eventDoc);
  (*env)->ReleaseStringUTFChars(env, jeventDoc, eventDoc);
  jstring response=(*env)->NewStringUTF(env, ret);
  free(ret);
  return response;
}

event_ptr eventParseJNI(JNIEnv *env, jobject jeventObject)
{
  jstring jeventName=(jstring)(*env)->CallObjectMethod(env, jeventObject, jniEvent.getEventName);
  if(jeventName==NULL) {log_error("ERROR: Extracting field in PDP event object"); return NULL;}
  const char *eventName=(*env)->GetStringUTFChars(env, jeventName, 0);

  jboolean istry=(jboolean)(*env)->CallBooleanMethod(env, jeventObject, jniEvent.isTryEvent);

  actionDescription_ptr actionDesc=pdp->actionDescStore->find(pdp->actionDescStore, eventName, TRUE);
  if(actionDesc==NULL) {log_warn("JNI-event: No action found for actionName=[%s]", eventName); return NULL;}

  event_ptr nEvent=NULL;
  nEvent = eventNew(actionDesc, istry);
  checkNullPtr(nEvent, "Failure parsing event: event_match_op could not be created!");

  // cleanup 1st step
  (*env)->ReleaseStringUTFChars(env, jeventName, eventName);
  nEvent->timestamp = timestampNewEmpty();

  jlong timestamp=(jlong)(*env)->CallLongMethod(env, jeventObject, jniEvent.getTimestamp);
  nEvent->timestamp->usec=timestamp;

  jobject eventParams=(jobject)(*env)->CallObjectMethod(env, jeventObject, jniEvent.getParams);
  if(eventParams==NULL) {log_error("ERROR: Could not retrieve required field [params] in PDP event"); return NULL;}
  jint eventParameterSize=(jint)(*env)->CallIntMethod(env, eventParams, jniArrayList.size);

  int xi;
  for(xi=0; xi < eventParameterSize; xi++)
  {
    log_debug("Retrieving parameter [%d]", xi);
    jobject param=(*env)->CallObjectMethod(env, eventParams, jniArrayList.get, xi);
    if(param==NULL) {log_error("ERROR: Could not retrieve required object (eventParameter)"); return NULL;}

    jstring eventParameterName=(jstring) (*env)->CallObjectMethod(env, param, jniParam.getName);
    if(eventParameterName==NULL) {log_error("ERROR: Could not retrieve parameterName"); return NULL;}
    const char *pname=(*env)->GetStringUTFChars(env, eventParameterName, 0);

    jint eventParameterType=(jint)(*env)->CallIntMethod(env, param, jniParam.getType);
    jobject valueObject=(*env)->CallObjectMethod(env, param, jniParam.getValue);
    if(valueObject==NULL) {log_error("Could not retrieve object for value!!"); return NULL;}

    log_debug("Processing parameter type... [%d]", eventParameterType);
    switch(eventParameterType)
    {
      case 0: // PARAMETER_TYPE_STRING
      case 1: // PARAMETER_TYPE_DATAUSAGE   // should not be used for event, only for eventMatch!
      case 2: // PARAMETER_TYPE_XPATH       // should not be used for event, only for eventMatch!
      case 3: // PARAMETER_TYPE_REGEX       // should not be used for event, only for eventMatch!
      case 4: // PARAMETER_TYPE_CONTEXT     // should not be used for event, only for eventMatch!
      {
        const char *pvalue=(*env)->GetStringUTFChars(env, (jstring)valueObject, 0);
        eventAddParamString(nEvent, pname, pvalue);
        (*env)->ReleaseStringUTFChars(env, valueObject, pvalue);
        break;
      }
      case 5: // PARAMETER_TYPE_BINARY
      {
        log_info("processing binary param value (byteArray)");
        //jbyteArray pval=(jbyteArray)(valueObject);
        //jsize bytearraysize=(*env)->GetArrayLength(env, pval);
        //log_info("Byte array size: [%d]", bytearraysize);

        //jbyte *pvalelements=(*env)->GetByteArrayElements(env, pval, 0);
        //int xx=0;
        //printf("byte array: [");
        //for(xx=0; xx<bytearraysize; xx++)
        //  printf("%d ", pvalelements[xx]);
        //printf("]\n");
        log_error("ignoring binary data (byteArray) for event...");
        break;
      }
      case 6: // PARAMETER_TYPE_INT
      {
        int pvalue=(jint)(*env)->CallIntMethod(env, valueObject, jniInteger.intValue);
        eventAddParamInt(nEvent, pname, pvalue);
        break;
      }
      case 7: // PARAMETER_TYPE_LONG
      {
        long long pvalue=(jlong)(*env)->CallLongMethod(env, valueObject, jniInteger.longValue);
        eventAddParamLong(nEvent, pname, pvalue);
        break;
      }
      case 8: // PARAMETER_TYPE_BOOL
      {
        log_debug("retrieving bool parameter...");
        int pvalue=(jboolean)(*env)->CallBooleanMethod(env, valueObject, jniBoolean.booleanValue);
        eventAddParamInt(nEvent, pname, pvalue);
        break;
      }
      case 9: // PARAMETER_TYPE_STRINGARRAY
      default:
      {
        log_error("ignoring string array (or unknown parameter type) for event...");
        break;
      }
    }

    // cleanup 2nd step
    (*env)->DeleteLocalRef(env, valueObject);
    (*env)->ReleaseStringUTFChars(env, eventParameterName, pname);
    log_debug("finished freeing temporary data...");
  }

  log_debug("finished parameter processing...");
  log_debug("Successfully parsed JNI event and created event referencing action \"%s\"",nEvent->actionDesc->actionName);
  return nEvent;
}

jobject prepareJNIresponse(JNIEnv *env, notifyResponse_ptr pdpResponse)
{
  checkNullPtr(pdpResponse, "Could not process NULL response from notifyEventJNI?!");
  if(pdpResponse->authorizationAction==NULL)
  {
    log_debug("authorizationAction is NULL ==> returning static RESPONSE_ALLOW");
    return jniDecision.allow;
  }

  jobject decision=(*env)->NewObject(env, jniDecision.cls, jniDecision.constructor);
  if(decision==NULL) {log_error("ERROR: Could not create decision object"); return NULL;}

  // copy authorizationActions
  jobject authorizationAction=(*env)->NewObject(env, jniAuthAction.cls, jniAuthAction.constructor);
  if(authorizationAction==NULL) {log_error("ERROR: Could not create authorizationAction object"); return NULL;}

  int i,j;
  if(pdpResponse->authorizationAction!=NULL)
  {
    // setting authorizationAction to ALLOW/INHIBIT
    (*env)->CallVoidMethod(env, authorizationAction, jniAuthAction.setType, pdpResponse->authorizationAction->response);
    jstring authActionName=(*env)->NewStringUTF(env, "bliblablub");
    (*env)->CallVoidMethod(env, authorizationAction, jniAuthAction.setName, authActionName);
    log_warn("name set to [bliblablub]");

    if(pdpResponse->authorizationAction->delay>0)
      (*env)->CallVoidMethod(env, authorizationAction, jniAuthAction.setDelay, pdpResponse->authorizationAction->delay);

    if(pdpResponse->authorizationAction->cntParamsModify>0)
    {
      for(i=0; i<pdpResponse->authorizationAction->cntParamsModify; i++)
      {
        jstring jname=(*env)->NewStringUTF(env, pdpResponse->authorizationAction->paramsModify[i]->paramDesc->name);
        jstring jvalue=(*env)->NewStringUTF(env, pdpResponse->authorizationAction->paramsModify[i]->value);
        jobject parameter=(*env)->NewObject(env, jniParam.cls, jniParam.constructor,
                                            jname,
                                            jvalue,
                                            0);
        if(parameter==NULL) {log_error("ERROR: Could not create parameter object for modifiers"); return NULL;}
        (*env)->CallVoidMethod(env, authorizationAction, jniAuthAction.addModifier, parameter);
      }
    }
    (*env)->CallVoidMethod(env, decision, jniDecision.setAuthAction, authorizationAction);
  }

  log_debug("Creating global reference to decision object");
  jobject grDecision=(*env)->NewGlobalRef(env, decision);
  (*env)->DeleteLocalRef(env, decision);

  log_info("JNI decision object successfully created");
  return grDecision;
}

JNIEXPORT jobject JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpNotifyEventJNI(JNIEnv *env, jobject jobj, jobject jeventObject)
{ // 32 = number of local references; arbitrary value?
  if((*env)->PushLocalFrame(env, 32)<0) {log_error("Error creating local frame for native method"); return NULL;}

  event_ptr levent=eventParseJNI(env, jeventObject);
  notifyResponse_ptr pdpResponse=pdp->notifyEvent(levent);
  jobject retVal=prepareJNIresponse(env, pdpResponse);
  notifyResponseFree(pdpResponse); // including eventFree
  (*env)->PopLocalFrame(env, retVal);
  return retVal;
}


JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_setRuntimeLogLevel(JNIEnv *env, jobject jobj, jint newLevel)
{
  return pefLogSetRTLevel(newLevel);
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_registerPEP
  (JNIEnv *env, jobject jobj, jstring jPEPname, jstring jClassName, jstring jMethodName, jstring jMethodSignature)
{
  if((*env)->PushLocalFrame(env, 32)<0) {log_error("Error creating local frame for native method"); return R_ERROR;}

  const char *pepName=(*env)->GetStringUTFChars(env, jPEPname, 0);
  pdpInterface_ptr ninterface=pdpInterfaceNew(pepName, PDP_INTERFACE_JNI);

  const char *className=(*env)->GetStringUTFChars(env, jClassName, 0);
  jclass lclsPEP=(*env)->FindClass(env, className);
  if(lclsPEP==NULL) {log_error("ERROR: Could not find required classes for PEP registering!"); return R_ERROR;}
  (*env)->ReleaseStringUTFChars(env, jClassName, className);

  // creating global references of class
  ninterface->classPEP=(*env)->NewGlobalRef(env, lclsPEP);
  if(ninterface->classPEP==NULL) {log_error("ERROR: Could not find required classes for PEP registering!"); return R_ERROR;}

  const char *methodName=(*env)->GetStringUTFChars(env, jMethodName, 0);
  const char *methodSignature=(*env)->GetStringUTFChars(env, jMethodSignature, 0);
  ninterface->pepSubscriptionMethod=(*env)->GetStaticMethodID(env, ninterface->classPEP, methodName, methodSignature);
  if(ninterface->pepSubscriptionMethod==NULL) {log_error("Error resolving methodID for PEP requests"); return R_ERROR;}
  (*env)->ReleaseStringUTFChars(env, jMethodName, methodName);
  (*env)->ReleaseStringUTFChars(env, jMethodSignature, methodSignature);

  unsigned int ret=pdpRegisterPEP(pepName, ninterface);
  log_debug("[JNI] Native PEP registration=[%u]", ret);
  (*env)->ReleaseStringUTFChars(env, jPEPname, pepName);

  (*env)->PopLocalFrame(env, NULL);
  return ret;
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_registerAction
  (JNIEnv *env, jobject jobj, jstring jActionName, jstring jPEPname)
{
  const char *actionName=(*env)->GetStringUTFChars(env, jActionName, 0);
  const char *pepName=(*env)->GetStringUTFChars(env, jPEPname, 0);

  unsigned int ret=pdpRegisterAction(actionName, pepName);
  log_debug("[JNI] Native action registration=[%u]", ret);
  (*env)->ReleaseStringUTFChars(env, jPEPname, pepName);
  (*env)->ReleaseStringUTFChars(env, jActionName, actionName);

  return ret;
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_registerPXP
  (JNIEnv *env, jobject jobj, jstring jPXPname, jstring jClassName, jstring jMethodName, jstring jMethodSignature)
{
  if((*env)->PushLocalFrame(env, 32)<0) {log_error("Error creating local frame for native method"); return R_ERROR;}

  const char *pxpName=(*env)->GetStringUTFChars(env, jPXPname, 0);
  pdpInterface_ptr ninterface=pdpInterfaceNew(pxpName, PDP_INTERFACE_JNI);

  const char *className=(*env)->GetStringUTFChars(env, jClassName, 0);
  jclass lclsPXP=(*env)->FindClass(env, className);
  if(lclsPXP==NULL) {log_error("ERROR: Could not find required classes for PXP registering!"); return R_ERROR;}
  (*env)->ReleaseStringUTFChars(env, jClassName, className);

  ninterface->classPXP=(*env)->NewGlobalRef(env, lclsPXP);
  if(ninterface->classPXP==NULL) {log_error("ERROR: Could not find required classes for PXP registering!"); return R_ERROR;}

  const char *methodName=(*env)->GetStringUTFChars(env, jMethodName, 0);
  const char *methodSignature=(*env)->GetStringUTFChars(env, jMethodSignature, 0);
  ninterface->pxpExecutionMethod=(*env)->GetStaticMethodID(env, ninterface->classPXP, methodName, methodSignature);
  if(ninterface->pxpExecutionMethod==NULL) {log_error("Error resolving methodID for PXP requests"); return R_ERROR;}
  (*env)->ReleaseStringUTFChars(env, jMethodName, methodName);
  (*env)->ReleaseStringUTFChars(env, jMethodSignature, methodSignature);

  unsigned int ret=pdpRegisterPXP(pxpName, ninterface);
  log_debug("[JNI] Native PXP registration=[%u]", ret);
  (*env)->ReleaseStringUTFChars(env, jPXPname, pxpName);

  (*env)->PopLocalFrame(env, NULL);
  return ret;
}

JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_registerExecutor
  (JNIEnv *env, jobject jobj, jstring jActionName, jstring jPXPname)
{
  const char *actionName=(*env)->GetStringUTFChars(env, jActionName, 0);
  const char *pxpName=(*env)->GetStringUTFChars(env, jPXPname, 0);

  unsigned int ret=pdpRegisterExecutor(actionName, pxpName);
  log_debug("[JNI] Native executor registration=[%u]", ret);
  (*env)->ReleaseStringUTFChars(env, jPXPname, pxpName);
  (*env)->ReleaseStringUTFChars(env, jActionName, actionName);

  return ret;
}


#endif




