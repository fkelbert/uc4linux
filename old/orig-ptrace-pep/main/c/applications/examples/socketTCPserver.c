/**
 * @file  socketTCPserver.c
 * @brief Exemplary server for socket communication over TCP socket
 *
 * @author cornelius moucha
**/

#include "socketUtils.h"
#include "log_socketTCPserver_pef.h"

unsigned char *data="testback\r\n";

char *handler(char *msg)
{
  log_trace("received msg=[%s]", msg);
  char *response=memAlloc(11*sizeof(char));
  snprintf(response,11, "%s", data);
  log_debug("returning response=[%s]", response);
  return response;
}

int main()
{
  logInit();

  log_info("Starting TCP socket server");
  unsigned char *result;
  pefSocket_ptr lsocket=pefSocketServerNew(PEFSOCKET_TCP, NULL, 10001, &handler);
  if(lsocket==NULL)
  {
    log_error("Error creating socket!");
    return 1;
  }
  log_info("socket created", result);
  getchar();

  return 0;
}


