/**
 * @file  pdpInJNI.h
 * @brief Header for PDP communication interface using JNI
 *
 * Description of the communication interface using the Java Native Interface (JNI). These
 * methods are called directly from Java applications, they are not intended to be called from
 * native C code. The appropriate Java class should have the class name PolicyDecisionPoint in the
 * package de.fraunhofer.iese.pef.pdp. Otherwise Java will complain about linking problems with the
 * native shared library of the PDP.
 *
 * @author cornelius moucha
 **/

#include <jni.h>
/* Header for class de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint */
#include "pdp.h"
#include "pdpJNItypes.h"


#ifndef _Included_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint
#define _Included_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint

#ifdef __cplusplus
extern "C" {
#endif


/**
 * JNI initialization method invoked with binding the native library to the JVM\n
 * This method prepares internal structures for all communication issues between PDP and the Java application, i.e.
 * prepares static representatives of Java class, their methods for achieving higher performance.
 * @param   jvm       JNI representation of the running Java virtual machine
 * @param   reserved  JNI internal parameter
 * @return  the version of the JNI interface on success\n or JNI_ERR on error
**/
JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM *jvm, void *reserved);

/**
 * JNI wrapper method for pdpStart()
 * @param   env   JNI environment
 * @param   jobj  JNI object representing the calling class
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpStart
  (JNIEnv *env, jobject jobj);

/**
 * JNI wrapper method for pdpStop()
 * @param   env   JNI environment
 * @param   jobj  JNI object representing the calling class
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpStop
  (JNIEnv *env, jobject jobj);

/**
 * JNI wrapper method for pdpNotifyEventXML()
 * @param   env        JNI environment
 * @param   jobj       JNI object representing the calling class
 * @param   jeventDoc  XMl representation of intercepted event (JNI string)
 * @return  the prepared notifyResponse_s structure based on the evaluation of the given event serialized to XML representation (JNI string)
**/
JNIEXPORT jstring JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpNotifyEventXML
  (JNIEnv *env, jobject jobj, jstring jeventDoc);

/**
 * JNI wrapper method for pdpNotifyEvent()
 * Receives a Java object of class de.fraunhofer.iese.pef.pdp.internal.Event, which is mapped to the internal event structure.
 * Afterwards the internal method pdpNotifyEvent() is called and the response is mapped back to a Java object of class
 * de.fraunhofer.iese.pef.pdp.internal.Decision.
 *
 * @param   env          JNI environment
 * @param   jobj         JNI object representing the calling class
 * @param   jeventObject Java object of class de.fraunhofer.iese.pef.pdp.internal.Event representing the intercepted event
 * @return  the prepared response as Java object of class de.fraunhofer.iese.pef.pdp.internal.Decision
**/
JNIEXPORT jobject JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpNotifyEventJNI
  (JNIEnv *env, jobject jobj, jobject jeventObject);

/**
 * JNI wrapper method for pdpDeployPolicy()
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @param   jPolicyDocPath     XMl representation of policy for deploying (JNI string)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpDeployPolicy
  (JNIEnv *env, jobject jobj, jstring jPolicyDocPath);

/**
 * JNI wrapper method for pdpDeployPolicy()
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @param   jPolicyDocPath     XMl representation of policy for deploying (JNI string)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpDeployPolicyString
  (JNIEnv *env, jobject jobj, jstring jPolicyString);

/**
 * JNI wrapper method for pdpDeployMechanism()
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @param   jPolicyDocPath     XMl representation of policy for deploying (JNI string)
 * @param   jMechName          name of mechanism in the policy for deploying (JNI string)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpDeployMechanism
  (JNIEnv *env, jobject jobj, jstring jPolicyDocPath, jstring jMechName);

/**
 * JNI wrapper method for pdpRevokeMechanism()
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @param   jMechanismName     the mechanism name for revoking (JNI string)
 * @param   jns                the namespace of mechanism name for revoking (JNI string)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_pdpRevokeMechanism
  (JNIEnv *env, jobject jobj, jstring jMechanismName, jstring jns);

/**
 * JNI wrapper method for pdpListDeployedMechanisms()
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @return  the list of mechanisms deployed in the PDP (separater: #$#)
**/
JNIEXPORT jstring JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_listDeployedMechanisms
  (JNIEnv *env, jobject jobj);

/**
 * JNI wrapper method for pdpListDeployedMechanisms()
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @return  the list of mechanisms deployed in the PDP as object of class java.util.ArrayList<String>
**/
JNIEXPORT jobject JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_listDeployedMechanismsJNI
  (JNIEnv *env, jobject jobj);

/**
 * JNI wrapper method for pefLogSetRTLevel(unsigned int)
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @param   newLevel           new logging level
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_setRuntimeLogLevel
  (JNIEnv *env, jobject jobj, jint newLevel);

/**
 * JNI wrapper method for pdpRegisterPEP(const char *name, pdpInterface_ptr linterface)
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @param   jPEPname           PEP name for registering (JNI string)
 * @param   jClassName         Java class name for JNI interface (JNI string)
 * @param   jMethodName        Java method name for JNI interface (JNI string)
 * @param   jMethodSignature   Java method name for JNI interface (JNI string)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_registerPEP
  (JNIEnv *env, jobject jobj, jstring jPEPname, jstring jClassName, jstring jMethodName, jstring jMethodSignature);


/**
 * JNI wrapper method for pdpRegisterAction(const char *actionName, const char *pepName)
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @param   jActionName        Action name for registering (JNI string)
 * @param   jPEPname           PEP name for registering (JNI string)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_registerAction
  (JNIEnv *env, jobject jobj, jstring jActionName, jstring jPEPname);

/**
 * JNI wrapper method for pdpRegisterPXP(const char *name, pdpInterface_ptr linterface)
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @param   jPXPname           PEP name for registering (JNI string)
 * @param   jClassName         Java class name for JNI interface (JNI string)
 * @param   jMethodName        Java method name for JNI interface (JNI string)
 * @param   jMethodSignature   Java method name for JNI interface (JNI string)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_registerPXP
  (JNIEnv *env, jobject jobj, jstring jPXPname, jstring jClassName, jstring jMethodName, jstring jMethodSignature);


/**
 * JNI wrapper method for pdpRegisterExecutor(const char *actionName, const char *pepName)
 * @param   env                JNI environment
 * @param   jobj               JNI object representing the calling class
 * @param   jActionName        Action name for registering (JNI string)
 * @param   jPXPname           PEP name for registering (JNI string)
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
JNIEXPORT jint JNICALL Java_de_fraunhofer_iese_pef_pdp_PolicyDecisionPoint_registerExecutor
  (JNIEnv *env, jobject jobj, jstring jActionName, jstring jPXPname);


#ifdef __cplusplus
}
#endif
#endif
