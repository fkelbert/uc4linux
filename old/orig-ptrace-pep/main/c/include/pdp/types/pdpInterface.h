/**
 * @file  pdpInterface.h
 * @brief Type definition header for PDP interfaces
 *
 * Provides type and structure definitions for PDP interfaces
 *
 * @author cornelius moucha
 **/

#ifndef PDPINTERFACE_H_
#define PDPINTERFACE_H_

#ifdef PDP_JNI
  #include <jni.h>
#endif

#include "paramInstanceType.h"
#include "socketUtils.h"

typedef struct pdpInterface_s  pdpInterface_t;
typedef        pdpInterface_t *pdpInterface_ptr;

extern pdpInterface_ptr pdpInterfaceStub;

/**
 * @struct pdpInterface_s
 * This structure describes an interface for communication with PEPs and PXPs.
 * It is instantiated upon parsing the interface description of a policy or when a PEP/PXP register itself at the PDP.
**/
struct pdpInterface_s
{
  /// Interface type: native, JNI, socket or xmlrpc
  unsigned int   type;
  /// Name of the PEP/PXP
  char          *name;
  /// Function pointer for PEP subscription
  unsigned int  (*pepSubscribe)(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe);
  /// Function pointer for PXP invocation
  unsigned int  (*pxpExecute)(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params);

  // JNI stuff
  #ifdef PDP_JNI
    /// JNI reference to the java class of the PEP
    jclass classPEP;

    jobject   instance;
    jmethodID getInstance;

    /// JNI reference to the java handling method for PEP subscriptions
    jmethodID pepSubscriptionMethod;

    /// JNI reference to the java class of the PXP
    jclass classPXP;
    /// JNI reference to the java handling method for PXP invocations
    jmethodID pxpExecutionMethod;
  #endif

  /// Reference to a socket if required
  pefSocket_ptr socket;
};

#endif /* PDPINTERFACE_H_ */



