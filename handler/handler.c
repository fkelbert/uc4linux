#include <sys/param.h>
#include <sys/ioctl.h>
#include <dev/systrace.h>

#include <err.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include <string.h>	/* for memcpy() */
#include <ctype.h>	/* for toupper() */

#include "./headers/definitions.h"
#include "./headers/variables.h"
#include "./utilities/functions.h"
//#include "./gui/ControllerGUI.h"
#include "./DataHandlers/DataHandler.h"
#include "./KernelConnectors/Connector.h"
#include "./utilities/encryption.h"

int main(int argc, char *argv[])
{
	struct systrace_policy strpol;
	struct systrace_answer strans;
	struct str_message strmsg;
	int cfd, pid;
	ssize_t n;
	
	/* initialize FD_Table */
	initializeFDTable();

	/* iget PID of the traced process */
	pid = atoi(argv[1]);
	
	/* connect to Kernel */
	connectToKernel( &cfd );
	
	/* attach a process to systrace */
	attachProcess( cfd, &pid );

	/* Install one policy. */
	installPolicy( cfd, pid, &strpol );

	/* Ask us about system calls. */
	registerSyscalls( cfd, &strpol );

	/* install key */
	initializeDataHandler();

	/*
	 * Now this process just answers requests for the operations the
	 * traced process performs that we have requested systrace to ask
	 * us about.
	 */
	      
	while( ( n = read( cfd, &strmsg, sizeof( strmsg ) ) ) == sizeof( strmsg ) )
	{
		switch (strmsg.msg_type)
		{
			case SYSTR_MSG_ASK:
			{
				/* get the sequence number of the received message */
				u_int16_t seqnr = strmsg.msg_seqnr;
		
				switch( strmsg.msg_data.msg_ask.code )
				{
					case 5:		/* open syscall */
					{
						/* handle open systemcall */
						handleOpenBefore( cfd, seqnr, &strmsg );
						break;
					}
					
					case 6:		/* close syscall */
					{
						/* handle close systemcall */
						handleCloseBefore( seqnr, &strmsg );
						break;
					}
					
					case 3:		/* read syscall */
					{
						/* handle read systemcall */
						handleReadBefore( cfd, seqnr, &strmsg );
						break;
					}
					
					case 4:		/* write syscall */
					{
						/* handle write systemcall */
						handleWriteBefore( cfd, seqnr, &strmsg );
						break;
					}
				}
		
				memset(&strans, 0, sizeof(strans));
				strans.stra_pid = strmsg.msg_pid;
				strans.stra_seqnr = strmsg.msg_seqnr;
				strans.stra_policy = SYSTR_POLICY_PERMIT;
				strans.stra_flags = SYSTR_FLAGS_RESULT;		/* we want the result of the syscall */
	
				if (ioctl(cfd, STRIOCANSWER, &strans) == -1)
					err(1, "STRIOCANSWER");
					
				break;
			}

			case SYSTR_MSG_RES:
			{
				u_int16_t seqnr = strmsg.msg_seqnr;
				
				switch( strmsg.msg_data.msg_ask.code )
				{
					case 5:		/* open syscall */
					{
						/* handle open systemcall */
						handleOpenAfter( cfd, seqnr, &strmsg );
						break;
					}

					case 3:		/* answer of read syscall */
					{
						/* handle read systemcall */
						handleReadAfter( cfd,seqnr, &strmsg );
						break;
					}
					case 4: /* write */
					{
						/* handle write systemcall */
						handleWriteAfter( cfd, seqnr, &strmsg );
						break;
					}
				}
			
				memset(&strans, 0, sizeof(strans));
				strans.stra_pid = strmsg.msg_pid;
				strans.stra_seqnr = strmsg.msg_seqnr;
				strans.stra_policy = SYSTR_POLICY_PERMIT;	/* the permission is not checked again, because */
									/*   the system call has already taken place */
	
				if (ioctl(cfd, STRIOCANSWER, &strans) == -1)
					err(1, "STRIOCANSWER");
			}
				
		}

	}

	if (n == -1)
		err(1, "read");
	close(cfd);
	exit(0);
}

