/**
 * @file  pdpOutPIP.c
 * @brief Implementation of PDP OUT communication interface for PIP requests
 *
 * Provide interfaces for outgoing communication to the PIP for initialization and evaluation requests.
 *
 * @remark the methods representationRefinesData and initialRepresentation are only used for OpenBSD-PIP in combination
 *         with the systrace-PEP and should not be used. They will be replaced with pipEval and pipInit!
 * @author cornelius moucha
 **/

#include "pdpOutPIP.h"
#include "log_pdpOutPIP_pef.h"

// // PDP OUT interface to PIP; native (only for OpenBSD)
#if PDP_PIPSOCKET == 0
  /// @todo should be unified using pipInit and pipEval!
  unsigned int representationRefinesData(char *rep, char *dataID)
  {
    char *tmpcont=NULL;
    list_of_data_ptr lod=NULL;
    //Let's check if data stored in file A is B
    tmpcont=dataCont_getNaming(data_flow_monitor->map,"FNAME", rep);
    if(tmpcont==NULL)
    { //no naming for A -> no data in the file
      return FALSE;
    }

    lod=dataCont_getDataIn(data_flow_monitor->map, tmpcont);
    if (list_of_data_length(lod)==0)
    { // no data in file A
      return FALSE;
    }

    if(list_of_data_find(lod, dataID)==0)
      //log_info("  [%s] \"refines\" [%s]", rep, dataID);
      return TRUE;
    else
      //log_info("  [%s] doesn't \"refine\" [%s]", rep, dataID);
      return FALSE;
  }

  // data_flow_monitor.c: given a container and a quantity of data in it, generates a new mapping
  char *initialRepresentation(char *cont, char *qod)
  {
    return (char*)IF_initCont(cont, qod);
  }

  int   pipEval(char *method, char *params)   {return FALSE;}
  char* pipInit(char *method, char *params)
  {
    char *retVal=malloc(8*sizeof(char));
    snprintf(retVal, 8, "NOVALUE");
    return retVal;
  }
#endif

// PDP OUT interface to PIP; stubs returning standard values
#if PDP_PIPSOCKET == -1
  unsigned int representationRefinesData(char *rep, char *dataID) {return FALSE;}
  char*        initialRepresentation(char *cont, char *qod)
  {
    char *retVal=malloc(8*sizeof(char));
    snprintf(retVal, 8, "NOVALUE");
    return retVal;
  }

  int   pipEval(char *method, char *params)   {return FALSE;}
  char* pipInit(char *method, char *params)
  {
    log_debug("pipInit stub");
    char *retVal=malloc(8*sizeof(char));
    snprintf(retVal, 8, "NOVALUE");
    return retVal;
  }

  char* pipInitDataID(char *method, char *param, char *initDataID)
  {
    log_debug("pipInit stub");
    char *retVal=malloc(8*sizeof(char));
    snprintf(retVal, 8, "NOVALUE");
    return retVal;
  }
#endif



