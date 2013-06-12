/*
 * pdpTCPsocketTest.c
 *
 *  Created on: Aug 18, 2011
 *      Author: cornelius moucha
 */

#include "socket_utils.h"
#include "log_pdpTCPsocketTest_c.h"

int main()
{
  log_info("Starting PDP communication test using TCP socket");

  unsigned char *result;
  int errno;

  unsigned char *data="hallo test";
  esfsocket_ptr lsocket=esfsocket_clientTCPConnect("localhost",9876);
  result=esfsocket_clientTCPSend(lsocket, data);

  log_info("received response=[%s]", result);

  return 0;
}


