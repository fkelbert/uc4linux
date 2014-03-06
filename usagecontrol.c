#include "usagecontrol.h"

volatile JavaVM *jvm = NULL;

JNIEnv *mainJniEnv;

// CAUTION!
// The following references are only valid within the main thread!
jclass classPepHandler;
jmethodID methodNotifyEvent;
jclass classString;

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

    // find the PdpController
	pdpClass = (*env)->FindClass(env, CLASS_PDP_CONTROLLER);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Could not find class " CLASS_PDP_CONTROLLER ".\n");
		exit(1);
	};

	// find the main method
	mainMethod = (*env)->GetStaticMethodID(env, pdpClass, METHOD_MAIN_NAME, METHOD_MAIN_SIG);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Method " METHOD_MAIN_NAME " not found.\n");
		exit(1);
	}

	// call the main method
	jarray arg = (*env)->NewObjectArray(env, 2, (*env)->FindClass(env, JNI_STRING), 0);
	(*env)->SetObjectArrayElement(env, arg, 0, (*env)->NewStringUTF(env, "-pp"));
	(*env)->SetObjectArrayElement(env, arg, 1, (*env)->NewStringUTF(env, "pdp1.properties"));
	(*env)->CallStaticVoidMethod(env, pdpClass, mainMethod, arg);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Exception in " METHOD_MAIN_NAME ".\n");
		exit(1);
	}

	// The above main() (i.e. this thread) should actually never exit
	pthread_exit((void *) true);
}


// This thread will wait for the JVM thread to exit,
// which will hopefully never happen
void *threadJvmWaiter(void *args) {
	void *ret;
	printf("WAITING for JVM to die!\n");
	pthread_join(jvmStarter, &ret);
	printf("JVM died. Exiting.\n");
	exit(1);
}


//void notifyEventToPep(char *name, int cntParams, const char ***params) {
//	jstring jName = (*mainJniEnv)->NewStringUTF(mainJniEnv, name);
//	jarray jKeys = (*mainJniEnv)->NewObjectArray(mainJniEnv, cntParams, classString, 0);
//	jarray jVals = (*mainJniEnv)->NewObjectArray(mainJniEnv, cntParams, classString, 0);
//
//	int i;
//	for (i = 0; i < cntParams; i++) {
//		(*mainJniEnv)->SetObjectArrayElement(mainJniEnv, jKeys, i, (*mainJniEnv)->NewStringUTF(mainJniEnv, (const char*) params[i*2]));
//		(*mainJniEnv)->SetObjectArrayElement(mainJniEnv, jVals, i, (*mainJniEnv)->NewStringUTF(mainJniEnv, (const char*) params[i*2+1]));
//	}
//
//	(*mainJniEnv)->CallStaticVoidMethod(mainJniEnv, classPepHandler, methodNotifyEvent, jName, jKeys, jVals);
//}

void notifyEventToPep(event *ev) {
	jstring name = (*mainJniEnv)->NewStringUTF(mainJniEnv, ev->name);
	jarray paramKeys = (*mainJniEnv)->NewObjectArray(mainJniEnv, ev->cntParams, classString, 0);
	jarray paramVals = (*mainJniEnv)->NewObjectArray(mainJniEnv, ev->cntParams, classString, 0);

	int i;
	for (i = 0; i < ev->cntParams; i++) {
		(*mainJniEnv)->SetObjectArrayElement(mainJniEnv, paramKeys, i, (*mainJniEnv)->NewStringUTF(mainJniEnv, (const char*) ev->params[i*2]));
		(*mainJniEnv)->SetObjectArrayElement(mainJniEnv, paramVals, i, (*mainJniEnv)->NewStringUTF(mainJniEnv, (const char*) ev->params[i*2+1]));
	}

	(*mainJniEnv)->CallStaticVoidMethod(mainJniEnv, classPepHandler, methodNotifyEvent, name, paramKeys, paramVals, ev->isActual);
}


