#ifndef UC_SETTINGS_H
#define UC_SETTINGS_H

#define UC_ENABLED 1
#define UC_DEBUG 1

#define USER_CLASSPATH "../pdp/CommunicationManager/target/CommunicationManager-1.0-jar-with-dependencies.jar"

static const char *ignoredFiles[] = {
		"/dev",
		"/etc",
		"/lib",
		"/opt",
		"/proc",
		"/usr",
		"/var",
		NULL
};


#define PROCFS_MNT "/proc"

#endif
