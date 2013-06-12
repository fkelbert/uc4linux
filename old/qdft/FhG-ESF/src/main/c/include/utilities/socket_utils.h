/*
 * socket_utils.h
 *
 *  Created on: Jul 29, 2011
 *      Author: cornelius moucha
 */

#ifndef SOCKET_UTILS_H_
#define SOCKET_UTILS_H_

#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "base.h"

typedef struct esfsocket_s
{
  int s;                                            // socket file descriptor
  unsigned char* socketName;                        // socket name (for domain)
  unsigned int   port;                              // socket port (for TCP)
  unsigned char * (*handler)(unsigned char *msg);   // handler method, called after receiving msg
} esfsocket_t;
typedef esfsocket_t *esfsocket_ptr;

esfsocket_ptr esfsocket_new(unsigned char *name, unsigned char *(*handler)(unsigned char *), int port);
esfsocket_ptr initSocket(unsigned char *socketName, void *socketHandler);
esfsocket_ptr initTCPSocket(int port, void *socketHandler);
void socketListener(esfsocket_ptr curSocket);
esfsocket_ptr esfsocket_clientConnect(unsigned char *name);
esfsocket_ptr esfsocket_clientTCPConnect(unsigned char *name, unsigned int port);
unsigned char *esfsocket_clientSend(esfsocket_ptr curSocket, unsigned char *msg);
unsigned char *esfsocket_clientTCPSend(esfsocket_ptr curSocket, unsigned char *msg);

#endif /* SOCKET_UTILS_H_ */
