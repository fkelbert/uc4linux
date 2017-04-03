#include "ucJni.h"

#if UC_JNI

volatile JavaVM *jvm = NULL;

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
	options[2].optionString = "-Xms2048m";
	options[3].optionString = "-Xmx2048m";

	vm_args.version = JNI_VERSION_1_8; //JDK version. This indicates version 1.6
	vm_args.nOptions = nOptions;
    vm_args.options = options;
	vm_args.ignoreUnrecognized = 1;

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
		printf("3\n");
		printf("Method " METHOD_MAIN_NAME " not found.\n");
		exit(1);
	}

	// call the main method
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

inline void notifyEventToPdpJni(event *ev) {
	jarray paramKeys = JniNewObjectArray(mainJniEnv, ev->cntParams, classString, 0);
	jarray paramVals = JniNewObjectArray(mainJniEnv, ev->cntParams, classString, 0);

	int i;
	for (i = 0; i < ev->cntParams; i++) {
		JniSetObjectArrayElement(mainJniEnv, paramKeys, i, JniNewStringUTF(mainJniEnv, ev->params[i]->key));
		JniSetObjectArrayElement(mainJniEnv, paramVals, i, JniNewStringUTF(mainJniEnv, ev->params[i]->val));
	}

	jobject resp = JniCallObjectMethod(mainJniEnv, nativeHandler, methodNotifyEvent, JniNewStringUTF(mainJniEnv, ev->name), paramKeys, paramVals, ev->isActual);
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
	printf(" done.\n");

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


void ucInitJni() {
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

	ucTypesSetJniEnv(mainJniEnv);

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
}


#endif