bool waitForStartupCompletion() {
	// get class pdp controller
	jclass classPdpController = (*mainJniEnv)->FindClass(mainJniEnv, CLASS_PDP_CONTROLLER);
	if ((*mainJniEnv)->ExceptionOccurred(mainJniEnv)) {
		printf("Could not find class " CLASS_PDP_CONTROLLER ".\n");
		return false;
	}

	jmethodID methodIsStarted = (*mainJniEnv)->GetStaticMethodID(mainJniEnv, classPdpController, METHOD_ISSTARTED_NAME, METHOD_ISSTARTED_SIG);
	if ((*mainJniEnv)->ExceptionOccurred(mainJniEnv)) {
		printf("Could not find " METHOD_ISSTARTED_NAME " method.\n");
		return false;
	}

	printf("Waiting for PdpController to get started");
	jboolean isStarted = JNI_FALSE;
	while (isStarted == JNI_FALSE) {
		isStarted = (*mainJniEnv)->CallStaticBooleanMethod(mainJniEnv, classPdpController, methodIsStarted);
		if ((*mainJniEnv)->ExceptionOccurred(mainJniEnv)) {
			printf("Could not execute " METHOD_ISSTARTED_NAME " method.\n");
			return false;
		}
		printf(".");
		fflush(stdout);
		sleep(1);
	}
	printf("\n");

	return true;
}


bool getMainJniRefs() {
	classPepHandler = (*mainJniEnv)->FindClass(mainJniEnv, CLASS_PEP_NATIVE_HANDLER);
	if ((*mainJniEnv)->ExceptionOccurred(mainJniEnv)) {
		printf("Could not find class " CLASS_PEP_NATIVE_HANDLER ".\n");
		return false;
	}

	methodNotifyEvent = (*mainJniEnv)->GetStaticMethodID(mainJniEnv, classPepHandler, METHOD_NOTIFY_NAME, METHOD_NOTIFY_SIG);
	if ((*mainJniEnv)->ExceptionOccurred(mainJniEnv)) {
		printf("Could not access method " METHOD_NOTIFY_NAME " in class " CLASS_PDP_CONTROLLER ".\n");
		return false;
	}

	classString = (*mainJniEnv)->FindClass(mainJniEnv, CLASS_STRING);
	if ((*mainJniEnv)->ExceptionOccurred(mainJniEnv)) {
		printf("Could not find class " CLASS_STRING ".\n");
		return false;
	}

	return true;
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
	if ((*jvm)->AttachCurrentThread((JavaVM*) jvm, (void**) &mainJniEnv, NULL) != 0) {
		printf("Error attaching main thread to JVM. Exiting.\n");
		exit(1);
	}


	if (!waitForStartupCompletion()) {
		printf("Error while starting up JVM. Exiting.\n");
		exit(1);
	}

	if (!getMainJniRefs()) {
		printf("Error getting main JNI references. Exiting.\n");
		exit(1);
	}



	event ev;
	ev.name = "foo";
	ev.isActual = true;
	const char *params[] = {
			"key1", "val1",
			"key2", "val2",
			"key3", "val3"
	};
	ev.params = params;
	ev.cntParams = 3;
	notifyEventToPep(&ev);


//	const char *paramSocket[][2] = {
//			{"PEP", "Linux"},
//			{"host", "machine"},
//			{"pid", "4562"},
//			{"fd", "2"},
//	};
//	notifyEventToPep("Socket", 4, (const char***) paramSocket);



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


/**
 * This function is used by strace to notify our
 * usage control framework about the fact that a system call
 * is happening. This call may be both desired or actual.
 *
 * If firstCall == 0, this method will invoke all
 * corresponding usage control functionality.
 * If firstCall == 1, then
 *
 *
 */
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
	if (exiting(tcp)) {
//		ucDesired(tcp);
		printf("%3d (d) %s\n", tcp->scno, tcp->s_ent->sys_name);
	}
	else {
		printf("%3d (a) %s\n", tcp->scno, tcp->s_ent->sys_name);
//		ucActual(tcp);
	}
}

