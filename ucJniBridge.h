#ifndef UC_JNI_BRIDGE
#define UC_JNI_BRIDGE

#define CLASS_STRING "java/lang/String"
#define CLASS_OBJECT "java/lang/Object"

#define JNI_TYPE(T)	"L" T ";"
#define JNI_VOID 	"V"
#define JNI_BOOL 	"Z"
#define JNI_OBJECT 	JNI_TYPE(CLASS_OBJECT)
#define JNI_STRING 	JNI_TYPE(CLASS_STRING)

#define PKG_PDP_CONTROLLER 			"de/tum/in/i22/pdp/"

#define CLASS_PDP_CONTROLLER 		PKG_PDP_CONTROLLER "PdpController"
#define CLASS_PEP_NATIVE_HANDLER 	"de/tum/in/i22/pdp/cm/in/pep/PepClientNativeHandler"

#define FIELD_NATIVE_PEP 		"nativePepHandler"
#define FIELD_NATIVE_PEP_SIG	JNI_TYPE(CLASS_PEP_NATIVE_HANDLER)

#define METHOD_MAIN_NAME 		"main"
#define METHOD_MAIN_SIG 		"([" JNI_STRING ")" JNI_VOID

#define METHOD_ISSTARTED_NAME 	"isStarted"
#define METHOD_ISSTARTED_SIG 	"()" JNI_BOOL

#define METHOD_NOTIFY_NAME 		"notifyEvent"
#define METHOD_NOTIFY_SIG 		"(" JNI_STRING "[" JNI_STRING "[" JNI_STRING JNI_BOOL ")" JNI_OBJECT

#endif
