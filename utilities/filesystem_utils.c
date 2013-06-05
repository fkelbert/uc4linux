/*
 * filesystem_utils.c
 *
 *  Created on: 2011/08/08
 *      Author: Florian Kelbert
 */
#include "filesystem_utils.h"


/**
 * if command is chrooted, it returns the chroot dir in dir and 1 as return value; 0 otherwise.
 *
 */
int get_chroot_dir(char *dir, char* command) {
	if (command == NULL) {
		return 0;
	}

	if (strlen(command) == strlen("httpd") && strstr(command,"httpd") == command) {
		strncpy(dir,"/var/www",8);
		dir[8] = '\0';
		return 1;
	}

	return 0;
}



/**
 * This method returns the current working directory (cwd) of the process with
 * process id pid.
 *
 * This method forks a child process and makes use of the lsof command to
 * retrieve the cwd. cwd is returned in the provided buffer cwd of size size.
 *
 * in: pid
 * out: cwd
 * in: size of cwd
 * out: port
 *
 * returns: 0 on success, -1 on failure
 */
int get_cwd(int pid, char *cwd, int size) {
	int pfd[2];

	// pipe to communicate result of lsof to parent
	if (pipe(pfd) == -1) {
		perror("pipe");
		return -1;
	}

	int childpid = fork();

	if (childpid == -1) {
		perror("fork");
		close(pfd[0]);
		close(pfd[1]);
		return -1;
	}

	// the child will do lsof, while the parent will be waiting for the result

	if (childpid == 0) {
		/* child doing lsof */

		close(pfd[0]);		// close reading end of pipe
		close(1);			// close stdout
		dup(pfd[1]);		// duplicate writing end of pipe to stdout

		// assemble parameters for lsof: -p<PID>, -d<FD>
		char buf_pid[BUFSIZE];
		memset(buf_pid, 0, BUFSIZE);
		snprintf(buf_pid,BUFSIZE,"-p%d",pid);

		// execute lsof -a -p<PID> -d'cwd' -Fn
		if (execl("/usr/local/sbin/lsof","lsof","-a",buf_pid,"-d","cwd","-Fn",(void*)0) == -1) {
			perror("execl");
		}

		close(pfd[1]);
		exit(0);
	}
	else {
		// parent reading from lsof

		close(pfd[1]);		// close writing end of pipe


		// read from pipe to get result of lsof
		int recvd = 0;
		char buf[BUFSIZE];
		memset(buf, 0, BUFSIZE);
		int c = 0;
		while((recvd = read(pfd[0], buf+c, BUFSIZE-c)) > 0) {
			c += recvd;
		}

		if (recvd == -1) {
			close(pfd[0]);
			return -1;
		}

		// now let's find the cwd

		char trash[BUFSIZE];

		memset(cwd, 0, size);

		int read_trash = 0;
		int read_cwd = 0;

		// scan the lsof output; the corresponding line starts with 'n' (see manpage of lsof)
		if (sscanf(buf,"%s\n%nn%[^\n]\n%n",trash,&read_trash,cwd,&read_cwd) != 2) {
			close(pfd[0]);
			return -1;
		}

		if (read_cwd-read_trash >= size) {
			cwd[size-1] = '\0';
		}

		// wait for child
		int status;
		wait(&status);

		close(pfd[0]);
	}

	return 0;
}




/**
 * Returns the absolute path of the file with relative filename rel_filename in
 * the memory provided by abs_filename of size size.
 * If rel_filename starts with a '/', it is copied to abs_filename as-is.
 *
 * rel_filename is converted to an absolute filename relative to the current
 * working directory of the process with process id pid.
 *
 * out: abs_filename
 * in: size
 * in: rel_filename
 * in: pid
 *
 * returns: the address of abs_filename on success, NULL on failure
 */
char *get_absolute_filename(char *abs_filename, int size, char *rel_filename, int pid, char* command) {
	int tmpsize = 2 * PATH_MAX;
	char path[tmpsize];
	memset(abs_filename,0,size);
	memset(path,0,tmpsize);


	int isChrooted = get_chroot_dir(path,command);

	// TODO: THIS DOES NOT WORK IF A PROCESS IS CHROOTED!!!


	// check, whether rel_filename is absolute
	if (rel_filename[0] == '/' && !isChrooted) {
		strncpy(abs_filename,rel_filename,size);
		abs_filename[size-1] = '\0';
	}
	else {
  	  // get current working directory, add / and copy relative filename
	   	if (!isChrooted) {
		  	get_cwd(pid,path,PATH_MAX);
		  }

    if (path != NULL && strstr(path,"(/dev") != NULL && command != NULL) {
      if (strstr(command,"vsftpd") != NULL) {
        strncpy(path,"/var/www",PATH_MAX);
      }
      else if (strstr(command,"ftp") != NULL) {
        strncpy(path,"/var/www/test",PATH_MAX);
      }
    }

		strncpy(path+strlen(path),"/",1);
		strncpy(path+strlen(path),rel_filename, tmpsize - strlen(path) - 1);
		path[tmpsize-1] = '\0';


		// finally, resolve to realpath (see man realpath)
		if (realpath(path, abs_filename) == NULL) {
			return NULL;
		}

	}
	return abs_filename;
}
