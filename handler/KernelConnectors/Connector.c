#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <err.h>
#include <dev/systrace.h>
#include "Connector.h"

/*
 * This method connects the
 * handler to Systrace
 *
 * @PARAM:	cfd	where the pointer to /dev/systrace should be stored
 */
void connectToKernel( int *cfd )
{
	int fd;

	if ((fd = open( DEVICE, O_RDONLY)) == -1)
		err( 1, DEVICE );

	/*
	 * Get a systrace descriptor.
	 */
	if (ioctl(fd, STRIOCCLONE, cfd) == -1)
		err( 1, "STRIOCCLONE" );
		
	close(fd);
}


/*
 * This method attaches a process
 * to the handler
 *
 * @PARAM:	cfd	the filepointer to the systrace API
 * 		pid	the pid of the traced process
 */
void attachProcess( int cfd, int *pid )
{
	if (ioctl(cfd, STRIOCATTACH, pid) == -1)
		err(1, "STRIOCATTACH");
}


/*
 * Method to install a new policy and permit all syscalls per default
 *
 * @PARAM:	cfd	file pointer to /dev/systrace
 * @PARAM:	pid	the pid of the traced process
 * @PARAM:	strpol	the systrace policy struct we want to use
 */
void installPolicy( int cfd, pid_t pid, struct systrace_policy *strpol )
{
	int i;
	
	/* Install one policy. */
	(*strpol).strp_op = SYSTR_POLICY_NEW;
	(*strpol).strp_maxents = NSYSCALLS;

	if (ioctl(cfd, STRIOCPOLICY, strpol) == -1)	/* &strpol */
		err(1, "STRIOCPOLICY NEW");

	(*strpol).strp_op = SYSTR_POLICY_ASSIGN;
	(*strpol).strp_pid = pid;

	if (ioctl(cfd, STRIOCPOLICY, strpol) == -1)	/* &strpol */
		err(1, "STRIOCPOLICY ASSIGN");

	/* Permit all system calls. */
	for (i = 0; i < NSYSCALLS; i++) {
		(*strpol).strp_op = SYSTR_POLICY_MODIFY;
		(*strpol).strp_code = i;
		(*strpol).strp_policy = SYSTR_POLICY_PERMIT;

		if (ioctl(cfd, STRIOCPOLICY, strpol) == -1)	/* &strpol */
			err(1, "STRIOCPOLICY MODIFY");
	}
}


/*
 * Method to register for syscalls
 * @PARAM:	cfd	file pointer to /dev/systrace
 * @PARAM: 	strpol	policy struct
 */
void registerSyscalls( int cfd, struct systrace_policy *strpol )
{
	/* Ask for systemcalls. */
	traceSyscall( strpol, cfd, 5 );	/* open syscall */
	traceSyscall( strpol, cfd, 6 );	/* close syscall */
	traceSyscall( strpol, cfd, 3 );	/* read syscall */
	traceSyscall( strpol, cfd, 4 );	/* write syscall */
}


/*
 * This method registers for a syscall
 * so that we get notified if the
 * syscall is used by any program
 *
 * @PARAM:	fd	filepointer of the DEVICE
 * @PARAM:	code	the representative code of the syscall we want to trace
 */
void traceSyscall( struct systrace_policy *strpol, int fd, int code )
{
	/* register for systemcalls. */
	(*strpol).strp_op = SYSTR_POLICY_MODIFY;
	(*strpol).strp_code = code;
	(*strpol).strp_policy = SYSTR_POLICY_ASK;

	if( ioctl( fd, STRIOCPOLICY, strpol ) == -1 )
		err( 1, "STRIOCPOLICY MODIFY" );
}

