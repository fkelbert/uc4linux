/**
 * @file  socketWin32.c
 * @brief Methods for socket communication in Windows
 *
 * @author cornelius moucha
 **/

#include "socketWin32.h"
#include "log_socketWin32_pef.h"

#ifdef __WIN32__

// ##############################################################
// TCP socket communication (reading, writing)
char *socketTCPread(int fd)
{
  log_debug("reading from socket...");

  char *msg=NULL;
  unsigned int msgLength=0;
  char *str=calloc(1024, sizeof(char));
  unsigned int terminate=0;
  int nRead=0;

  do
  {
    memset(str, 0, 1024);

    nRead=recv(fd, str, 1024, 0);
    if(nRead<0)
    {
      log_error("error reading socket(%d): %d\n\n\n", nRead, WSAGetLastError());
      closesocket(fd);
      WSACleanup();
      return NULL;
    }
    if(nRead==0)
    { // TODO: handling required!
      log_error("Remote peer closed the connection... terminating.");
      terminate=1;
      //break;
    }

    log_trace("Received [%d] bytes from socket=[%.40s]", nRead, str);
    msg=realloc(msg, (msgLength+nRead)*sizeof(char));
    memcpy( (msg+msgLength), str, nRead);
    msgLength+=nRead;
    log_debug("copied to msg-buffer ==> (msgLength=%d)", msgLength);

    /// @todo check for \0 -terminator!
    if(msg[msgLength-2]=='\r' && msg[msgLength-1]=='\n')
    {
      log_debug("found terminator symbols!! exiting...");
      msg[msgLength-2]='\0';
      break;
    }
    if(terminate==1) break;
  }
  while(nRead>0);

  if(terminate==1 && nRead==0 && msgLength==0)
  {
    log_warn("received empty message on socket?!?!");
    return NULL;
  }
  log_debug("finished reading from socket");
  return msg;
}

int socketTCPsend(int fd, char *msg)
{
  int sent=send(fd, msg, strlen(msg)+1,0); // including NULL-terminator
  if(sent==-1) log_error("Error sending msg=[%d]", WSAGetLastError());
  else
  {
    log_debug("send msg=[%d][%s] to socket", sent, msg);

    // check msg for \r\n
    if(msg[strlen(msg)-1]=='\r') log_trace("msg already contains \r");
    else
    {
      log_debug("no terminator present; send terminator symbol separately");
      int nSend=send(fd, "\r\n", 2, 0);
      if(nSend==SOCKET_ERROR)
      {
        log_error("Error sending response on socket: %d\n", WSAGetLastError());
        closesocket(fd);
        WSACleanup();
        return -1;
      }
    }
  }
  return sent;
}

// ##############################################################
// TCP socket server
int socketTCPserverInit(unsigned int port)
{
  int lSocket=INVALID_SOCKET;

  WSADATA wsaData;
  int iResult=WSAStartup(MAKEWORD(2,2), &wsaData);
  if(iResult!=0)
  {
    printf("WSAStartup failed with error: %d\n", iResult);
    return INVALID_SOCKET;
  }
  else log_debug("Winsock initialized!");

  struct addrinfo *result = NULL;
  struct addrinfo hints;
  ZeroMemory(&hints, sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  hints.ai_flags = AI_PASSIVE;

  char *portStr=malloc(6*sizeof(char));
  snprintf(portStr, 6, "%d", port);

  // Resolve the server address and port
  iResult = getaddrinfo(NULL, portStr, &hints, &result);
  if(iResult!=0)
  {
    log_error("getaddrinfo failed with error: %d\n", iResult);
    WSACleanup();
    return INVALID_SOCKET;
  }

  // Create a SOCKET for connecting to server
  lSocket=socket(result->ai_family, result->ai_socktype, result->ai_protocol);
  if(lSocket==INVALID_SOCKET)
  {
    log_error("socket failed with error: %ld\n", WSAGetLastError());
    freeaddrinfo(result);
    WSACleanup();
    return INVALID_SOCKET;
  }

  if(bind(lSocket, result->ai_addr, (int)result->ai_addrlen) < 0)
  {
    log_error("Error binding socket");
    freeaddrinfo(result);
    WSACleanup();
    return INVALID_SOCKET;
  }
  log_info("Successfully created win-TCP-socket for port=[%d]", port);

  return lSocket;
}

void socketTCPlistener(pefSocket_ptr curSocket)
{
  while(1)
  {
    int fromlen, ns;
    ns=accept(curSocket->s, NULL, NULL);
    if(ns<0) {log_error("Error accepting incoming connection");continue;}

    // only one active persistent connection (same as in Linux)
    char *msg=NULL;
    unsigned int nRead=0;
    while(1)
    {
      msg=socketTCPread(ns);
      if(msg==NULL) {log_warn("received empty msg => terminating connection."); break;}
      char *response=curSocket->handler(msg);

      int nSend=socketTCPsend(ns, response);
      if(nSend==SOCKET_ERROR)
      {
        log_error("Error sending response on socket: %d\n", WSAGetLastError());
        closesocket(ns);
        WSACleanup();
        return;
      }
      log_debug("Bytes sent: %d=[%s]\n", nSend, response);
    }
    closesocket(ns);
  }
}

// ##############################################################
// TCP socket client
int socketTCPclientConnect(const char *name, unsigned int port)
{
  WSADATA wsaData;
  SOCKET curSocket = INVALID_SOCKET;
  struct addrinfo *result = NULL, *ptr = NULL, hints;
  int iResult;

  // Initialize Winsock
  if(WSAStartup(MAKEWORD(2,2), &wsaData) != 0)
  {
    log_error("WSAStartup failed!\n");
    return INVALID_SOCKET;
  }

  ZeroMemory( &hints, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;

  char *portStr=malloc(6*sizeof(char));
  snprintf(portStr, 6, "%d", port);

  // Resolve the server address and port
  iResult = getaddrinfo(name, portStr, &hints, &result);
  if(iResult != 0 )
  {
    log_error("Getaddrinfo failed with error: %d", iResult);
    WSACleanup();
    return INVALID_SOCKET;
  }

  // Attempt to connect to an address until one succeeds
  for(ptr=result; ptr!=NULL;ptr=ptr->ai_next)
  {
    curSocket=socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);
    if(curSocket==INVALID_SOCKET)
    {
      log_error("Error creating socket: %ld", WSAGetLastError());
      WSACleanup();
      return 1;
    }

    iResult=connect(curSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if(iResult == SOCKET_ERROR)
    {
      closesocket(curSocket);
      curSocket=INVALID_SOCKET;
      continue;
    }
    break;
  }
  freeaddrinfo(result);

  if(curSocket==INVALID_SOCKET)
  {
    log_error("Error connecting to socket");
    WSACleanup();
    return INVALID_SOCKET;
  }
  log_info("Successfully connected to socket=[%s] on port=[%d]", name, port);
  return curSocket;
}

char *socketTCPresponseSend(pefSocket_ptr curSocket, char *request)
{
  int ret=socketTCPsend(curSocket->s, request);
  log_debug("Finished sending msg to socket (ret=[%d]); waiting for answer...", ret);

  char *msg=socketTCPread(curSocket->s);
  log_debug("Received response-msg=[%s]", msg);
  return msg;
}


#endif // __WIN32__
