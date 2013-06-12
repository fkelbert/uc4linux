// daemonization headers
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
// end daemonization headers

#include <unistd.h>
#include "logger.h"

#define DAEMON_NAME "fhg_esf_axis2cd"
#define PID_FILE    "/var/run/esf_axis2cd.pid"

// install signal handler for a proper shutdown!
int main(unsigned int argc, char **argv)
{
  syslog(LOG_INFO, "%s daemon starting up", DAEMON_NAME);
  setlogmask(LOG_UPTO(LOG_DEBUG));
  //openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
  openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_NEWS); // changed to news facility to separate from existing logging messages

  syslog(LOG_INFO, "starting daemonizing process");

  /* Our process ID and Session ID */
  pid_t pid, sid;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) exit(EXIT_FAILURE);
  if (pid > 0) exit(EXIT_SUCCESS);

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();
  if(sid < 0)
  {
    printf("setting sid failed!\n");
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory */
  if ((chdir("/")) < 0)
  {
    printf("changing dir failed!\n");
    exit(EXIT_FAILURE);
  }

  syslog(LOG_INFO, "starting axis http server");
  /* Daemon-specific initialization goes here */
  char *cmd[] = { "./axis2_http_server", (char *)0 };
  char *myenv[]={"AXIS2C_HOME=/home/raindrop/hiwi/prog/FhG-ESF/target/linux/x86/axis2c",(char *)0 };
  int ret=execve("./axis2_http_server",cmd,myenv);

  /* Close out the standard file descriptors */
  //Because daemons generally dont interact directly with user so there is no need of keeping these open
  close(STDIN_FILENO);
  close(STDOUT_FILENO);
  close(STDERR_FILENO);

  /* An infinite loop */
  while (1)
  {
     /* Do some task here ... */
     sleep(30);
  }
  exit(EXIT_SUCCESS);

  syslog(LOG_INFO, "%s daemon exiting", DAEMON_NAME);
  return 0;
}

// working main without daemonizing
//int main(unsigned int argc, char **argv)
//{
//  printf("Starting axis2c chainload...\n");
//  char *cmd[] = { "./axis2_http_server", (char *)0 };
//  char *myenv[]={"AXIS2C_HOME=/home/raindrop/hiwi/prog/FhG-ESF/target/linux/x86/axis2c",(char *)0 };
//  int ret=execve("./axis2_http_server",cmd,myenv);
//
//  //int ret=execv("./axis2_http_server",cmd);
//
//  return 0;
//}