// PDP OUT interface to PIP; communication via JNI-interface (PDP_PIPSOCKET==1)
#if PDP_PIPSOCKET == 1
  unsigned int representationRefinesData(char *rep, char *dataID)
  { /// @todo forward to pipEval(...)
    return TRUE;
  }
  char *initialRepresentation(char *cont, char *qod)
  { /// @todo forward to pipInit(...)
    return "NOVALUE";
  }

  extern JavaVM *curjvm;
  int pipEval(char *method, char *params)
  { // return -1, because 0 => FALSE and 1 => TRUE
    if(curjvm==NULL) {log_error("ERROR! JVM is not set! -> returning -1..."); return -1;}
    JNIEnv *lenv=NULL;
    (*curjvm)->AttachCurrentThread(curjvm, (void **)&lenv, NULL);
    if(lenv==NULL) {log_error("ERROR! JNI environment could not be retrieved! -> returning errorStr..."); return -1;}

    jstring jniRequest=(*lenv)->NewStringUTF(lenv, method);
    jstring jniParams=(*lenv)->NewStringUTF(lenv, params);

    //int ret=(*lenv)->CallIntMethod(lenv, jniPDP.instance, jniPDP.handlePIPeval, jniRequest, jniParams);
    int ret=(*lenv)->CallIntMethod(lenv, jniPIP.instance, jniPIP.handlePIPeval, jniRequest, jniParams);
    //(*lenv)->ReleaseStringUTFChars(lenv, jniRequest, jniRequest);
    log_debug("PIP returned=[%d]", ret);
    return ret;
  }

  // method -> deployContext        ; params -> contextDescription
  // method -> initialRepresentation; params -> representation
  char* pipInit(char *method, char *param)
  {
    log_debug("pipInit JNI");
    if(curjvm==NULL) {log_error("ERROR! JVM is not set! -> returning errorStr..."); return errorStr;}
    JNIEnv *lenv=NULL;
    (*curjvm)->AttachCurrentThread(curjvm, (void **)&lenv, NULL);
    if(lenv==NULL) {log_error("ERROR! JNI environment could not be retrieved! -> returning errorStr..."); return errorStr;}
    if((*lenv)->PushLocalFrame(lenv, 32)<0) {log_error("Error creating local frame for native method"); return errorStr;}

    //jstring jniMethod=(*lenv)->NewStringUTF(lenv, method);
    jstring jniParam=(*lenv)->NewStringUTF(lenv, param);
    jstring retStr=(jstring)(*lenv)->CallObjectMethod(lenv, jniPIP.instance, jniPIP.handlePIPinit, jniParam);
    char *strval=(char*)(*lenv)->GetStringUTFChars(lenv, retStr, 0);
    log_debug("PIP returned=[%s]", strval);

    (*lenv)->PopLocalFrame(lenv, NULL);
    return strval;
  }

  //pvalue=(char*)pipInitDataID(pvalue, pqod, initDataID);
  // method -> deployContext        ; params -> contextDescription; initDataID -> null
  // method -> initialRepresentation; params -> representation    ; initDataID -> given dataID
  char* pipInitDataID(char *method, char *param, char *initDataID)
  {
    log_debug("pipInit JNI");
    if(curjvm==NULL) {log_error("ERROR! JVM is not set! -> returning errorStr..."); return errorStr;}
    JNIEnv *lenv=NULL;
    (*curjvm)->AttachCurrentThread(curjvm, (void **)&lenv, NULL);
    if(lenv==NULL) {log_error("ERROR! JNI environment could not be retrieved! -> returning errorStr..."); return errorStr;}
    if((*lenv)->PushLocalFrame(lenv, 32)<0) {log_error("Error creating local frame for native method"); return errorStr;}

    //jstring jniMethod=(*lenv)->NewStringUTF(lenv, method);
    jstring jniParam=(*lenv)->NewStringUTF(lenv, param);
    jstring jniDataID=(*lenv)->NewStringUTF(lenv, initDataID);

    log_debug("method=[%s]", method);
    log_debug("param=[%s]", param);
    log_debug("initDataID=[%s]", initDataID);
    log_debug("handler=[%p]", jniPIP.handlePIPinitDataID);

    jstring retStr=(jstring)(*lenv)->CallObjectMethod(lenv, jniPIP.instance, jniPIP.handlePIPinitDataID, jniParam, jniDataID);

    char *strval=(char*)(*lenv)->GetStringUTFChars(lenv, retStr, 0);
    log_debug("PIP returned=[%s]", strval);

    (*lenv)->PopLocalFrame(lenv, NULL);
    return strval;
  }

#endif



// PDP OUT interface to PIP using TCP socket
// > 1 : communication via TCP socket
#if PDP_PIPSOCKET > 1
  unsigned int representationRefinesData(unsigned char *rep, unsigned char *dataID)
  {
    char *msg=calloc(1024,sizeof(char));
    snprintf(msg, 1024, "representationRefinesData,-1,%s,%s,true\r\n",rep,dataID);
    unsigned char *response=pefsocketTCP_clientSend(pipSocket, msg);
    log_error("received response for representationRefinesData=[%s]",response);
    return atoi(response);
  }

  // data_flow_monitor.c: given a container and a quantity of data in it, generates a new mapping
  char *initialRepresentation(char *cont, char *qod)
  {
    char *msg=calloc(1024,sizeof(char));
    snprintf(msg, 1024, "initialRepresentation,-1,%s\r\n",cont);
    log_error("sending to PIP: [%s]", msg);
    unsigned char *response=pefsocketTCP_clientSend(pipSocket, msg);
    log_error("received response for initialRepresentation=[%s]",response);
    log_info(" * %s -----> %s", cont, response);
    return response;
  }

  int   pipEval(char *method, char *params)   {return FALSE;}
  char* pipInit(char *method, char *params)
  {
    char *retVal=malloc(8*sizeof(char));
    snprintf(retVal, 8, "NOVALUE");
    return retVal;
  }
#endif

