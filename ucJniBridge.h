#ifndef UC_JNI_BRIDGE
#define UC_JNI_BRIDGE

#define CLASS_STRING 		"java/lang/String"
#define CLASS_OBJECT 		"java/lang/Object"
#define CLASS_THROWABLE		"java/lang/Throwable"

#define JNI_TYPE(T)	"L" T ";"
#define JNI_VOID 	"V"
#define JNI_BOOL 	"Z"
#define JNI_OBJECT 	JNI_TYPE(CLASS_OBJECT)
#define JNI_STRING 	JNI_TYPE(CLASS_STRING)

#define MAIN_PKG				"de/tum/in/i22/uc/"

#define CLASS_CONTROLLER 		MAIN_PKG "Controller"
#define CLASS_NATIVE_HANDLER 	MAIN_PKG "jni/NativeHandler"

#define FIELD_NATIVE_HANDLER 		"_instance"
#define FIELD_NATIVE_HANDLER_SIG	JNI_TYPE(CLASS_NATIVE_HANDLER)

#define METHOD_EXC_MESSAGE_NAME	"getMessage"
#define METHOD_EXC_MESSAGE_SIG	"()" JNI_STRING

#define METHOD_MAIN_NAME 		"main"
#define METHOD_MAIN_SIG 		"([" JNI_STRING ")" JNI_VOID

#define METHOD_ISSTARTED_NAME 	"isStarted"
#define METHOD_ISSTARTED_SIG 	"()" JNI_BOOL

#define METHOD_NOTIFY_NAME 		"notifyEvent"
#define METHOD_NOTIFY_SIG 		"(" JNI_STRING "[" JNI_STRING "[" JNI_STRING JNI_BOOL ")" JNI_OBJECT

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

#endif
