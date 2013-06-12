/**
 * @file  socketLinux.h
 * @brief Method definition header for socket communication in Linux
 *
 * @author cornelius moucha
 **/

#ifndef SOCKETLINUX_H_
#define SOCKETLINUX_H_

#ifndef __WIN32__

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <stdio.h>          // for EOF-symbol...
#include <unistd.h>         // for unlink
#include <arpa/inet.h>      // for inet_ntoa
#include "base.h"
#include "memUtils.h"
#include "pefsocketType.h"

#define INVALID_SOCKET -1





/**
 * Initialize a new server for TCP socket
 * @param   port    the port for listening
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
int   socketTCPserverInit(unsigned int port);

/**
 * Listener method for TCP socket server\n
 * Accepts incoming connections, reads the message and forwards it
 * to the associated handler method specified in the socket structure.
 * Finally the response is sent back to the communication partner.
 * @param   curSocket   reference to the socket structure
**/
void  socketTCPlistener(pefSocket_ptr curSocket);

/**
 * Initialize a new client for TCP socket
 * @param   name    the hostname for the connection
 * @param   port    the port for the connection
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
int   socketTCPclientConnect(const char *name, unsigned int port);

/**
 * Socket client communication\n
 * Send the given message to the communication partner and return
 * the retrieved response
 * @param   curSocket reference to the socket structure
 * @param   msg       message to send
 * @return  the retrieved response
**/
char *socketTCPresponseSend(pefSocket_ptr curSocket, char *msg);

/**
 * Internal method for reading from the socket\n
 * Read until the final terminator sequence \\r\\n is retrieved
 * @param   fd    the file descriptor for reading
 * @return  the message retrieved from the socket
**/
char *socketTCPread(int fd);

/**
 * Internal method for writing to the socket\n
 * Send the given message and the final terminator sequence \\r\\n
 * if not already present in message
 * @param   fd    the file descriptor for writing
 * @param   msg   the message to be sent
 * @return  the amount of characters sent to the socket
**/
int   socketTCPsend(int fd, char *msg);

/**
 * Internal method for reading from the socket\n
 * Read from the socket into the prepared buffer at most up to the
 * given buffer size
 * @param   fd            the file descriptor for reading
 * @param   str           prepared message buffer
 * @param   bufferLength  size of the message buffer
 * @return  the amount of characters read from the socket
**/
int   socketTCPreadNative(int fd, char *str, size_t bufferLength);


#endif

#endif /* SOCKETLINUX_H_ */



