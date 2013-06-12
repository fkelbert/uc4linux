/**
 * @file  pdpJNItypes.h
 * @brief Type definition header for the JNI interface
 *
 * Represents the JNI classes for communication between Java and C.
 *
 * @author cornelius moucha
**/

#ifndef PDPJNISTRUCT_H_
#define PDPJNISTRUCT_H_

#ifdef PDP_JNI
  #include "jni.h"

/**
 * @struct jniPDP_s
 * @brief Representation of the Java class de.fraunhofer.iese.pef.pdp.PolicyDecisionPoint
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniPDP_s
{
  jclass    cls;
  jobject   instance;
  jmethodID getInstance;
  jmethodID handlePEPsubscribe;
} jniPDP;

/**
 * @struct jniDecision_s
 * @brief Representation of the Java class de.fraunhofer.iese.pef.pdp.internal.Decision
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniDecision_s
{
  jclass    cls;
  jmethodID constructor;
  jmethodID addExecuteAction;
  jmethodID setAuthAction;
  jobject   allow;
  jobject   inhibit;
} jniDecision;

/**
 * @struct jniExecuteAction_s
 * @brief Representation of the Java class de.fraunhofer.iese.pef.pdp.internal.ExecuteAction
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniExecuteAction_s
{
  jclass    cls;
  jmethodID constructor;
  jmethodID addParameter;
  jmethodID setName;
} jniExecuteAction;

/**
 * @struct jniAuthAction_s
 * @brief Representation of the Java class de.fraunhofer.iese.pef.pdp.AuthorizationAction
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniAuthAction_s
{
  jclass    cls;
  jmethodID constructor;
  jmethodID setType;
  jmethodID setName;
  jmethodID setDelay;
  jmethodID addModifier;
} jniAuthAction;

/**
 * @struct jniArrayList_s
 * @brief Representation of the Java class java.util.ArrayList
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniArrayList_s
{
  jclass    cls;
  jmethodID constructor;
  jmethodID size;
  jmethodID get;
  jmethodID add;
} jniArrayList;

/**
 * @struct jniInteger_s
 * @brief Representation of the Java class java.lang.Integer
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniInteger_s
{
  jclass    cls;
  jmethodID intValue;
  jmethodID longValue;
} jniInteger;

/**
 * @struct jniBoolean_s
 * @brief Representation of the Java class java.lang.Boolean
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniBoolean_s
{
  jclass    cls;
  jmethodID booleanValue;
} jniBoolean;

/**
 * @struct jniByteArray_s
 * @brief Representation of the Java class for byte arrays (byte[])
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniByteArray_s
{
  jclass    cls;
} jniByteArray;

/**
 * @struct jniParam_s
 * @brief Representation of the Java class de.fraunhofer.iese.pef.pdp.internal.Param
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniParam_s
{
  jclass    cls;
  jmethodID constructor;
  jmethodID getName;
  jmethodID getType;
  jmethodID getValue;
} jniParam;

/**
 * @struct jniEvent_s
 * @brief Representation of the Java class de.fraunhofer.iese.pef.pdp.internal.Event
 *
 * Stores the JNI class definition and required methodIDs for achieving higher performance with respect to the communication
 * between Java and C using JNI.
**/
struct jniEvent_s
{
  jclass    cls;
  jmethodID getEventName;
  jmethodID isTryEvent;
  jmethodID getTimestamp;
  jmethodID getParams;
} jniEvent;

struct jniPIP_s
{
  jclass    cls;
  jobject   instance;
  jmethodID getInstance;
  jmethodID handlePIPinit;
  jmethodID handlePIPinitDataID;
  jmethodID handlePIPeval;
} jniPIP;

#endif
#endif /* PDPJNISTRUCT_H_ */







