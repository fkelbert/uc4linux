/**
 * @file  pdpOutPXP.h
 * @brief Header for PDP communication for PXP execution
 *
 * Provide information about used communication interfaces of the PDP. Depending on the
 * PDP configuration a native, a JNI interface, TCP socket interface and/or an XML-RPC interface is provided
 * for invoking an executor of an action.
 *
 * @author cornelius moucha
 **/

#ifndef PDPOUTPXP_H_
#define PDPOUTPXP_H_

#ifdef PDP_JNI
  #include <jni.h>
#endif

#include "base.h"
#include "pdpJNItypes.h"
#include "pdpInterface.h"
#include "paramInstanceType.h"

/**
 * PDP out interface: PXP invocation via JNI interface
 * @param   linterface   reference to the used interface
 * @param   name         action name for execution
 * @param   cntParams    amount of parameters in array
 * @param   params       array of parameters for this action execution
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pxpExecuteJNI(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params);

/**
 * PDP out interface: PXP invocation via socket
 * @param   linterface   reference to the used interface
 * @param   name         action name for execution
 * @param   cntParams    amount of parameters in array
 * @param   params       array of parameters for this action execution
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pxpExecuteSocket(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params);

/**
 * PDP out interface: PXP invocation via XML-RPC
 * @param   linterface   reference to the used interface
 * @param   name         action name for execution
 * @param   cntParams    amount of parameters in array
 * @param   params       array of parameters for this action execution
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pxpExecuteXMLrpc(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params);

/**
 * PDP out interface: PXP invocation via native method
 * @remark  should be only method signature and defined in PEP linking to the PDP library
 * @param   linterface   reference to the used interface
 * @param   name         action name for execution
 * @param   cntParams    amount of parameters in array
 * @param   params       array of parameters for this action execution
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pxpExecuteNative(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params);

/**
 * PDP out interface: Stub method for uninitialized interfaces
 * @param   linterface   reference to the used interface
 * @param   name         action name for execution
 * @param   cntParams    amount of parameters in array
 * @param   params       array of parameters for this action execution
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int pxpExecuteStub(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params);

#endif /* PDPOUTPXP_H_ */





