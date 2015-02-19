#ifndef UC_SETTINGS_H
#define UC_SETTINGS_H

#define UC_ENABLED 1
#define UC_DEBUG 1

// Enable _either_ of the two following options
#define UC_JNI 0
#define UC_THRIFT 1

#define UC_ONLY_EXECVE 0
#define UC_ONLY_EXECVE_TWICE 0 // send execve as desired and as actual

#define UC_THRIFT_SSL_ENABLED 0
#define UC_THRIFT_PDP_PORT 21003

#define USER_CLASSPATH "Jni-1.0-jar-with-dependencies.jar"

#define UC_THRIFT_SSL_CERTIFICATE_DIRECTORY "/home/florian/certificates/"
#define UC_THRIFT_SSL_CA 					UC_THRIFT_SSL_CERTIFICATE_DIRECTORY "/CA.pem"

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
