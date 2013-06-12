#ifndef _UC4LINUX_SHELL_WRAPPER_H
#define _UC4LINUX_SHELL_WRAPPER_H

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <glib-object.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "logger.h"
#include "soup_utils.h"

#define SYSCALL_HANDLER_URI "http://127.0.0.1:8081/syscall_handler"
#define SYSCALL_HANDLER_METHOD "attach_process"
#define STARTUP_SHELL "/bin/bash"

#endif /* _UC4LINUX_SHELL_WRAPPER_H */
