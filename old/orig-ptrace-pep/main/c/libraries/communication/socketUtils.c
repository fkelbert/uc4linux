/**
 * @file  socketUtils.c
 * @brief Implementation for socket communications
 *
 * Provide a generic interface for socket communication, as in general socket communication is platform dependent.
 * @author cornelius moucha
 **/

#include "socketUtils.h"
#include "log_socketUtils_pef.h"


// ##############################################################
// Generic socket structure...
// internally called with socket initialization
pefSocket_ptr pefSocketServerNew(unsigned int type, const char *name, unsigned int port, PEFSOCKET_HANDLER handler)
{
  pefSocket_ptr nsocket=(pefSocket_ptr)memAlloc(sizeof(pefSocket_t));
  checkNullPtr(nsocket, "Could not allocate memory for socket structure");

  nsocket->s=0;
  nsocket->type=type;
  nsocket->socketThread=NULL;
  nsocket->handler=handler;

  if(type==PEFSOCKET_TCP)
  { // initialization of TCP socket
    nsocket->name=NULL;
    nsocket->port=port;
    nsocket->s=socketTCPserverInit(port);
    if(nsocket->s==-1) {log_error("Error creating socket!"); return NULL;}

    log_info("Opened TCP-socket (fd=[%d], port=[%d])", nsocket->s, port);
    if(listen(nsocket->s, 5))
    {
      log_error("Error listening to socket");
      return NULL;
    }
    nsocket->read=socketTCPread;
    nsocket->send=socketTCPsend;
    nsocket->clientSend=NULL;

    nsocket->socketThread=pefThreadNew(socketTCPlistener, nsocket);
    checkNullPtr(nsocket->socketThread, "Error instantiating new thread for socket listener!");

    log_debug("pefSocket successfully allocated for port=[%d] with handler=[%p]", port, handler);
    return nsocket;
  }
  else
  {
    // currently only TCP socket is supported!
    log_error("Currently only TCP sockets are supported!");
    return NULL;
    /*if(name!=NULL)
    {
      nsocket->name=(char*)memCalloc(strlen(name)+1, sizeof(char));
      checkNull(nsocket->name, NULL, {free(nsocket);}, "Could not allocate memory for socket name");
      strncpy(nsocket->name, name, strlen(name)+1);
    }
    if(name!=NULL) log_debug("pefSocket successfully allocated for socketName=[%s] with handler=[%p]", name, handler);
    */
  }

  return nsocket;
}

pefSocket_ptr pefSocketClientNew(unsigned int type, const char *name, unsigned int port)
{
  pefSocket_ptr curSocket=(pefSocket_ptr)memAlloc(sizeof(pefSocket_t));
  checkNullPtr(curSocket, "Could not allocate memory for socket structure");

  curSocket->s=0;
  curSocket->socketThread=NULL;

  if(type==PEFSOCKET_TCP)
  { // initialization of TCP client socket
    checkNullPtr(name, "Error: Require name (hostname) for connecting to TCP socket");

    curSocket->name=(char*)memCalloc(strlen(name)+1, sizeof(char));
    checkNull(curSocket->name, NULL, {free(curSocket);}, "Could not allocate memory for socket name");
    strncpy(curSocket->name, name, strlen(name)+1);
    curSocket->handler=NULL;
    curSocket->port=port;

    log_trace("Trying to connect to [%s] on port=[%d]", name, port);
    curSocket->s=socketTCPclientConnect(name, port);
    if(curSocket->s<0)
    {
      log_error("Error creating client TCP socket");
      free(curSocket->name);
      free(curSocket);
      return NULL;
    }
    curSocket->send=socketTCPsend;
    curSocket->read=socketTCPread;
    curSocket->clientSend=socketTCPresponseSend;

    log_info("Client TCP socket created; socket=[%d]", curSocket->s);
    return curSocket;
  }
  else
  {
    // currently only TCP socket is supported!
    log_error("Currently only TCP sockets are supported!");
    return NULL;
  }
}


unsigned int pefSocketFree(pefSocket_ptr socket)
{
  log_debug("stopping TCP-socket");
  if(socket->name!=NULL) free(socket->name);
  if(socket->socketThread!=NULL)
  {
    int ret=pefThreadKill(socket->socketThread);
    if(ret!=0) log_warn("Error terminating socket thread");
    ret=pefThreadFree(socket->socketThread);
    if(ret!=R_SUCCESS) log_warn("Error deallocating socket thread.");

  }
  log_debug("Socket Thread terminated");

  close(socket->s);
  free(socket);
  return R_SUCCESS;
}


