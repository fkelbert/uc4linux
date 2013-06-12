/**
 * @file  pdp.h
 * @brief Header for PDP implementation
 *
 * Main header file of PDP implementation; specifies the external available methods of the PDP
 * The PDP is the decision component in the policy enforcement framework (PEF). Hence, it provides
 * several ways for communication with other components, e.g. PEPs for notification of intercepted events
 * as well as management purposes such as starting, stopping the PDP, listing all installed mechanisms or
 * deployment and revocation of policies.\n
 * In sum, the following methods are available:
 * -# PDP Management
 *  - pdpStart()
 *  - pdpStop()
 *  - pdpListDeployedMechanisms()
 * -# Policy Management
 *  - pdpDeployMechanism(const char*, const char*)
 *  - pdpDeployPolicy(const char*)
 *  - pdpRevokeMechanism(const char*)
 * -# Event Notifications
 *  - pdpNotifyEvent(event_ptr)
 *  - pdpNotifyEventXML(const char*)
 *
 * \n
 * For the introduced methods, several way of accessing are possible. In case of direct linking the PDP shared library
 * to the enforcement or management component and accessing the methods with native function calls, all of these methods
 * are also encapsulated in different communication interfaces. Currently the following communication interfaces are
 * available.
 * - Native function call
 * - JNI (Java native interface)
 *  -# defined in pdpjni.c, pdpjni.h
 *  -# all introduced methods can be call from the class namespace: de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint
 *  -# additional methods for JNI:
 *   -# Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_listDeployedMechanismsJNI(JNIEnv*, jobject)\n
 *      (returning java.util.ArrayList)
 *   -# Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpNotifyEventJNI(JNIEnv*, jobject, jobject)\n
 *      (returning object of class Decision)
 * - TCP socket
 *  -# defined in pdpSocket.c
 *  -# communication using XML-based interfaces, e.g. pdpNotifyEventXML
 *  -# not fully implemented and not adapted to new structure
 *  -# not tested!
 * - XML-RPC communication
 *  -# defined in pdpxmlrpc.c
 *  -# not fully implemented and not adapted to new structure
 *  -# not tested!
 *
 *
 *
 * @author cornelius moucha
 **/

#pragma once

#include "base.h"
#include "pdpInternal.h"
#include "pdpCommInterfaces.h"

