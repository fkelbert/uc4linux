
// Daemonize headers
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <syslog.h>
#include <string.h>
// end daemonize headers


#include <unistd.h>
#include "logger.h"
#include "log_fhg_esf_loader_c.h"

#define DAEMON_NAME "fhg_esf_axis2cd"
#define PID_FILE "/var/run/esf_axis2cd.pid"

// install signal handler for a proper shutdown!

int main(unsigned int argc, char **argv)
{
  syslog(LOG_INFO, "%s daemon starting up", DAEMON_NAME);
  setlogmask(LOG_UPTO(LOG_DEBUG));
  //openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_USER);
  openlog(DAEMON_NAME, LOG_CONS | LOG_NDELAY | LOG_PERROR | LOG_PID, LOG_NEWS); // changed to news facility to separate from existing logging messages

  syslog(LOG_INFO, "starting the daemonizing process");

  /* Our process ID and Session ID */
  pid_t pid, sid;

  /* Fork off the parent process */
  pid = fork();
  if (pid < 0) exit(EXIT_FAILURE);

  /* If we got a good PID, then we can exit the parent process. */
  // Child can continue to run even after the parent has finished executing
  if (pid > 0) exit(EXIT_SUCCESS);

  /* Change the file mode mask */
  umask(0);

  /* Create a new SID for the child process */
  sid = setsid();
  if (sid < 0) {
    /* Log the failure */
    printf("setting sid failed!\n");
    exit(EXIT_FAILURE);
  }

  /* Change the current working directory */
  if ((chdir("/")) < 0) {
    /* Log the failure */
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
     sleep(30); /* wait 30 seconds */
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



// old main with direct axis invocation
//#include <axis2_svc_skeleton.h>
//#include <axutil_log_default.h>
//#include <axutil_error_default.h>
//#include <axutil_array_list.h>
//#include <axiom_text.h>
//#include <axiom_node.h>
//#include <axiom_element.h>
//#include <axiom.h>
//#include <axis2_util.h>
//#include <axutil_env.h>
//#include <axis2_http_server.h>
//#include <axis2_http_transport.h>



//axutil_env_t *system_env = NULL;
//axis2_transport_receiver_t *server = NULL;
//#define DEFAULT_REPO_PATH "/home/raindrop/hiwi/localaxis2c/"
//AXIS2_IMPORT extern int axis2_http_socket_read_timeout;
////AXIS2_IMPORT extern axis2_char_t *axis2_request_url_prefix;


//axutil_env_t *init_syetem_env(
//    axutil_allocator_t * allocator,
//    const axis2_char_t * log_file)
//{
//    axutil_error_t *error = axutil_error_create(allocator);
//    axutil_log_t *log = axutil_log_create(allocator, NULL, log_file);
//    /* if (!log) */
//
// /* 		  log = axutil_log_create_default (allocator); */
//    axutil_thread_pool_t *thread_pool = axutil_thread_pool_init(allocator);
//    /* We need to init the parser in main thread before spawning child threads
//    */
//    axiom_xml_reader_init();
//    return axutil_env_create_with_error_log_thread_pool(allocator, error, log,
//                                                        thread_pool);
//}
//

//int main(unsigned int argc, char **argv)
//{
//  axutil_log_levels_t log_level = AXIS2_LOG_LEVEL_DEBUG;
//  const axis2_char_t *log_file = "axis2.log";
//  const axis2_char_t *repo_path = DEFAULT_REPO_PATH;
//	unsigned int len;
//  int log_file_size = AXUTIL_LOG_FILE_SIZE;
//	unsigned int file_flag = 0;
//  int port = 9090;
//	axis2_status_t status;
//
//  axutil_allocator_t *allocator = NULL;
//  axutil_env_t *env = NULL;
//  allocator = axutil_allocator_init(NULL);
//
//  if (!allocator) exit(EXIT_FAILURE);
//
//  env = init_syetem_env(allocator, log_file);
//  env->log->level = log_level;
//  env->log->size = log_file_size;
//
//  axutil_error_init();
//  system_env = env;
//
//  AXIS2_LOG_INFO(env->log, "Starting Axis2 HTTP server....");
//  AXIS2_LOG_INFO(env->log, "Apache Axis2/C version in use : %s",axis2_version_string());
//  AXIS2_LOG_INFO(env->log, "Server port : %d", port);
//  AXIS2_LOG_INFO(env->log, "Repo location : %s", repo_path);
//  AXIS2_LOG_INFO(env->log, "Read Timeout : %d ms", axis2_http_socket_read_timeout);
//
//	status = axutil_file_handler_access (repo_path, AXIS2_R_OK);
//	if (status == AXIS2_SUCCESS)
//	{
//		len = (unsigned int)strlen (repo_path);
//        /* We are sure that the difference lies within the unsigned int range */
//		if ((len >= 9) && !strcmp ((repo_path + (len - 9)), "axis2.xml"))
//		{
//			file_flag = 1;
//		}
//	}
//	else
//	{
//		AXIS2_LOG_WARNING (env->log, AXIS2_LOG_SI, "provided repo path %s does "
//						   "not exist or no permissions to read, set "
//						   "repo_path to DEFAULT_REPO_PATH", repo_path);
//		repo_path = DEFAULT_REPO_PATH;
//	}
//
//
//  server = axis2_http_server_create(env, repo_path, port);
//
//  if (!server)
//  {
//      AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,
//                      "Server creation failed: Error code:" " %d :: %s",
//                      env->error->error_number,
//                      AXIS2_ERROR_GET_MESSAGE(env->error));
//      exit(EXIT_FAILURE);
//
//  }
//  printf("Started Simple Axis2 HTTP Server ...\n");
//  if(axis2_transport_receiver_start(server, env) == AXIS2_FAILURE)
//  {
//      AXIS2_LOG_ERROR(env->log, AXIS2_LOG_SI,"Server start failed: Error code:" " %d :: %s",env->error->error_number,AXIS2_ERROR_GET_MESSAGE(env->error));
//      exit(EXIT_FAILURE);
//  }
//  else log_trace("problem calling axis2_transport_receiver_start!\n");
//  log_trace("axis2 http server started\n");
//
//  return 0;
//}
