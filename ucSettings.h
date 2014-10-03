#ifndef UC_SETTINGS_H
#define UC_SETTINGS_H

#define UC_ENABLED 1
#define UC_DEBUG 0

// Enable _either_ of the two following options
#define UC_JNI 0
#define UC_THRIFT 1

#define USER_CLASSPATH "Jni-1.0-jar-with-dependencies.jar"

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