#ifdef  __cplusplus
  extern "C" {
#endif

// exported PDP methods

/**
 * Start PDP\n
 * Initialization of internal PDP structures such as hashtable for mechanisms, action descriptions
 * and interfaces for communication with the PDP by other components, e.g. PEP.\n
 * Internally sets the function pointers for the exported methods.
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT bool               pdpStart();

/**
 * Stop PDP\n
 * Deallocation of internal PDP structures such as hashtable for mechanisms, synchronization mutex,
 * and communication interfaces. Deallocates the action description store and set the
 * internal function pointers for exported methods to stubs to avoid accessing structures, which
 * are not initialized any more.
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT bool               pdpStop();

/**
 * Revoke Mechanism\n
 * Revoking of an installed mechanism deallocates the internal state machine for this mechanism and
 * unsubscribes irrelevant actions from the responsible PEP.
 *
 * @param   mechName    the mechanism name for revoking
 * @param   ns          the namespace of the mechanism for revoking
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpRevokeMechanism(const char *mechName, const char *ns);

/**
 * Revoke Policy\n
 * Revoking of an installed policy deallocates the internal state machine for all contained mechanisms in this policy
 * and unsubscribes irrelevant actions from the responsible PEP.
 *
 * @param   policyDocPath  the filename of the policy for revoking
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpRevokePolicy(const char *policyDocPath);

/**
 * Deploy Mechanism\n
 * Deploy a new mechanism given a local filename of a policy and a mechanism name. The file is read and its
 * XML structure is validated against the PEF policy scheme. Afterwards the given mechanism name is searched
 * in the policySet and for this mechanism an internal structure including the evaluation state machine is
 * instantiated.
 *
 * @param   policyDocPath    filename to the local file of the policy
 * @param   mechanismName    name of mechanism in the policy for deploying
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpDeployMechanism(const char *policyDocPath, const char *mechanismName);

/**
 * Deploy Mechanism\n
 * Deploy a new policy given a local filename of a policy. The file is read and its
 * XML structure is validated against the PEF policy scheme. Afterwards all contained mechanisms in the
 * policySet are instantiated in the PDP.
 *
 * @param   policyDocPath    filename to the local file of the policy
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpDeployPolicy(const char *policyDocPath);

/**
 * Deploy PolicyString\n
 * Deploy a new policy given a string of a policy. The policy is completely contained in the given
 * String, which is  read and its XML structure is validated against the PEF policy scheme.
 * Afterwards all contained mechanisms in the policySet are instantiated in the PDP.
 *
 * @param   policyString    policy as null-terminated string
 * @param   ns              namespace of policy; required as for file-based deploying the filename is used
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpDeployPolicyString(const char *policyString, const char *ns);

/**
 * Deploy MechanismString\n
 * Deploy a new mechanism given a string of a policy. The policy is completely contained in the given
 * String, which is  read and its XML structure is validated against the PEF policy scheme.
 * Afterwards the given mechanism name is searched in the policySet and for this mechanism an
 * internal structure including the evaluation state machine is instantiated.
 *
 * @param   policyString     policy as null-terminated string
 * @param   ns               namespace of policy; required as for file-based deploying the filename is used
 * @param   mechanismName    name of mechanism in the policy for deploying
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpDeployMechanismString(const char *policyString, const char *ns, const char *mechanismName);

/**
 * List deployed mechanisms\n
 * The internal hashtable for installed mechanisms is read and the names of all deployed mechanisms are returned in the
 * format mechanismName#$#mechanismName#$#...
 *
 * @return  the list of mechanisms deployed in the PDP
**/
LIBEXPORT char              *pdpListDeployedMechanisms();

/**
 * Notify Event - XML based\n
 * This method is called, when the PEP wants to notify the PDP about an intercepted event using XML representations.
 * For a direct communication between PEP and PDP using native function calls (e.g. in case of direct linking between
 * PDP and PEP), this method should NOT be called! Instead use the method pdpNotifyEvent(event_ptr levent)!
 * This method is internally called from the different PDP interfaces, e.g. socket interface, upon receiving an event,
 * because they cannot provide the required event_s structure, but have to use the XML representation. The return value
 * therefore is also an XML representation of the PDP's decision, i.e. the authorization action and potential one ore more
 * exeucte actions.\n
 * Internally reads the XML document in memory, parse it and instantiates the internal event_s structure.
 * Finally the method pdpNotifyEvent(event_ptr levent) is invoked and the response is serialized using
 * notifyResponseSerialize(notifyResponse_ptr) back to XML.
 *
 * @see pdpNotifyEvent(event_ptr)
 *
 * @param   eventDoc     XML representation of the intercepted event as null-terminated string
 * @return  the prepared notifyResponse_s structure based on the evaluation of the given event serialized to XML representation
**/
LIBEXPORT char              *pdpNotifyEventXML(const char *eventDoc);

/**
 * Notify Event\n
 * This method is called, when the PEP wants to notify the PDP about an intercepted event.
 * Internally it iterates over all subscribed mechanisms for the intercepted action. This
 * subscription is prepared internally when the mechanism was deployed to the PDP. For each
 * mechanism, the event is matched against the specified trigger event in the policy and
 * for a positive matching the condition is evaluated. If the condition is satisfied, the
 * mechanism relevant parts are appended to the notifyResponse structure.
 *
 * @param   levent     intercepted event
 * @return  the prepared notifyResponse_s structure based on the evaluation of the given event
**/
LIBEXPORT notifyResponse_ptr pdpNotifyEvent(event_ptr levent);

/**
 * PEP registration\n
 * This method is called, when a PEP wants to register itself at the PDP. This registration is
 * necessary for a later subscription to that PEP for actions in a policy. A PEP can either manually
 * register or the PEP interface can be specified in the policy.
 *
 * @param   name        unique name of the PEP
 * @param   linterface  communication interface for that PEP
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpRegisterPEP(const char *name, pdpInterface_ptr linterface);

/**
 * PEP action registration\n
 * This method is called, when a PEP wants to register an action at the PDP. This registration is
 * necessary for a later subscription to that PEP for that particular action in a policy. A PEP can
 * either manually register an action or the action registration can be specified in the policy.
 *
 * @param   name    action name
 * @param   pepID   unique name of the PEP as used in pdpRegisterPEP
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpRegisterAction(const char *name, const char *pepID);

/**
 * PXP registration\n
 * This method is called, when a PXP wants to register itself at the PDP. This registration is
 * necessary for later executions of specified actions in the policy. A PXP can either manually
 * register or the PXP interface can be specified in the policy.
 *
 * @param   name        unique name of the PXP
 * @param   linterface  communication interface for that PXP
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpRegisterPXP(const char *name, pdpInterface_ptr linterface);

/**
 * PXP executor registration\n
 * This method is called, when a PXP wants to register an execution action at the PDP. This registration is
 * necessary for later executions of this particular action as specified in the policy. A PEP can
 * either manually register an executor or the registration can be specified in the policy.
 *
 * @param   name    execution action name
 * @param   pxpID   unique name of the PXP as used in pdpRegisterPXP
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
LIBEXPORT unsigned int       pdpRegisterExecutor(const char *name, const char *pxpID);

// method stubs used in case of uninitialized or stopped PDP
bool               stubStart();
bool               stubStop();
unsigned int       stubRevokeMechanism(const char *mechName, const char *ns);
unsigned int       stubRevokePolicy(const char *policyDocPath);
unsigned int       stubDeployMechanism(const char *policyDocPath, const char *mechanismName);
unsigned int       stubDeployPolicy(const char *policyDocPath);
char*              stubListDeployedMechanisms();
char*              stubNotifyEventXML(const char *eventDoc);
notifyResponse_ptr stubNotifyEvent(event_ptr levent);


#ifdef  __cplusplus
  }
#endif






