/*
 * shell_wrapper.c
 *
 *      Author: Ricardo Neisse
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "logger.h"
#include "soup_utils.h"

int main (int argc, char* argv) {
  SoupSession *session;
  char *service_uri = "http://127.0.0.1:8081/syscall_handler";
  char *method = "attach_process";
  GValue g_return_value;
  char *result;
  int errno;
  int pid = getpid();
  
  plog(LOG_INFO, "Starting monitored shell");
  plog(LOG_DEBUG, " - login [%s]",getlogin());
  plog(LOG_DEBUG, " - Process id [%d]",pid);
  plog(LOG_DEBUG, " - User id/effective id [%d/%d]",getuid(), geteuid());

  plog(LOG_TRACE, "Initializing Glib");
  g_thread_init (NULL);
  g_type_init ();
  g_set_prgname ("shell_wrapper");  
  plog(LOG_TRACE, "Creating SoupSession");
  session = soup_session_sync_new();

  plog(LOG_TRACE, "Converting process id to string");
  char data[10];
  sprintf(data,"%d",pid);

  plog(LOG_TRACE, "Sending process id to syscall handler");
  if (!do_xmlrpc (session, service_uri, method, &g_return_value, G_TYPE_STRING, data, G_TYPE_INVALID)) {
    plog(LOG_ERROR, "Syscall handler not available, aborting user shell session");
    exit(1);
  }

  plog(LOG_TRACE, "Checking response syscall handler");
  if (!check_xmlrpc (&g_return_value, G_TYPE_STRING, &result)) {
    plog(LOG_ERROR, "Protocol error in response from syscall handler, aborting shell session");
    exit(1);
  }

  if (strcmp (result, "success")) {
    plog(LOG_ERROR, "Response from syscall handler not success, aborting shell session");
    exit(1);
  }

  plog(LOG_INFO, "Running user shell [bash]");
  errno = execl("/usr/local/bin/bash",0);
  
  plog(LOG_ERROR, "Error number is %d", errno);

  return 0;
}

