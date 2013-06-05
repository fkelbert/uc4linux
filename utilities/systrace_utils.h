/*
 * systrace_utils.h
 *
 *      Authors: Carolina Lorini, Ricardo Neisse
 */

#ifndef systrace_utils_h
#define systrace_utils_h

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/stat.h>      // for fstat()
#include <sys/limits.h>    // LONG_MAX, LONG_MIN
#include <dev/systrace.h>  // This should be the last include

#include "logger.h"

// TODO: get this number from kernel header file
#define AMOUNT_OF_FDS 128
#define AMOUNT_OF_PIDS 100000


#define INTERCEPT_MAXSYSCALLARGS 10

// Translation should have no symlinks
#define ICTRANS_NOLINKS 1

// Taken from intercept.h
struct intercept_replace {
  int num;
  int ind[INTERCEPT_MAXSYSCALLARGS];
  char *address[INTERCEPT_MAXSYSCALLARGS];
  size_t len[INTERCEPT_MAXSYSCALLARGS];
  unsigned int flags[INTERCEPT_MAXSYSCALLARGS];
};

// This method sets all bytes to 0
void initializeFDTable(void);

/*
 * This method stores the mapping between the
 * filedescriptor and the filename
 *
 * @PARAM:	fd		the filepointer
 * @PARAM:	filename	the corresponding filename
 *
 * @RETURN	0	if everything is fine
 * 		-1	if there was an error
 */
int store_fd_filename_mapping( int pid, int fd, char *filename );

/*
 * This method deletes the mapping between the
 * filedescriptor and the filename
 *
 * @PARAM:	fd	the filepointer
 *  @RETURN	0	if everything went well
 * 		-1	if there was an error
 */
int delete_fd_filename_mapping( int pid, int fd );

char* get_file_name(int pid, int fd);

/*
 * Method used to copy bytes from the address
 * space of the traced program to the address
 * space of the handler
 * @PARAM:	fd	the filepointer of the DEVICE
 * @PARAM:	pid	the PID of the traces process
 * @PARAM:	op	do we want to read or write?
 * @PARAM:	addr	from where do we read (addr in userspace)
 * @PARAM:	buf	where do we store the read bytes
 * @PARAM:	size	how many bytes shoud be read
 *
 * @RETURN how many bytes are indeed read
 */
int copy_io(int fd, pid_t pid, int op, void *addr, void *buf, size_t size);

/*
 * This method reads the string where source points to
 *
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	op		do we want to read or write?
 * @PARAM:	source		from where do we read
 * @PARAM:	target		where do we store the read bytes
 */
int get_string_buffer( int fd, pid_t pid, int op, void *source, void *target );

/*
 * This method reads SIZE bytes from where SOURCE points to
 * and stores the bytes where TARGET points to
 * 
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	source		from where do we read
 * @PARAM:	target		where do we store the read bytes
 * @PARAM:	size		the amount of bytes we process
 * 
 * @RETURN: how many bytes are indeed processed
 */
int get_fixed_size_buffer( int fd, pid_t pid, void *source, void *target, size_t size );

/*
 * This method writes SIZE bytes from where SOURCE points to
 * and stores the bytes where TARGET points to
 * 
 * @PARAM: 	fd		the filepointer of the DEVICE
 * @PARAM: 	pid		the PID of the traces process
 * @PARAM: 	source		from where do we read
 * @PARAM: 	target		where do we store the read bytes
 * @PARAM: 	size		the amount of bytes we process
 * 
 * @RETURN: how many bytes are indeed processed
 */
int set_fixed_size_buffer( int fd, pid_t pid, void *source, void *target, size_t size );


int obsd_replace(int fd, pid_t pid, u_int16_t seqnr, struct intercept_replace *repl);

/* This method will replace the filename of
 * the open syscall
 *
 * @PARAM:	 cfd		filepointer to /dev/systrace
 * @PARAM:	 pid		the pid of the traced process
 * @PARAM:	 seqnr		the sequence number of the open syscall
 * 				where you want to change the filename
 * @PARAM:	 newFilename	the new filename
 *
 */
void open_filename_replace( int cfd, pid_t pid, u_int16_t seqnr, char* newFilename );

void open_flag_replace( int cfd, pid_t pid, u_int16_t seqnr, int flag );


int get_fd(int pid, char *filename );
#endif
