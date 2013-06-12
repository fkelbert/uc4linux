/**
 * @file  socketTCPclient.c
 * @brief Exemplary client for socket communication over TCP socket
 *
 * @author cornelius moucha
**/

#include "socketUtils.h"
#include "log_socketTCPclient_pef.h"

int main()
{
  logInit();
  log_info("Starting communication test using TCP socket");

  char *result;
  char *data="hallo test\r\n";

  pefSocket_ptr lsocket=pefSocketClientNew(PEFSOCKET_TCP, "localhost", 9983);
  if(lsocket==NULL) {log_error("Could not create socket."); return 1;}
  getchar();

  while(1)
  {
    result=lsocket->clientSend(lsocket, data);
    log_info("received response=[%s]", result);
    if(result==NULL) break;
    getchar();
  }
  log_debug("terminating...");

  return 0;
}


