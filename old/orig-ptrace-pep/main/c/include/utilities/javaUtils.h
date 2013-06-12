/**
 * @file  javaUtils.h
 * @brief Auxiliary methods definitions for Java/JNI
 *
 * @author cornelius moucha
 **/

#ifndef JAVAUTILS_H_
#define JAVAUTILS_H_

#include "base.h"
#include "jni.h"


unsigned int initJVM(char *classpath);
unsigned int killJVM();
unsigned int stopJVM();
JNIEnv*      getJNIenvironment();



#endif /* JAVAUTILS_H_ */
