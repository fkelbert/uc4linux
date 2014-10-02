#include "usagecontrol.h"

volatile JavaVM *jvm = NULL;

// Definitions to have all eclipse errors in one place rather than all over the C file...
#define JniFindClass(env,class) 							(*env)->FindClass(env, class)
#define JniGetStaticMethodID(env,class,name,sig)			(*env)->GetStaticMethodID(env, class, name, sig)
#define JniNewObjectArray(env,size,class,arg)	 			(*env)->NewObjectArray(env, size, class, arg)
#define JniNewStringUTF(env,str)							(*env)->NewStringUTF(env, str)
#define JniSetObjectArrayElement(env,arr,pos,elem)			(*env)->SetObjectArrayElement(env, arr, pos, elem)

#define JniCallStaticVoidMethod(env,class,method,...) 		(*env)->CallStaticVoidMethod(env, class, method, ##__VA_ARGS__)
#define JniCallStaticBooleanMethod(env,class,method,...)	(*env)->CallStaticBooleanMethod(env, class, method, ##__VA_ARGS__)
#define JniCallStaticObjectMethod(env,class,method,...)		(*env)->CallStaticObjectMethod(env, class, method, ##__VA_ARGS__)

#define JniCallBooleanMethod(env,object,method,...)			(*env)->CallBooleanMethod(env, object, method, ##__VA_ARGS__)
#define JniCallObjectMethod(env,object,method,...)		(	*env)->CallObjectMethod(env, object, method, ##__VA_ARGS__)

#define JniAttachCurrentThread(jvm,env,arg)					(*jvm)->AttachCurrentThread(jvm, env, arg)

#define JniGetMethodID(env,class,name,sig)					(*env)->GetMethodID(env, class, name, sig)
#define JniGetStaticFieldID(env,class,name,sig)					(*env)->GetStaticFieldID(env, class, name, sig)
#define JniGetStaticObjectField(env,class,field)					(*env)->GetStaticObjectField(env, class, field)

#define JniExceptionCheck(env)								(*env)->ExceptionCheck(env)
#define JniExceptionClear(env)								(*env)->ExceptionClear(env)
#define JniExceptionDescribe(env)							(*env)->ExceptionDescribe(env)

JNIEnv *mainJniEnv;

// CAUTION!
// The following references are only valid within the main thread!
jclass classNativeHandler;
jclass classNativeHandler;
jclass classString;
jclass classThrowable;
jmethodID methodNotifyEvent;
jmethodID methodIsStarted = NULL;
jobject nativeHandler = NULL;

pthread_t jvmStarter;

void *threadJvmStarter(void *args) {
	JNIEnv *env;
    JavaVMInitArgs vm_args;

    int nOptions = 4;
    JavaVMOption options[nOptions];
    options[0].optionString = "-Djava.class.path=" USER_CLASSPATH; //Path to the java source code
    options[1].optionString = "-Djava.compiler=NONE";
    options[2].optionString = "-Xms128m";
    options[3].optionString = "-Xmx1536m";

    vm_args.version = JNI_VERSION_1_6; //JDK version. This indicates version 1.6
    vm_args.nOptions = nOptions;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = 0;

    jmethodID mainMethod;
    jclass mainClass;

    // start the JVM
    if (JNI_CreateJavaVM((JavaVM**) &jvm, (void**)&env, &vm_args) < 0) {
    	pthread_exit((void *) false);
    }

    // find the Controller
    mainClass = JniFindClass(env, CLASS_NATIVE_HANDLER);
	if (JniExceptionCheck(env) == JNI_TRUE) {
		printf("Could not find main class " CLASS_NATIVE_HANDLER ".\n");
		exit(1);
	};

	// find the main method
	mainMethod = JniGetStaticMethodID(env, mainClass, METHOD_MAIN_NAME, METHOD_MAIN_SIG);
	if (JniExceptionCheck(env) == JNI_TRUE) {
		printf("Method " METHOD_MAIN_NAME " not found.\n");
		exit(1);
	}



	// call the main method
//	jarray arg = JniNewObjectArray(env, 2, JniFindClass(env, JNI_STRING), 0);
//	JniSetObjectArrayElement(env, arg, 0, JniNewStringUTF(env, "-pp"));
//	JniSetObjectArrayElement(env, arg, 1, JniNewStringUTF(env, "pdp1.properties"));
//	JniCallStaticVoidMethod(env, pdpClass, mainMethod, arg);
	JniCallStaticVoidMethod(env, mainClass, mainMethod);
	if (JniExceptionCheck(env) == JNI_TRUE) {
		printf("Exception in " METHOD_MAIN_NAME ".\n");
		JniExceptionDescribe(env);
		exit(1);
	}

	JniExceptionClear(env);


	// The above main() (i.e. this thread) should actually never exit
	pthread_exit((void *) true);
	return (void*) true;
}


