#include "usagecontrol.h"

volatile JavaVM *jvm = NULL;

// Definitions to have all eclipse errors in one place rather than all over the C file...
#define JniFindClass(env,class) 							(*env)->FindClass(env, class)
#define JniExceptionOccurred(env)							(*env)->ExceptionOccurred(env)
#define JniGetStaticMethodID(env,class,name,sig)			(*env)->GetStaticMethodID(env, class, name, sig)
#define JniNewObjectArray(env,size,class,arg)	 			(*env)->NewObjectArray(env, size, class, arg)
#define JniNewStringUTF(env,str)							(*env)->NewStringUTF(env, str)
#define JniSetObjectArrayElement(env,arr,pos,elem)			(*env)->SetObjectArrayElement(env, arr, pos, elem);
#define JniCallStaticVoidMethod(env,class,method,...) 		(*env)->CallStaticVoidMethod(env, class, method, ##__VA_ARGS__);
#define JniCallStaticBooleanMethod(env,class,method,...)	(*env)->CallStaticBooleanMethod(env, class, method, ##__VA_ARGS__)
#define JniAttachCurrentThread(jvm,env,arg)					(*jvm)->AttachCurrentThread(jvm, env, arg)

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
    pdpClass = JniFindClass(env, CLASS_PDP_CONTROLLER);
	if (JniExceptionOccurred(env)) {
		printf("Could not find class " CLASS_PDP_CONTROLLER ".\n");
		exit(1);
	};

	// find the main method
	mainMethod = JniGetStaticMethodID(env, pdpClass, METHOD_MAIN_NAME, METHOD_MAIN_SIG);
	if (JniExceptionOccurred(env)) {
		printf("Method " METHOD_MAIN_NAME " not found.\n");
		exit(1);
	}

	// call the main method
	jarray arg = JniNewObjectArray(env, 2, JniFindClass(env, JNI_STRING), 0);
	JniSetObjectArrayElement(env, arg, 0, JniNewStringUTF(env, "-pp"));
	JniSetObjectArrayElement(env, arg, 1, JniNewStringUTF(env, "pdp1.properties"));
	JniCallStaticVoidMethod(env, pdpClass, mainMethod, arg);
	if (JniExceptionOccurred(env)) {
		printf("Exception in " METHOD_MAIN_NAME ".\n");
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

void notifyEventToPdp(event *ev) {
	jstring name = JniNewStringUTF(mainJniEnv, ev->name);
	jarray paramKeys = JniNewObjectArray(mainJniEnv, ev->cntParams, classString, 0);
	jarray paramVals = JniNewObjectArray(mainJniEnv, ev->cntParams, classString, 0);

	int i;
	for (i = 0; i < ev->cntParams; i++) {
		JniSetObjectArrayElement(mainJniEnv, paramKeys, i, JniNewStringUTF(mainJniEnv, (const char*) ev->params[i]->key));
		JniSetObjectArrayElement(mainJniEnv, paramVals, i, JniNewStringUTF(mainJniEnv, (const char*) ev->params[i]->val));
	}

	JniCallStaticVoidMethod(mainJniEnv, classPepHandler, methodNotifyEvent, name, paramKeys, paramVals, ev->isActual);
}


bool waitForStartupCompletion() {
	// get class pdp controller
	jclass classPdpController = JniFindClass(mainJniEnv, CLASS_PDP_CONTROLLER);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("Could not find class " CLASS_PDP_CONTROLLER ".\n");
		return false;
	}

	jmethodID methodIsStarted = JniGetStaticMethodID(mainJniEnv, classPdpController, METHOD_ISSTARTED_NAME, METHOD_ISSTARTED_SIG);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("Could not find " METHOD_ISSTARTED_NAME " method.\n");
		return false;
	}

	printf("Waiting for PdpController to get started");
	jboolean isStarted = JNI_FALSE;
	while (isStarted == JNI_FALSE) {
		isStarted = JniCallStaticBooleanMethod(mainJniEnv, classPdpController, methodIsStarted);
		if (JniExceptionOccurred(mainJniEnv)) {
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
	classPepHandler = JniFindClass(mainJniEnv, CLASS_PEP_NATIVE_HANDLER);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("Could not find class " CLASS_PEP_NATIVE_HANDLER ".\n");
		return false;
	}

	methodNotifyEvent = JniGetStaticMethodID(mainJniEnv, classPepHandler, METHOD_NOTIFY_NAME, METHOD_NOTIFY_SIG);
	if (JniExceptionOccurred(mainJniEnv)) {
		printf("Could not access method " METHOD_NOTIFY_NAME " in class " CLASS_PDP_CONTROLLER ".\n");
		return false;
	}

	classString = JniFindClass(mainJniEnv, CLASS_STRING);
	if (JniExceptionOccurred(mainJniEnv)) {
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
	if (JniAttachCurrentThread((JavaVM*) jvm, (void**) &mainJniEnv, NULL) != 0) {
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



//	event ev;
//	ev.name = "foo";
//	ev.isActual = true;
//	ev.cntParams = 3;
//	ev.params = malloc (ev.cntParams * sizeof(param));
//	ev.params[0].key = "k1";
//	ev.params[1].key = "k2";
//	ev.params[2].key = "k3";
//	ev.params[0].val = "v1";
//	ev.params[1].val = "v2";
//	ev.params[2].val = "v3";
//	notifyEventToPdp(&ev);

	event *ev = createEventWithStdParams("foo", 3);
	addParam(ev, createParam("k1", "v1"));
	addParam(ev, createParam("k2", "v2"));
	addParam(ev, createParam("k3", "v3"));
	notifyEventToPdp(ev);
	destroyEvent(ev);


//	const char *paramSocket[][2] = {
//			{"PEP", "Linux"},
//			{"host", "machine"},
//			{"pid", "4562"},
//			{"fd", "2"},
//	};
//	notifyEventToPdp("Socket", 4, (const char***) paramSocket);



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
	bool actual = !exiting(tcp);

	uc_log("%3d (%c) %s ... ", tcp->scno, actual ? 'A' : 'D', tcp->s_ent->sys_name);

	if (!ucSemanticsDefined(tcp->scno)) {
		uc_log("ignoring.\n");
		return;
	}

	event *ev = ucSemanticsFunct[tcp->scno](tcp);
	if (ev == NULL) {
		uc_log("returned NULL.\n");
		return;
	}

	if (!actual) {
//		ucDesired(tcp);
		ev->isActual = false;
		uc_log("--- DESIRED ---\n");
	}
	else {
		uc_log("--- ACTUAL --- ... executing.\n");
		ev->isActual = true;
		notifyEventToPdp(ev);
//		ucActual(tcp);
	}

	destroyEvent(ev);
}

