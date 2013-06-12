/*
 * shell_wrapper.c
 *      Author: Ricardo Neisse
 */

#include <unistd.h>
#include <string.h>
#include "socket_utils.h"
#include "log_shell_wrapper_c.h"

int main(int argc, char **argv)
{
  unsigned char *result;
  int errno;
  int pid = getpid();

  log_info("Starting monitored shell");
  log_debug(" - login [%s]", getlogin());
  log_debug(" - Process id [%d]", pid);
  log_debug(" - User id/effective id [%d/%d]", getuid(), geteuid());

  char data[10];
  snprintf(data, 10, "%d\0", pid);
  log_trace("Sending process id=[%s] to systracePEP",data);

  esfsocket_ptr lsocket=esfsocket_clientConnect("/tmp/systracePEP");
  log_info("Checking response syscall handler");
  result=esfsocket_clientSend(lsocket, data);

  if(strncmp(result, "success", 7))
  {
    log_error("Response from syscall handler not >success<, aborting shell session");
    exit(EXIT_FAILURE);
  }

  log_info("Running user shell [bash]");
  errno = execl("/usr/local/bin/bash", 0);
  log_info("bash returned with error code=[%d]", errno);
  return errno;
}

