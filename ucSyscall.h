/*
 * ucSyscall.h
 *
 *  Created on: Jul 1, 2013
 *      Author: user
 */

#ifndef UCSYSCALL_H_
#define UCSYSCALL_H_

#include "syscall.h"

// these values are from syscallent.h
#define SYS_socket_subcall	400
#define SYS_ipc_subcall	420

#define SYS_socket (SYS_socket_subcall + 1)
#define SYS_bind (SYS_socket_subcall + 2)
#define SYS_connect (SYS_socket_subcall + 3)
#define SYS_listen (SYS_socket_subcall + 4)
#define SYS_accept (SYS_socket_subcall + 5)
#define SYS_getsockname (SYS_socket_subcall + 6)
#define SYS_getpeername (SYS_socket_subcall + 7)
#define SYS_socketpair (SYS_socket_subcall + 8)
#define SYS_send (SYS_socket_subcall + 9)
#define SYS_recv (SYS_socket_subcall + 10)
#define SYS_sendto (SYS_socket_subcall + 11)
#define SYS_recvfrom (SYS_socket_subcall + 12)
#define SYS_shutdown (SYS_socket_subcall + 13)
#define SYS_setsockopt (SYS_socket_subcall + 14)
#define SYS_getsockopt (SYS_socket_subcall + 15)
#define SYS_sendmsg (SYS_socket_subcall + 16)
#define SYS_recvmsg (SYS_socket_subcall + 17)
#define SYS_accept4 (SYS_socket_subcall + 18)
//#define SYS_recvmmsg (SYS_socket_subcall + 19)

#define SYS_semop (SYS_ipc_subcall + 21)
#define SYS_semget (SYS_ipc_subcall + 22)
#define SYS_semctl (SYS_ipc_subcall + 23)
#define SYS_semtimedop (SYS_ipc_subcall + 24)
#define SYS_msgsnd (SYS_ipc_subcall + 31)
#define SYS_msgrcv (SYS_ipc_subcall + 32)
#define SYS_msgget (SYS_ipc_subcall + 33)
#define SYS_msgctl (SYS_ipc_subcall + 34)
#define SYS_shmat (SYS_ipc_subcall + 41)
#define SYS_shmdt (SYS_ipc_subcall + 42)
#define SYS_shmget (SYS_ipc_subcall + 43)
#define SYS_shmctl (SYS_ipc_subcall + 44)

#define SYS_cloneFirstAction 501

#endif /* UCSYSCALL_H_ */