// This thread will wait for the JVM thread to exit,
// which will hopefully never happen
void *threadJvmWaiter(void *args) {
	void *ret;
	pthread_join(jvmStarter, &ret);
	printf("JVM died. Exiting.\n");
	exit(1);
}


void notifyEventToPdp(event *ev) {
	jarray paramKeys = JniNewObjectArray(mainJniEnv, ev->cntParams, classString, 0);
	jarray paramVals = JniNewObjectArray(mainJniEnv, ev->cntParams, classString, 0);

	int i;
	for (i = 0; i < ev->cntParams; i++) {
		JniSetObjectArrayElement(mainJniEnv, paramKeys, i, ev->params[i]->key);
		JniSetObjectArrayElement(mainJniEnv, paramVals, i, ev->params[i]->val);
	}

	jobject resp = JniCallObjectMethod(mainJniEnv, nativeHandler, methodNotifyEvent, ev->name, paramKeys, paramVals, ev->isActual);
}


bool waitForStartupCompletion() {
	printf("Waiting for Controller to get started ...");

	JniExceptionClear(mainJniEnv);
	while (nativeHandler == NULL || methodIsStarted == NULL) {
		printf(".");
		fflush(stdout);
		sleep(1);
	}

	while(!JniCallBooleanMethod(mainJniEnv, nativeHandler, methodIsStarted)) {
/*		if (JniExceptionCheck(mainJniEnv) == JNI_TRUE) {
			JniExceptionDescribe(mainJniEnv);
			JniExceptionClear(mainJniEnv);
		}
*/
		printf(".");
		fflush(stdout);
		sleep(1);
	}
	printf("\n");

	return true;
}


bool getMainJniRefs() {
	classNativeHandler = JniFindClass(mainJniEnv, CLASS_NATIVE_HANDLER);
	if (JniExceptionCheck(mainJniEnv) == JNI_TRUE) {
		printf("Could not find class " CLASS_NATIVE_HANDLER ".\n");
		JniExceptionDescribe(mainJniEnv);
		JniExceptionClear(mainJniEnv);
		return false;
	}

	methodNotifyEvent = JniGetMethodID(mainJniEnv, classNativeHandler, METHOD_NOTIFY_NAME, METHOD_NOTIFY_SIG);
	if (JniExceptionCheck(mainJniEnv) == JNI_TRUE) {
		printf("Could not access method " METHOD_NOTIFY_NAME " in class " CLASS_NATIVE_HANDLER ".\n");
		JniExceptionDescribe(mainJniEnv);
		JniExceptionClear(mainJniEnv);
		return false;
	}

	jfieldID fieldNativeHandler = JniGetStaticFieldID(mainJniEnv, classNativeHandler, FIELD_NATIVE_HANDLER, FIELD_NATIVE_HANDLER_SIG);
	if (JniExceptionCheck(mainJniEnv) == JNI_TRUE) {
		printf("(1) Could not access field " FIELD_NATIVE_HANDLER " in class " CLASS_NATIVE_HANDLER ".\n");
		JniExceptionDescribe(mainJniEnv);
		JniExceptionClear(mainJniEnv);
		return false;
	}

	printf("Waiting for NativeHandler to be initialized...");
	while (nativeHandler == NULL) {
		nativeHandler = JniGetStaticObjectField(mainJniEnv, classNativeHandler, fieldNativeHandler);
		if (JniExceptionCheck(mainJniEnv) == JNI_TRUE) {
			printf("(2) Could not access field " FIELD_NATIVE_HANDLER " in class " CLASS_NATIVE_HANDLER ".\n");
			JniExceptionDescribe(mainJniEnv);
			JniExceptionClear(mainJniEnv);
			return false;
		}
		if (nativeHandler == NULL) {
			sleep(1);
		}
	}
	printf(" done.\n");

	methodIsStarted = JniGetMethodID(mainJniEnv, classNativeHandler, METHOD_ISSTARTED_NAME, METHOD_ISSTARTED_SIG);
	if (JniExceptionCheck(mainJniEnv) == JNI_TRUE) {
		printf("Could not find " METHOD_ISSTARTED_NAME " method.\n");
		JniExceptionDescribe(mainJniEnv);
		JniExceptionClear(mainJniEnv);
		return false;
	}

	classString = JniFindClass(mainJniEnv, CLASS_STRING);
	if (JniExceptionCheck(mainJniEnv) == JNI_TRUE) {
		printf("Could not find class " CLASS_STRING ".\n");
		JniExceptionDescribe(mainJniEnv);
		JniExceptionClear(mainJniEnv);
		return false;
	}

	classThrowable = JniFindClass(mainJniEnv, CLASS_THROWABLE);

	return true;
}

