/**
 * @file  socketLinuxTCP.c
 * @brief Implementation for TCP socket communication in Linux
 *
 * @author cornelius moucha
 **/

#include "socketLinux.h"
#include "log_socketLinuxTCP_pef.h"

#ifndef __WIN32__

// ##############################################################
// TCP socket communication (reading, writing)
int socketTCPsend(int fd, char *msg)
{
  return send(fd, msg, strlen(msg)+1, 0);
}

int socketTCPreadNative(int fd, char *str, size_t bufferLength)
{
  return read(fd, str, bufferLength);
}

char *socketTCPread(int fd)
{
  char *msg=NULL;
  unsigned int msgLength=0;
  char *str=calloc(1024, sizeof(char));
  unsigned int terminate=0;
  int nRead=0;

  do
  {
    memset(str, 0, 1024);
    nRead=socketTCPreadNative(fd, str, 4096);

    if(nRead==-1) {perror("Error reading"); return NULL;}
    if(nRead==0)
    {
      log_error("Remote peer closed the connection... terminating.");
      terminate=1;
      break;
    }
    log_trace("Received [%d] bytes from socket=[%.40s]", nRead, str);

    // Concatenate buffer and received bytes
    msg=realloc(msg, (msgLength+nRead)*sizeof(char));
    memcpy( (msg+msgLength), str, nRead);
    msgLength+=nRead;
    log_debug("Copied to msg-buffer ==> (msgLength=%d)", msgLength);

    unsigned int addNullTerminator=0;
    if(msg[msgLength-1]=='\0') addNullTerminator=1;

    //int xa;
    //for(xa=0; xa<msgLength; xa++)
    //  printf("%x ", str[xa]);
    //printf("\n# %x # %x #\n", msg[msgLength-2-addNullTerminator], msg[msgLength-1-addNullTerminator]);
    //fflush(stdout);

    // check for terminator symbols (\r\n)
    if(msg[msgLength-2-addNullTerminator]=='\r' && msg[msgLength-1-addNullTerminator]=='\n')
    {
      log_debug("found terminator symbols!! exiting...");
      msg[msgLength-2]='\0';
      break;
    }
    if(terminate==1) break;
  }
  while(nRead>0);
  if(str!=NULL) free(str);

  if(terminate==1 && nRead==0 && msgLength==0)
  {
    log_warn("received empty message on socket?!?!");
    return NULL;
  }
  log_debug("finished reading from socket");
  return msg;
}

// ##############################################################
// TCP server socket
int socketTCPserverInit(unsigned int port)
{
  int lsocket=-1;
  if((lsocket=socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    log_error("Error creating TCP server socket");
    return INVALID_SOCKET;
  }
  log_trace("TCP server socket created");

  struct sockaddr_in sockInet;
  memset(&sockInet, 0, sizeof(struct sockaddr_in));

  sockInet.sin_addr.s_addr=INADDR_ANY;
  sockInet.sin_port=htons(port);
  sockInet.sin_family=AF_INET;

  if(bind(lsocket, (struct sockaddr*)&sockInet, sizeof(sockInet)) < 0)
  {
    log_error("Error binding TCP server socket");
    return INVALID_SOCKET;
  }
  log_debug("TCP server socket port=[%d] successfully initialized", port);
  return lsocket;
}

void socketTCPlistener(pefSocket_ptr curSocket)
{
  socklen_t fromlen;
  struct sockaddr_in fsaun;

  while(1)
  {  // main accept() loop
    fromlen = sizeof(fsaun);
    int ns=0;
    log_debug("waiting for incoming connections...");
    if((ns = accept(curSocket->s, (struct sockaddr *)&fsaun, &fromlen)) == -1)
    {
      perror("accept");
      continue;
    }
    log_trace("TCP server: got connection from %s", inet_ntoa(fsaun.sin_addr));
    /// @todo handling of more than one active connection! (currently one active connection blocks everything)
    /// @todo ==> move reading to separate thread?

    char *msg=NULL;
    unsigned int nRead=0;
    while(1)
    {
      msg=socketTCPread(ns);

      if(msg==NULL) {log_warn("received empty msg => terminating connection."); break;}

      if(curSocket->handler==NULL)
      {
        log_error("ERROR: no socket handler assigned?!?!");
        return;
      }
      char *response=curSocket->handler(msg);
      free(msg);
      msg=NULL;

      log_trace("sending response=[%s]...", response);
      /// @todo check response before sending it?
      /// @todo check whether response was completely sent
      int nSend=socketTCPsend(ns, response);
      log_debug("Bytes sent: %d=[%s]\n", nSend, response);
      if(response!=NULL) free(response);
      if(nSend==-1)
      {
        log_error("Error sending response on socket: %d\n", nSend);
        continue;
      }
    }
    close(ns);
  }
}

// ##############################################################
// TCP socket client
int socketTCPclientConnect(const char *name, unsigned int port)
{
  struct sockaddr_un saun;
  int curSocket;
  if((curSocket=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    log_error("Error creating socket");
    return -1;
  }
  log_trace("Successfully created client TCP socket");

  // get IP address from name
  struct hostent* hostinfo=gethostbyname(name);
  long hostaddress;
  memcpy(&hostaddress, hostinfo->h_addr, hostinfo->h_length);

  struct sockaddr_in sockInet;
  sockInet.sin_addr.s_addr=hostaddress;
  sockInet.sin_port=htons(port);
  sockInet.sin_family=AF_INET;

  if(connect(curSocket, (struct sockaddr*)&sockInet, sizeof(struct sockaddr_in)) < 0)
  {
    log_error("ERROR connecting to socket(port=[%d], name=[%s]", port, name); // connection refused if no one listens
    //perror("connecting error");
    return -1;
  }
  log_info("Successfully connected to TCP socket=[%s], port=[%d]", name, port);
  return curSocket;
}

char *socketTCPresponseSend(pefSocket_ptr curSocket, char *request)
{
  int ret=socketTCPsend(curSocket->s, request);
  log_debug("Finished sending msg to socket (ret=[%d]); waiting for answer...", ret);

  if(request[strlen(request)-1]=='\r') log_trace("Request already contains \r");
  else
  {
    log_debug("no terminator present; send terminator symbol separately");
    ret=socketTCPsend(curSocket->s, "\r\n");
    if(ret==-1) log_error("Error sending response on socket\n");
  }

  char *msg=curSocket->read(curSocket->s);
  log_debug("Received response-msg=[%s]", msg);
  return msg;
}

#endif
