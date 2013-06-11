/*
 * syscalls.h
 *
 *  Created on: Jun 10, 2013
 *      Author: user
 */

#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#include <syscall.h>

#define SOCKET_OFFSET 349

#define SYS_socket 350
#define SYS_bind 351
#define SYS_connect 352
#define SYS_listen 353
#define SYS_accept 354
#define SYS_getsockname 355
#define SYS_getpeername 356
#define SYS_socketpair 357
#define SYS_send 358
#define SYS_recv 359
#define SYS_sendto 360
#define SYS_recvfrom 361
#define SYS_shutdown 362
#define SYS_setsockopt 363
#define SYS_getsockopt 364
#define SYS_sendmsg 365
#define SYS_recvmsg 366

#define INTERCEPT_EXECVE 1
#define INTERCEPT_SOCKETCALL 1

extern char *syscallTable[];
extern int syscallsIntercept[];

#endif /* SYSCALLS_H_ */
