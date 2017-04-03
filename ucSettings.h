#ifndef UC_SETTINGS_H
#define UC_SETTINGS_H

#define STRACE_SILENT 1 // Make default strace silent (only effective if UC_ENABLED 0)

#define UC_ENABLED 1

#define UC_STRACE_ONLY 0 // only do strace interposition
#define UC_UC4LINUX_ONLY 0 // do not notify to PDP (but do everything on the C side)

#define UC_DEBUG 0
#define UC_LOG_EVENT_PARAMETERS 0
#define UC_LOG_TIME 0

// Enable _either_ of the two following options
#define UC_JNI 1
#define UC_THRIFT 0

#define UC_ONLY_EXECVE 0

#define UC_THRIFT_SSL_ENABLED 1
#define UC_THRIFT_PDP_PORT 21003

#define USER_CLASSPATH "Jni-1.0-jar-with-dependencies.jar"

#define UC_THRIFT_SSL_CERTIFICATE_DIRECTORY "/home/user/acns/config/certificates/"
#define UC_THRIFT_SSL_CA 					UC_THRIFT_SSL_CERTIFICATE_DIRECTORY "/CA.pem"


// File to ignore. BEWARE: function ignoreFilename() only ignores files starting with '/'
static const char *ignoredFiles[] = {
		"/dev",
		"/etc",
		"/lib",
		"/opt",
		"/proc",
		"/usr",
//		"/var",
		NULL
};

#define PROCFS_MNT "/proc"

#endif
