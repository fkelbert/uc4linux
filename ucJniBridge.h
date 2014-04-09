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
#define CLASS_NATIVE_HANDLER 	MAIN_PKG "cm/handlers/NativeHandler"

#define FIELD_NATIVE_HANDLER 		"_nativeHandler"
#define FIELD_NATIVE_HANDLER_SIG	JNI_TYPE(CLASS_NATIVE_HANDLER)

#define METHOD_EXC_MESSAGE_NAME	"getMessage"
#define METHOD_EXC_MESSAGE_SIG	"()" JNI_STRING

#define METHOD_MAIN_NAME 		"main"
#define METHOD_MAIN_SIG 		"([" JNI_STRING ")" JNI_VOID

#define METHOD_ISSTARTED_NAME 	"started"
#define METHOD_ISSTARTED_SIG 	"()" JNI_BOOL

#define METHOD_NOTIFY_NAME 		"notifyEvent"
#define METHOD_NOTIFY_SIG 		"(" JNI_STRING "[" JNI_STRING "[" JNI_STRING JNI_BOOL ")" JNI_OBJECT

#endif
