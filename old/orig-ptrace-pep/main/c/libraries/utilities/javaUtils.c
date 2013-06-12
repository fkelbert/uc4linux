/**
 * @file  javaUtils.c
 * @brief Auxiliary methods for Java/JNI
 *
 * @author cornelius moucha
 **/

#include "javaUtils.h"
#include "log_javaUtils_pef.h"

// Global persistent reference to Java virtual machine
JavaVM *curjvm=NULL;
unsigned int initRMIjvm=FALSE;

#ifdef __WIN32__
  /// temporary type definition for win32
  typedef jint (CALLBACK *fpCJV)(JavaVM**, void**, JavaVMInitArgs*);

  unsigned int initJVM(char *classpath)
  {
    if(curjvm!=NULL)
    {
      log_warn("JVM is already instantiated?! PDP is executing within JNI environment => Skipping initiating new virtual machine.");
      return R_SUCCESS;
    }

    log_info("Starting JVM");
    JNIEnv *env;

    fpCJV CreateJavaVM;
    //HINSTANCE hVM=LoadLibrary("E:\\Programme\\Java\\jre6\\bin\\client\\jvm.dll");
    // jvm.dll needs to be in environment PATH
    HINSTANCE hVM=LoadLibrary("jvm.dll");
    if(hVM==NULL)
    {
      log_warn("Could not load jvm.dll! Please ensure it is in the environment PATH!");
      //DWORD dwe=GetLastError();
      return R_ERROR;
    }

    // Retrieve Method pointer
    CreateJavaVM=(fpCJV)GetProcAddress(hVM, "JNI_CreateJavaVM");

    char *jvmArgClassPath=memCalloc(strlen(classpath)+19, sizeof(char));
    snprintf(jvmArgClassPath, strlen(classpath)+19, "-Djava.class.path=%s", classpath);
    log_trace("classPathArg for JVM: [%s]", jvmArgClassPath);

    JavaVMInitArgs jvmargs;
    JavaVMOption options;
    //options.optionString="-Djava.class.path=../../../bin/java/rmiServer.jar";
    options.optionString=jvmArgClassPath;
    jvmargs.version  = JNI_VERSION_1_6;
    jvmargs.nOptions = 1;
    jvmargs.options  = &options;
    jvmargs.ignoreUnrecognized = 0;

    int ret=CreateJavaVM(&curjvm, (void**)&env, &jvmargs);
    if(ret<0)
    {
      log_fatal("Unable to instantiate JVM (%d)\n", ret);
      return R_ERROR;
    }
    log_info("JVM started...");
    initRMIjvm=TRUE;

    return R_SUCCESS;
  }

  unsigned int killJVM()
  {
    int ret=(*curjvm)->DestroyJavaVM(curjvm);
    log_info("Destroying JVM: [%d]", ret);
    return ret;
  }

  unsigned int stopJVM()
  {
    if(initRMIjvm==TRUE) return killJVM();
    else return R_SUCCESS;
  }

#else
  #ifdef PEF_LOGANDROID
    unsigned int initJVM(char *classpath)
    {
      if(curjvm!=NULL)
      {
        log_warn("JVM is already instantiated?! PDP is executing within JNI environment => Skipping initiating new virtual machine.");
        return R_SUCCESS;
      }
      log_error("ERROR: stopping JVM in android not supported!!!");
      return R_ERROR;
    }

    /// @todo Calling destroyVM sometimes freezes testpdp application?!
    unsigned int killJVM()
    {
      log_error("ERROR: stopping JVM in android not supported!!!");
      return R_ERROR;
    }

    unsigned int stopJVM()
    {
      log_error("ERROR: stopping JVM in android not supported!!!");
      return R_ERROR;
    }
  #else
     unsigned int initJVM(char *classpath)
     {
       if(curjvm!=NULL)
       {
         log_warn("JVM is already instantiated?! PDP is executing within JNI environment => Skipping initiating new virtual machine.");
         return R_SUCCESS;
       }

       log_info("Starting JVM");
       JNIEnv *env;

       char *jvmArgClassPath=memCalloc(strlen(classpath)+49, sizeof(char));
       snprintf(jvmArgClassPath, strlen(classpath)+19, "-Djava.class.path=%s", classpath);
       log_trace("classPathArg for JVM: [%s]", jvmArgClassPath);

       //char *jvmOption2=memCalloc(29, sizeof(char));
       //snprintf(jvmOption2, 29, "-XX:+AllowUserSignalHandlers");
       //log_trace("Option2: [%s]", jvmOption2);

       char *jvmOption3=memCalloc(29, sizeof(char));
       snprintf(jvmOption3, 29, "-Xrs");  // reduced signal usage by JVM (no SIGINT, SIGTERM, ...); only critical signal handlers such as SIGSEGV
       log_trace("Option3: [%s]", jvmOption3);

       JavaVMInitArgs jvmargs;
       JavaVMOption options[3];
       options[0].optionString=jvmArgClassPath;
       options[1].optionString=jvmOption3;
       //options[1].optionString=jvmOption2;
       jvmargs.version  = JNI_VERSION_1_6;
       jvmargs.nOptions = 2;
       jvmargs.options  = options;
       jvmargs.ignoreUnrecognized = 0;

       int ret=JNI_CreateJavaVM(&curjvm, (void**)&env, &jvmargs);
       if(ret<0)
       {
         log_fatal("Unable to instantiate JVM (%d)\n", ret);
         return R_ERROR;
       }
       log_info("JVM started...");
       initRMIjvm=TRUE;

       return R_SUCCESS;
     }

     /// @todo Calling destroyVM sometimes freezes testpdp application?!
     unsigned int killJVM()
     {
       log_trace("Invoking destroyJVM...");
       int ret=-1;
       //ret=(*curjvm)->DestroyJavaVM(curjvm);
       log_info("Destroying JVM: [%d]", ret);
       return ret;
     }

     unsigned int stopJVM()
     {
       if(initRMIjvm==TRUE)
       {
         log_debug("JVM was initiated manually, stopping JVM...");
         JNIEnv *env=getJNIenvironment();
         if(env==NULL) {log_error("Error retrieving JNI environment, aborting..."); return R_ERROR;}

         jclass clsRMI=(*env)->FindClass(env, "de/fraunhofer/iese/pef/pdp/rmiServer/pdpRMIserver");
         if(clsRMI==NULL) {log_error("Could not find RMI-class."); return R_ERROR;}
         log_debug("pdpRMI class information retrieved");

         jmethodID rmiStopMathod=(*env)->GetStaticMethodID(env, clsRMI, "stopRMIserver", "()I");
         if(rmiStopMathod==NULL) {log_error("Could not find stopping method in pdpRMI-class."); return R_ERROR;}
         log_debug("RMI stop method retrieved");

         jint ret=(*env)->CallStaticIntMethod(env, clsRMI, rmiStopMathod);
         log_debug("Stopping RMI server returned [%d]", ret);
         if(ret==R_ERROR) {log_error("Error stopping RMI server."); return R_ERROR;}
         return killJVM();
       }
       else
       {
         log_debug("JVM was NOT initiated manually, NOT stopping...");
         return R_SUCCESS;
       }
     }
  #endif
#endif


JNIEnv* getJNIenvironment()
{
  JNIEnv *lenv=NULL;
  if((*curjvm)->GetEnv(curjvm, (void **)&lenv, JNI_VERSION_1_6))
  {
    log_error("ERROR retrieving JNI environment from JVM");
    return NULL;
  }
  return lenv;
}

