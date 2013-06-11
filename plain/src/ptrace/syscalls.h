/*
 * syscalls.h
 *
 *  Created on: Jun 10, 2013
 *      Author: user
 */

#ifndef SYSCALLS_H_
#define SYSCALLS_H_

#define SOCKET_OFFSET 347

#define SYS_socket 348
#define SYS_bind 349
#define SYS_connect 350
#define SYS_listen 351
#define SYS_accept 352
#define SYS_getsockname 353
#define SYS_getpeername 354
#define SYS_socketpair 355
#define SYS_send 356
#define SYS_recv 357
#define SYS_sendto 358
#define SYS_recvfrom 359
#define SYS_shutdown 360
#define SYS_setsockopt 361
#define SYS_getsockopt 362
#define SYS_sendmsg 363
#define SYS_recvmsg 364

#include <syscall.h>

extern char *syscallTable[];

#endif /* SYSCALLS_H_ */
