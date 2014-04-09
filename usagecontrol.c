#include "usagecontrol.h"

volatile JavaVM *jvm = NULL;

// Definitions to have all eclipse errors in one place rather than all over the C file...
#define JniFindClass(env,class) 							(*env)->FindClass(env, class)
#define JniGetStaticMethodID(env,class,name,sig)			(*env)->GetStaticMethodID(env, class, name, sig)
#define JniNewObjectArray(env,size,class,arg)	 			(*env)->NewObjectArray(env, size, class, arg)
#define JniNewStringUTF(env,str)							(*env)->NewStringUTF(env, str)
#define JniSetObjectArrayElement(env,arr,pos,elem)			(*env)->SetObjectArrayElement(env, arr, pos, elem);
#define JniCallStaticVoidMethod(env,class,method,...) 		(*env)->CallStaticVoidMethod(env, class, method, ##__VA_ARGS__);
#define JniCallStaticBooleanMethod(env,class,method,...)	(*env)->CallStaticBooleanMethod(env, class, method, ##__VA_ARGS__)
#define JniCallStaticObjectMethod(env,class,method,...)		(*env)->CallStaticObjectMethod(env, class, method, ##__VA_ARGS__)
#define JniCallObjectMethod(env,object,method,...)		(*env)->CallObjectMethod(env, object, method, ##__VA_ARGS__)
#define JniAttachCurrentThread(jvm,env,arg)					(*jvm)->AttachCurrentThread(jvm, env, arg)

#define JniGetMethodID(env,class,name,sig)					(*env)->GetMethodID(env, class, name, sig)
#define JniGetStaticFieldID(env,class,name,sig)					(*env)->GetStaticFieldID(env, class, name, sig)
#define JniGetStaticObjectField(env,class,field)					(*env)->GetStaticObjectField(env, class, field)

#define JniExceptionOccurred(env)							(*env)->ExceptionOccurred(env)
#define JniExceptionCheck(env)								(*env)->ExceptionCheck(env)
#define JniExceptionClear(env)								(*env)->ExceptionClear(env)
#define JniExceptionDescribe(env)							(*env)->ExceptionDescribe(env)

JNIEnv *mainJniEnv;

// CAUTION!
// The following references are only valid within the main thread!
jclass classNativeHandler;
jclass classController;
jclass classString;
jclass classThrowable;
jmethodID methodNotifyEvent;
jobject nativeHandler;

pthread_t jvmStarter;

void *threadJvmStarter(void *args) {
	JNIEnv *env;
    JavaVMInitArgs vm_args;
    JavaVMOption options;
    options.optionString = "-Djava.class.path=" USER_CLASSPATH; //Path to the java source code
    vm_args.version = JNI_VERSION_1_6; //JDK version. This indicates version 1.6
    vm_args.nOptions = 1;
    vm_args.options = &options;
    vm_args.ignoreUnrecognized = 0;

    jmethodID mainMethod;
    jclass pdpClass;

    // start the JVM
    if (JNI_CreateJavaVM((JavaVM**) &jvm, (void**)&env, &vm_args) < 0) {
    	pthread_exit((void *) false);
    }

    // find the Controller
    pdpClass = JniFindClass(env, CLASS_CONTROLLER);
	if (JniExceptionOccurred(env)) {
		printf("Could not find class " CLASS_CONTROLLER ".\n");
		exit(1);
	};

	// find the main method
	mainMethod = JniGetStaticMethodID(env, pdpClass, METHOD_MAIN_NAME, METHOD_MAIN_SIG);
	if (JniExceptionOccurred(env)) {
		printf("Method " METHOD_MAIN_NAME " not found.\n");
		exit(1);
	}

	// call the main method
//	jarray arg = JniNewObjectArray(env, 2, JniFindClass(env, JNI_STRING), 0);
//	JniSetObjectArrayElement(env, arg, 0, JniNewStringUTF(env, "-pp"));
//	JniSetObjectArrayElement(env, arg, 1, JniNewStringUTF(env, "pdp1.properties"));
//	JniCallStaticVoidMethod(env, pdpClass, mainMethod, arg);
	JniCallStaticVoidMethod(env, pdpClass, mainMethod);
	if (JniExceptionOccurred(env)) {
		printf("Exception in " METHOD_MAIN_NAME ".\n");
		JniExceptionDescribe(env);
		exit(1);
	}

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
	// get class pdp controller
	classController = JniFindClass(mainJniEnv, CLASS_CONTROLLER);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("Could not find class " CLASS_CONTROLLER ".\n");
		JniExceptionDescribe(mainJniEnv);
		return false;
	}

	jmethodID methodIsStarted = JniGetStaticMethodID(mainJniEnv, classController, METHOD_ISSTARTED_NAME, METHOD_ISSTARTED_SIG);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("Could not find " METHOD_ISSTARTED_NAME " method.\n");
		JniExceptionDescribe(mainJniEnv);
		return false;
	}

	printf("Waiting for Controller to get started ...");

	// Leave the UC framework some time to initialize. Otherwise an exception will occur.
	// TODO: How can we get rid of this workaround?
	sleep(1);

	jboolean isStarted = JNI_FALSE;
	while (isStarted == JNI_FALSE) {
		isStarted = JniCallStaticBooleanMethod(mainJniEnv, classController, methodIsStarted);
		if (JniExceptionCheck(mainJniEnv)) {
			printf("\nException in " METHOD_ISSTARTED_NAME "():\n");
			JniExceptionDescribe(mainJniEnv);
		}
		printf(".");
		fflush(stdout);
		sleep(1);
	}

	printf("\n");

	return true;
}


