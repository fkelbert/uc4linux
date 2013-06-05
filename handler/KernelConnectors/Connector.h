#ifndef CONNECTOR_H_
#define CONNECTOR_H_

/* device file */
#define DEVICE "/dev/systrace"
#define NSYSCALLS 512

/*
 * This method connects the
 * handler to Systrace
 *
 * @PARAM:	cfd	where the pointer to /dev/systrace should be stored
 */
void connectToKernel( int *cfd );


/*
 * This method attaches a process
 * to the handler
 *
 * @PARAM:	cfd	the filepointer to the systrace API
 * 		pid	the pid of the traced process
 */
void attachProcess( int cfd, int *pid );


/*
 * Method to install a new policy and permit all syscalls per default
 *
 * @PARAM:	fd	file pointer to /dev/systrace
 * @PARAM:	pid	the pid of the traced process
 * @PARAM:	strpol	the systrace policy struct we want to use
 */
void installPolicy( int cfd, pid_t pid, struct systrace_policy *strpol );


/*
 * Method to register for syscalls
 * @PARAM:	cfd	file pointer to /dev/systrace
 * @PARAM:	strpol	policy struct
 */
void registerSyscalls( int cfd, struct systrace_policy *strpol );


/*
 * This method registers for a syscall
 * so that you get notified if the
 * syscall is used by any program
 *
 * @PARAM:	strpol	policy struct
 * @PARAM:	fd	filepointer of the DEVICE
 * @PARAM:	code	the representative code of the syscall we want to trace
 */
void traceSyscall( struct systrace_policy *strpol, int fd, int code );

#endif // CONNECTOR_H_
