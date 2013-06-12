/**
 * @file  socketWin32.h
 * @brief Method definition header for socket communication in Windows
 *
 * @author cornelius moucha
 **/

#ifndef SOCKETWIN32_H_
#define SOCKETWIN32_H_

#ifdef __WIN32__

// windows socket error codes
// 10038 - WSAENOTSOCK       - Socket operation on nonsocket.
// 10053 - WSAECONNABORTED   - Software caused connection abort.
// 10054 - WSAECONNRESET     - Connection reset by peer.
// 10093 - WSANOTINITIALISED - Successful WSAStartup not yet performed.

#ifdef __WIN32__
  #ifndef _WIN32_WINNT
    #define _WIN32_WINNT 0x0501 // telling I'm using windows XP, otherwise method getaddrinfo not available!
  #endif

  #include <winsock2.h>
  #include <ws2tcpip.h>
#endif

#include "base.h"
#include "pefsocketType.h"

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
char *socketTCPresponseSend(pefSocket_ptr curSocket, char *request);

/**
 * Internal method for reading from the socket\n
 * Read until the final terminator sequence \r\n is retrieved
 * @param   fd    the file descriptor for reading
 * @return  the message retrieved from the socket
**/
char *socketTCPread(int fd);

/**
 * Internal method for writing to the socket\n
 * Send the given message and the final terminator sequence \r\n
 * if not already present in message
 * @param   fd    the file descriptor for writing
 * @return  the amount of characters sent to the socket
**/
int   socketTCPsend(int fd, char *msg);

#endif

#endif /* SOCKETWIN32_H_ */




