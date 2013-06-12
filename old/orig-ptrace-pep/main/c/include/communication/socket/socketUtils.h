/**
 * @file  socketUtils.h
 * @brief Method defintions for socket communications
 *
 * Provide a generic interface for socket communication, as in general socket communication is platform dependent.
 * @author cornelius moucha
 **/


#ifndef SOCKETUTILS_H_
#define SOCKETUTILS_H_

#include <string.h>
#include "base.h"
#include "pefsocketType.h"

#ifdef __WIN32__
  #include "socketWin32.h"
#else
  #include "socketLinux.h"
#endif

/**
 * Instantiates a new server socket.
 * @param   type     desired socket type; TCP, Unix domain or named pipes
 * @param   name     name of the socket; required for unix domain sockets
 * @param   port     socket port; required for TCP socket
 * @param   handler  socket handling method
 * @return  the prepared socket structure on success or NULL otherwise
**/
pefSocket_ptr pefSocketServerNew(unsigned int type, const char *name, unsigned int port, PEFSOCKET_HANDLER handler);

/**
 * Instantiates a new client socket.
 * @param   type     desired socket type; TCP, Unix domain or named pipes
 * @param   name     host for connecting with
 * @param   port     port for connecting
 * @return  the prepared socket structure on success or NULL otherwise
**/
pefSocket_ptr pefSocketClientNew(unsigned int type, const char *name, unsigned int port);

/**
 * Deallocates an socket structure\n
 * This method deallocates the memory of an socket structure and terminates the associated socket handling thread.
 * @param   socket   socket structure for deallocation
 * @return  R_SUCCESS on success or R_ERROR otherwise
**/
unsigned int  pefSocketFree(pefSocket_ptr socket);


#endif /* SOCKETUTILS_H_ */



