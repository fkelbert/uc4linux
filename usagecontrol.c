#include "usagecontrol.h"

volatile JavaVM *jvm = NULL;

void *createJvmThread(void *args) {
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
	jarray arg = (*env)->NewObjectArray(env, 0, (*env)->FindClass(env, JNI_STRING), 0);
//	(*env)->SetObjectArrayElement(env, arg, 0, (*env)->NewStringUTF(env, "-pp"));
//	(*env)->SetObjectArrayElement(env, arg, 1, (*env)->NewStringUTF(env, "pdp1.properties"));
	(*env)->CallStaticVoidMethod(env, pdpClass, mainMethod, arg);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Exception in " METHOD_MAIN_NAME ".\n");
		exit(1);
	}

	pthread_exit((void *) true);
}


bool ucInit() {
	pthread_t jvmThread;
	JNIEnv *env;
	void *ret;

	jclass classPdpController;
	jclass classPepHandler;
	jmethodID methodIsStarted;
	jmethodID methodNotifyEvent;
	jfieldID fieldPepHandler;
	jobject objPepHandler;
	jboolean isStarted;

	if (pthread_create( &jvmThread, NULL, createJvmThread, NULL) != 0) {
		printf("Unable to create JVM thread.\n");
	}

	printf("Wait for startup of JVM");
	while (jvm == NULL) {
		printf(".");
		usleep(30000);
	}
	printf("\n");



	// Attach this thread to JVM
	if ((*jvm)->AttachCurrentThread((JavaVM*) jvm, (void**) &env, NULL) != 0) {
		printf("Error attaching main thread to JVM.\n");
		exit(1);
	}

	// check whether we had found PdpController before
	classPdpController = (*env)->FindClass(env, CLASS_PDP_CONTROLLER);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Could not find class " CLASS_PDP_CONTROLLER ".\n");
		exit(1);
	}

	methodIsStarted = (*env)->GetStaticMethodID(env, classPdpController, METHOD_ISSTARTED_NAME, METHOD_ISSTARTED_SIG);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Could not find " METHOD_ISSTARTED_NAME " method.\n");
		exit(1);
	}

	printf("Waiting for PdpController to get started");
	isStarted = JNI_FALSE;
	while (isStarted == JNI_FALSE) {
		isStarted = (*env)->CallStaticBooleanMethod(env, classPdpController, methodIsStarted);
		if ((*env)->ExceptionOccurred(env)) {
			printf("Could not execute " METHOD_ISSTARTED_NAME " method.\n");
			exit(1);
		}
		printf(".");
		fflush(stdout);
		sleep(1);
	}
	printf("\n");



	fieldPepHandler = (*env)->GetStaticFieldID(env, classPdpController, FIELD_NATIVE_PEP, FIELD_NATIVE_PEP_SIG);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Could not find field " FIELD_NATIVE_PEP " in class " CLASS_PDP_CONTROLLER ".\n");
		exit(1);
	}


	objPepHandler = (*env)->GetStaticObjectField(env, classPdpController, fieldPepHandler);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Could not access field " FIELD_NATIVE_PEP " in class " CLASS_PDP_CONTROLLER ".\n");
		exit(1);
	}

	classPepHandler = (*env)->FindClass(env, CLASS_PEP_NATIVE_HANDLER);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Could not find class " CLASS_PEP_NATIVE_HANDLER ".\n");
		exit(1);
	}

	methodNotifyEvent = (*env)->GetMethodID(env, classPepHandler, METHOD_NOTIFY_NAME, METHOD_NOTIFY_SIG);
	if ((*env)->ExceptionOccurred(env)) {
		printf("Could not access method " METHOD_NOTIFY_NAME " in class " CLASS_PDP_CONTROLLER ".\n");
		exit(1);
	}


	jstring eventName = (*env)->NewStringUTF(env, "foo");

	jarray paramKeys = (*env)->NewObjectArray(env, 2, (*env)->FindClass(env, CLASS_STRING), 0);
	(*env)->SetObjectArrayElement(env, paramKeys, 0, (*env)->NewStringUTF(env, "key1"));
	(*env)->SetObjectArrayElement(env, paramKeys, 1, (*env)->NewStringUTF(env, "key2"));

	jarray paramValues = (*env)->NewObjectArray(env, 2, (*env)->FindClass(env, CLASS_STRING), 0);
	(*env)->SetObjectArrayElement(env, paramValues, 0, (*env)->NewStringUTF(env, "val1"));
	(*env)->SetObjectArrayElement(env, paramValues, 1, (*env)->NewStringUTF(env, "val2"));

	(*env)->CallVoidMethod(env, objPepHandler, methodNotifyEvent, eventName, paramKeys, paramValues);


	eventName = (*env)->NewStringUTF(env, "Socket");

	paramKeys = (*env)->NewObjectArray(env, 4, (*env)->FindClass(env, CLASS_STRING), 0);
	(*env)->SetObjectArrayElement(env, paramKeys, 0, (*env)->NewStringUTF(env, "PEP"));
	(*env)->SetObjectArrayElement(env, paramKeys, 1, (*env)->NewStringUTF(env, "host"));
	(*env)->SetObjectArrayElement(env, paramKeys, 2, (*env)->NewStringUTF(env, "pid"));
	(*env)->SetObjectArrayElement(env, paramKeys, 3, (*env)->NewStringUTF(env, "fd"));

	paramValues = (*env)->NewObjectArray(env, 4, (*env)->FindClass(env, CLASS_STRING), 0);
	(*env)->SetObjectArrayElement(env, paramValues, 0, (*env)->NewStringUTF(env, "Linux"));
	(*env)->SetObjectArrayElement(env, paramValues, 1, (*env)->NewStringUTF(env, "machine"));
	(*env)->SetObjectArrayElement(env, paramValues, 2, (*env)->NewStringUTF(env, "4562"));
	(*env)->SetObjectArrayElement(env, paramValues, 3, (*env)->NewStringUTF(env, "2"));

	(*env)->CallVoidMethod(env, objPepHandler, methodNotifyEvent, eventName, paramKeys, paramValues);
//	sleep(1);



	pthread_join(jvmThread, &ret);

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
	}
	else {
//		ucActual(tcp);
	}
}

