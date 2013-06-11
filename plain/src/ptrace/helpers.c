#include "helpers.h"

/*
 * Gets the information about the user of a determined process.
 *
 * @param pid Pid of the process whose user information is desired to know.
 *
 * @return Structure with the user information; possibly NULL in case of error.
 */
struct passwd *getUserInfo(int pid) {
	int buflen = 512;
	FILE *f;
	char filename[buflen];
	char buffer[buflen];
	int uid, dummy;

	/* Open the status file of the process in order to know
	 * the user id of the process */
	snprintf(filename, buflen, "/proc/%d/status", pid);
	f = fopen(filename, "r");
	/* Scan the file until we find the uid */
	do {
		fscanf(f, "%s\t%d\t%d", buffer, &dummy, &uid);
	}
	while (strstr(buffer, "Uid") == NULL);
	/* Get the user struct from the uid */

	return ((struct passwd *) getpwuid(uid));
}

/**
 * Get the processes command line
 * @param pid the process id
 * @param buf the buffer into which the command line is stored
 * @param len the length of the buffer
 * @return buf, or NULL
 */
char *getCmdline(int pid, char *buf, int len) {
	int fnbuflen = 512;
	char filename[fnbuflen];
	FILE *f = NULL;

	snprintf(filename, fnbuflen, "/proc/%d/cmdline", pid);
	if ((f = fopen(filename, "r"))) {
		return (fgets(buf, len, f));
	}

	perror("fopen");
	return NULL;
}