void ucEnd() {
	while(1);
}

bool ucInit() {
	initPep2PdpThriftClient(21003);

	if (connectPep2PdpThriftClient()) {
		printf("success\n");
	}
	else {
		printf("failure\n");
	}

	return false;

	pthread_t jvmWaiter;
	void *ret;

	if (pthread_create( &jvmStarter, NULL, threadJvmStarter, NULL) != 0) {
		printf("Unable to create JVM starter thread.\n");
	}

	printf("Wait for startup of JVM");
	while (jvm == NULL) {
		printf(".");
		usleep(30000);
	}
	printf("\n");



	// Attach this thread to JVM
	if (JniAttachCurrentThread((JavaVM*) jvm, (void**) &mainJniEnv, NULL) != 0) {
		printf("Error attaching main thread to JVM. Exiting.\n");
		exit(1);
	}

	ucTypesInit(mainJniEnv);

	if (!getMainJniRefs()) {
		printf("Error getting main JNI references. Exiting.\n");
		exit(1);
	}

	if (!waitForStartupCompletion()) {
		printf("Error while starting up JVM. Exiting.\n");
		exit(1);
	}

	// start new thread to wait for JVM to die (which should not happen)
	if (pthread_create( &jvmWaiter, NULL, threadJvmWaiter, NULL) != 0) {
		printf("Unable to create JVM waiter thread.\n");
	}

	return true;
}

bool ignoreExecve = true;

void notifySyscall(struct tcb *tcp) {
	if (!tcp || !tcp->s_ent || !tcp->s_ent->sys_name) {
		return;
	}

	/* It is kind of weird, that we need to use exiting()
	 * in this way here. My guess is, that execve (which is stopped
	 * three times in the beginning) makes this necessary.
	 * TODO: Handle this case more seriously;
	 * maybe use syscall_fixup_on_sysenter() in syscall.c
	 * This also needs to be fixed in ucPIP_main.c::ucPIP_update() then.
	 */
	bool actual = is_actual(tcp);

	uc_log("==== %3d (%05d, %c) %s ... ", tcp->scno, tcp->pid, actual ? 'A' : 'D', tcp->s_ent->sys_name);

	if (!ucSemanticsDefined(tcp->scno)) {
		uc_log("ignoring.\n");
		return;
	}

	event *ev = ucSemanticsFunct[tcp->scno](tcp);
	if (ev == NULL) {
		uc_log("returned NULL.\n");
		return;
	}

	/*
	 * Always signal these calls as actual event.
	 */
	switch (tcp->scno) {
		case SYS_execve:
			ignoreExecve = !ignoreExecve;
			if (ignoreExecve) {
				uc_log("ignoring.\n");
				return;
			}
			/* no break */
		case SYS_exit:
		case SYS_exit_group:
			ev->isActual = true;
			break;
		default:
			ev->isActual = actual;
	}

	uc_log("notifying PDP... ");
	notifyEventToPdp(ev);
	uc_log("done.");

	destroyEvent(ev);
	uc_log("\n");
}

