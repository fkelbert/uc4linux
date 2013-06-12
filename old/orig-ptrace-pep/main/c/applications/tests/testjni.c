/**
 * @file  testjni.c
 * @brief Exemplary execution test for PDP in combination with JNI
 *
 * @author cornelius moucha
**/

#include <jni.h>
#include "pdp.h"
#include "javaUtils.h"
#include "log_testjni_pef.h"

pdpInterface_ptr pdpInterfaceStub;
static char* const jpdpClassPathOption="-Djava.class.path=/home/moucha/workspace/pef/bin/java";

int main(int argc, char **argv)
{
  log_debug("Starting JNI-test application...");
  log_debug("Starting JVM");

  initJVM("/home/moucha/workspace/pef/bin/java");
  JNIEnv *env=getJNIenvironment();

  jclass clsPDPmain=(*env)->FindClass(env, "de/fraunhofer/iese/pef/pdp/example/pdpNativeExample");
  if(clsPDPmain==NULL) {log_error("could not find pdpMain-class."); return 1;}
  log_info("pdpMain class information retrieved");

  jmethodID pdpMainMathod=(*env)->GetStaticMethodID(env, clsPDPmain, "main", "([Ljava/lang/String;)V");
  if(pdpMainMathod==NULL) {log_error("Could not find main-method in pdpMain-class."); return 1;}
  log_info("PDP main method retrieved.");

  log_info("Calling main-method of jpdp...");
  (*env)->CallStaticVoidMethod(env, clsPDPmain, pdpMainMathod, pdpMainMathod, NULL);
  log_info("jpdp main-method finished...");

  getchar();
  // Destroy JVM
  //int n=(*jvm)->DestroyJavaVM(jvm);
  //log_info("Destroying JVM: [%d]", n);
  killJVM();
  return 0;
}
