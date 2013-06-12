/*
 * systrace_utils.h
 *      Authors: Carolina Lorini, Ricardo Neisse
 */

#ifndef systrace_utils_h
#define systrace_utils_h

#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <err.h>
#include <sys/ioctl.h>
#include <sys/param.h>     // for MAXPATHLEN
#include <sys/stat.h>      // for fstat()
#include <sys/limits.h>    // LONG_MAX, LONG_MIN
#include <dev/systrace.h>  // This should be the last include
#include <sys/syscall.h>   // for MAX_SYSCALL
#include <glib/ghash.h>

// TODO: get this number from kernel header file
#define AMOUNT_OF_FDS 128
#define AMOUNT_OF_PIDS 1000000


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

unsigned int initializeFDTable();

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
unsigned int store_fd_filename_mapping( int pid, int fd, char *filename );

/*
 * This method deletes the mapping between the filedescriptor and the filename
 *
 * @PARAM:	fd	the filepointer
 * @RETURN	R_SUCCESS	if everything went well
 *      		R_ERROR  	if there was an error
 */
unsigned int delete_fd_filename_mapping( int pid, int fd );
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

unsigned char *do_attach_process(unsigned char *msg);
int init_systrace();
int ask_syscalls(int cfd, int syscall_code, struct systrace_policy strpol);
int install_policy(int pid);
unsigned int answerSystrace(struct str_message *sys_msg, struct systrace_answer *sys_ans);



// move to syscallHandler!
int obsd_replace(pid_t pid, u_int16_t seqnr, struct intercept_replace *repl);

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
void open_filename_replace(pid_t pid, u_int16_t seqnr, char* newFilename );

void open_flag_replace(pid_t pid, u_int16_t seqnr, int flag );

#endif