bool getMainJniRefs() {
	classNativeHandler = JniFindClass(mainJniEnv, CLASS_NATIVE_HANDLER);
	if (JniExceptionCheck(mainJniEnv)) {
		printf("Could not find class " CLASS_NATIVE_HANDLER ".\n");
		return false;
	}

	methodNotifyEvent = JniGetMethodID(mainJniEnv, classNativeHandler, METHOD_NOTIFY_NAME, METHOD_NOTIFY_SIG);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("Could not access method " METHOD_NOTIFY_NAME " in class " CLASS_CONTROLLER ".\n");
		return false;
	}

	jfieldID fieldNativeHandler = JniGetStaticFieldID(mainJniEnv, classController, FIELD_NATIVE_HANDLER, FIELD_NATIVE_HANDLER_SIG);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("(1) Could not access field " FIELD_NATIVE_HANDLER " in class " CLASS_CONTROLLER ".\n");
		return false;
	}

	nativeHandler = JniGetStaticObjectField(mainJniEnv, classController, fieldNativeHandler);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("(2) Could not access field " FIELD_NATIVE_HANDLER " in class " CLASS_CONTROLLER ".\n");
		return false;
	}

	classString = JniFindClass(mainJniEnv, CLASS_STRING);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("Could not find class " CLASS_STRING ".\n");
		return false;
	}

	classThrowable = JniFindClass(mainJniEnv, CLASS_THROWABLE);

	return true;
}

void ucEnd() {
	while(1);
}

bool ucInit() {
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

	if (!waitForStartupCompletion()) {
		printf("Error while starting up JVM. Exiting.\n");
		exit(1);
	}

	if (!getMainJniRefs()) {
		printf("Error getting main JNI references. Exiting.\n");
		exit(1);
	}

	// start new thread to wait for JVM to die (which should not happen)
	if (pthread_create( &jvmWaiter, NULL, threadJvmWaiter, NULL) != 0) {
		printf("Unable to create JVM waiter thread.\n");
	}

	return true;
}





//int ucDesired(struct tcb *tcp) {
//	int retval = ucPDPask(tcp);
//
//	switch(retval) {
//		case UC_PDP_ALLOW:
//		case UC_PDP_MODIFY:
//			ucPIP_update(tcp);
//			break;
//		case UC_PDP_INHIBIT:
//			break;
//		case UC_PDP_DELAY:
//			break;
//	}
//
//	return (retval);
//}


//int ucActual(struct tcb *tcp) {
//	int retval = ucPDPask(tcp);
//
//	switch(retval) {
//		case UC_PDP_ALLOW:
//			ucPIP_update(tcp);
//			break;
//		case UC_PDP_DELAY:
//		case UC_PDP_MODIFY:
//		case UC_PDP_INHIBIT:
//			// TODO: does it make sense to modify/delay/inhibit
//			// after the call has been executed???
//			break;
//	}
//
//	return (retval);
//}



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

	ev->isActual = actual;

	/*
	 * SYS_exit and SYS_exit_group will never return;
	 * SYS_execve will not return on success.
	 * Always signal as actual event.
	 */
	if (!actual && (tcp->scno == SYS_exit || tcp->scno == SYS_exit_group || tcp->scno == SYS_execve)) {
		ev->isActual = true;
	}

	uc_log("notifying PDP... ");
	notifyEventToPdp(ev);
	uc_log("done.");

	destroyEvent(ev);
	uc_log("\n");
}

