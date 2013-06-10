#include "helpers.h"

/*
 * Gets the information about the user of a determined process.
 *
 * @param pid Pid of the process whose user information is desired to know.
 *
 * @return Structure with the user information.
 */
struct passwd *getUserInfo(int pid){
	int buflen = 512;
	FILE *f;
	char filename[buflen];
	char buffer[buflen];
	int uid, dummy;

	/* Open the status file of the process in order to know
	 * the user id of the process */
	snprintf(filename,buflen,"/proc/%d/status",pid);
	f=fopen(filename,"r");
	/* Scan the file until we find the uid */
	do{
		fscanf(f,"%s\t%d\t%d",buffer, &dummy,&uid);
	}while(strstr(buffer,"Uid") == NULL);
	/* Get the user struct from the uid */

	return ((struct passwd *) getpwuid (uid));
}
