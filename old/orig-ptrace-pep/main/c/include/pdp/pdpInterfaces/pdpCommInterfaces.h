/**
 * @file  pdpCommInterfaces.h
 * @brief Header for PDP communication interfaces
 *
 * Provide information about used communication interfaces of the PDP. Depending on the
 * PDP configuration a JNI interface, TCP socket interface and/or an XML-RPC interface is provided.
 *
 * @author cornelius moucha
 **/

#pragma once

#include "base.h"
#include "pdpConfig.h"
#include "memUtils.h"

#include "pdpInterface.h"
#include "pdpOutPEP.h"
#include "pdpOutPXP.h"

#ifdef  __cplusplus
  extern "C" {
#endif

#include "jni.h"

#define PDP_DEPLOYPOLICY    48
#define PDP_REVOKEMECHANISM 49
#define PDP_NOTIFYEVENT     50
#define PDP_LISTMECHANISMS  51
#define PDP_START           52
#define PDP_STOP            53
#define PIP_INITIALREP      54
#define PIP_REPREFINEDATA   55

#define PDP_INTERFACE_NATIVE 0
#define PDP_INTERFACE_JNI    1
#define PDP_INTERFACE_SOCKET 2
#define PDP_INTERFACE_XMLRPC 3


/**
 * Creates a new interface structure
 *
 * @param   name    name of the communication partner, i.e. PEP or PXP
 * @param   type    interface type: PDP_INTERFACE_NATIVE, PDP_INTERFACE_JNI, ...
 * @return  reference to created interface structure on success or NULL otherwise
 */
pdpInterface_ptr pdpInterfaceNew(const char *name, unsigned int type);

/**
 * Deallocates an interface structure\n
 * @param   linterface   interface structure for deallocation
 * @return  R_SUCCESS on success or R_ERROR otherwise
 */
unsigned int     pdpInterfaceFree(pdpInterface_ptr linterface);

/**
 * Initialize the static interfaces of the PDP, i.e. incoming interfaces
 * @return  R_SUCCESS on success or R_ERROR otherwise
&*/
unsigned int     pdpInterfacesInit();

/**
 * Stops the static interface of the PDP, i.e. incoming interfaces
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int     pdpInterfacesStop();

// PDP input interface via XML-RPC
#if PDP_XMLRPC == 1
  #ifdef __WIN32__
    #error "libsoup not supported in WIN32! XML-RPC interface not available."
  #endif
  #include "soupUtils.h"
  #include "xmlUtils.h"
  void pdpXMLrpc_handler(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data);
#endif

// PDP input interface via TCP socket
#if PDP_TCPSOCKET > 0
  #include "socketUtils.h"
  #include "xmlUtils.h"
  char *pdpSocketHandler(char *msg);
#endif

// PDP input interface via JNI
#ifdef PDP_JNI
  // JNI connection header
  #include "pdpInJNI.h"
#endif

// PDP input interface via RMI
#if PDP_RMI > 0
  #include "javaUtils.h"
#endif


// PDP OUT interface to PIP
// > 1 : communication via TCP socket
// ==1 : communication via JNI-interface
// ==0 : communication via internal method (internal PIP; only for OpenBSD)
// < 0 : no PIP -> stubs returning standard value
#if PDP_PIPSOCKET == 0
  // information flow stuff
  #include "data_flow_monitor.h"
#endif

// ==1 : communication via JNI-interface
#if PDP_PIPSOCKET == 1
  #ifdef PDP_JNI
    // JNI connection header
    #include "pdpInJNI.h"
  #endif
#endif

// > 1 : communication via TCP socket
#if PDP_PIPSOCKET > 1
  #include "pefsocket.h"
#endif


#ifdef  __cplusplus
  }
#endif


