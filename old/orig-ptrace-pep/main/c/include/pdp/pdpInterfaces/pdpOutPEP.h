/**
 * @file  pdpOutPEP.h
 * @brief Header for outgoing PDP communication for PEP subscription
 *
 * Provide information about used communication interfaces of the PDP. Depending on the
 * PDP configuration a native, a JNI interface, TCP socket interface and/or an XML-RPC interface is provided
 * for subscription to a PEP.
 *
 * @author cornelius moucha
 **/

#ifndef PDPOUTPEP_H_
#define PDPOUTPEP_H_

#include "base.h"
#include "pdpInterface.h"
#include "pdpJNItypes.h"

/**
 * PDP out interface: PEP subscription via JNI interface
 * @param   linterface   reference to the used interface
 * @param   name         action name for subscribing
 * @param   unsubscribe  boolean flag indicating subscribe or unsubscribe of this action
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pepSubscribeJNI(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe);

/**
 * PDP out interface: PEP subscription via Socket
 * @param   linterface   reference to the used interface
 * @param   name         action name for subscribing
 * @param   unsubscribe  boolean flag indicating subscribe or unsubscribe of this action
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pepSubscribeSocket(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe);

/**
 * PDP out interface: PEP subscription via XML-RPC
 * @param   linterface   reference to the used interface
 * @param   name         action name for subscribing
 * @param   unsubscribe  boolean flag indicating subscribe or unsubscribe of this action
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pepSubscribeXMLrpc(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe);

/**
 * PDP out interface: PEP subscription via native method call
 * @remark  should be only method signature and defined in PEP linking to the PDP library
 * @param   linterface   reference to the used interface
 * @param   name         action name for subscribing
 * @param   unsubscribe  boolean flag indicating subscribe or unsubscribe of this action
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pepSubscribeNative(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe);

/**
 * PDP out interface: Stub method for uninitialized interfaces
 * @param   linterface   reference to the used interface
 * @param   name         action name for subscribing
 * @param   unsubscribe  boolean flag indicating subscribe or unsubscribe of this action
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pepSubscribeStub(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe);

#endif /* PDPOUTPEP_H_ */




