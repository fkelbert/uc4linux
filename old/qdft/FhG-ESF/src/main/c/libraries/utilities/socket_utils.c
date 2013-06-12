/*
 * socket_utils.c
 *
 *  Created on: Jul 29, 2011
 *      Author: cornelius moucha
 */

#include "socket_utils.h"
#include "log_socket_utils_c.h"

void socketListener(esfsocket_ptr curSocket)
{
  while(1)
  {
    int fromlen, ns;
    struct sockaddr_un fsaun;
    if((ns = accept(curSocket->s, (struct sockaddr*)&fsaun, &fromlen)) < 0)
    {
      log_error("Error accepting incoming connection");
      return;
    }

    FILE *fp=fdopen(ns, "r");
    int xi=0;
    char c;
    char *str=malloc(20*sizeof(char));
    while( (c = fgetc(fp))!=EOF)
    {
      str[xi++]=c;
      if(c=='\0') break;
    }
    log_trace("ok, received from socket=[%s]\n", str);

    unsigned char *response=curSocket->handler(str);
    send(ns, response, strlen(response)+1, 0);
    free(str);
    free(response);    
  }
}

esfsocket_ptr initSocket(unsigned char *socketName, void *socketHandler)
{
  esfsocket_ptr curSocket=esfsocket_new(socketName, socketHandler, -1);

  if((curSocket->s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    log_error("Error creating socket");
    return NULL;
  }
  struct sockaddr_un saun;
  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, curSocket->socketName);

  unlink(curSocket->socketName);   // remove existing bindings to address (only for development!)
  //if(bind(curSocket->s, &saun, sizeof(saun.sun_family) + strlen(saun.sun_path)+1) < 0)
  if(bind(curSocket->s, (struct sockaddr*)&saun, SUN_LEN(&saun)) < 0)
  {
    log_error("Error binding socket");
    return NULL;
  }

  if(chmod(socketName, S_IRWXU | S_IRWXG | S_IRWXO)<0)
  {
    log_error("Adjusting socket permissions failed");
    return NULL;
  }

  if(listen(curSocket->s, 5))
  {
    log_error("Error listening to socket");
    return NULL;
  }

  pthread_t attachingThread;
  pthread_create(&attachingThread, NULL, (void*)socketListener, curSocket);
  return curSocket;
}

esfsocket_ptr esfsocket_new(unsigned char *name, unsigned char *(*handler)(unsigned char *), int port)
{
  esfsocket_ptr nsocket=(esfsocket_ptr)mem_alloc(sizeof(esfsocket_t));
  check_null_ptr(nsocket, "Could not allocate memory for socket");

  nsocket->s=0;
  if(name!=NULL)
  {
    nsocket->socketName=(unsigned char*)mem_calloc(strlen(name)+1, sizeof(unsigned char));
    check_null(nsocket->socketName, NULL, {free(nsocket);}, "Could not allocate memory for socket name");
    strncpy(nsocket->socketName, name, strlen(name)+1);
  }
  else
  {
    nsocket->socketName=NULL;
    nsocket->port=port;
  }

  nsocket->handler=handler;
  if(name!=NULL) log_debug("esfsocket successfully allocated for socketName=[%s] with handler=[%p]", name, handler);
  else           log_debug("esfsocket successfully allocated for port=[%d] with handler=[%p]", port, handler);
  return nsocket;
}

esfsocket_ptr esfsocket_clientNew(unsigned char *name)
{
  esfsocket_ptr nsocket=(esfsocket_ptr)mem_alloc(sizeof(esfsocket_t));
  check_null_ptr(nsocket, "Could not allocate memory for socket");

  nsocket->s=0;
  nsocket->socketName=(unsigned char*)mem_calloc(strlen(name)+1, sizeof(unsigned char));
  check_null(nsocket->socketName, NULL, {free(nsocket);}, "Could not allocate memory for socket name");
  strncpy(nsocket->socketName, name, strlen(name)+1);
  nsocket->handler=NULL;

  log_debug("esfClientsocket successfully allocated for socketName=[%s]", name);
  return nsocket;
}

esfsocket_ptr esfsocket_clientConnect(unsigned char *name)
{
  esfsocket_ptr curSocket=(esfsocket_ptr)mem_alloc(sizeof(esfsocket_t));
  check_null_ptr(curSocket, "Could not allocate memory for socket");

  curSocket->s=0;
  curSocket->socketName=(unsigned char*)mem_calloc(strlen(name)+1, sizeof(unsigned char));
  check_null(curSocket->socketName, NULL, {free(curSocket);}, "Could not allocate memory for socket name");
  strncpy(curSocket->socketName, name, strlen(name)+1);
  curSocket->handler=NULL;

  struct sockaddr_un saun;
  if((curSocket->s=socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    log_error("Error creating socket");
    return NULL;
  }

  saun.sun_family=AF_UNIX;
  strcpy(saun.sun_path, curSocket->socketName);
  if(connect(curSocket->s, (struct sockaddr*)&saun, sizeof(saun.sun_family) + strlen(saun.sun_path)+1) < 0)
  {
    log_error("Error connecting to socket");
    return NULL;
  }
  log_info("Successfully connected to socket=[%s]", curSocket->socketName);
  return curSocket;
}

unsigned char *esfsocket_clientSend(esfsocket_ptr curSocket, unsigned char *msg)
{
  FILE *fp;
  fp=fdopen(curSocket->s, "r");
  send(curSocket->s, msg, strlen(msg)+1,0);

  int xi=0;
  char c;
  char *str=malloc(20*sizeof(char));

  while( (c = fgetc(fp))!=EOF)
  {
    str[xi++]=c;
    if(c=='\0') break;
  }
  log_trace("ok, received from socket=[%s]", str);
  fclose(fp);
  return str;
}

void socketTCPListener(esfsocket_ptr curSocket)
{
  while(1)
  {
    int fromlen, ns;
    struct sockaddr_un fsaun;
    if((ns = accept(curSocket->s, (struct sockaddr*)&fsaun, &fromlen)) < 0)
    {
      log_error("Error accepting incoming connection");
      return;
    }

    char str[1024];
    read(ns,str,1024);
    log_trace("ok, received from socket=[%s]\n", str);

    unsigned char *response=curSocket->handler(str);
    write(ns, response, strlen(response)+1);
  }
}

esfsocket_ptr initTCPSocket(int port, void *socketHandler)
{
  esfsocket_ptr curSocket=esfsocket_new(NULL, socketHandler, port);

  if((curSocket->s = socket(AF_INET, SOCK_STREAM, 0)) < 0)
  {
    log_error("Error creating socket");
    return NULL;
  }

  struct sockaddr_in sockInet;
  sockInet.sin_addr.s_addr=INADDR_ANY;
  sockInet.sin_port=htons(port);
  sockInet.sin_family=AF_INET;

  if(bind(curSocket->s, (struct sockaddr*)&sockInet, sizeof(struct sockaddr_in)) < 0)
  {
    log_error("Error binding socket");
    return NULL;
  }
  log_info("Opened TCP-socket (fd=[%d], port=[%d])", curSocket->s, port);

  if(listen(curSocket->s, 5))
  {
    log_error("Error listening to socket");
    return NULL;
  }

  pthread_t socketThread;
  pthread_create(&socketThread, NULL, (void*)socketTCPListener, curSocket);
  return curSocket;
}

esfsocket_ptr esfsocket_clientTCPConnect(unsigned char *name, unsigned int port)
{
  esfsocket_ptr curSocket=(esfsocket_ptr)mem_alloc(sizeof(esfsocket_t));
  check_null_ptr(curSocket, "Could not allocate memory for socket");

  curSocket->s=0;
  curSocket->socketName=(unsigned char*)mem_calloc(strlen(name)+1, sizeof(unsigned char));
  check_null(curSocket->socketName, NULL, {free(curSocket);}, "Could not allocate memory for socket name");
  strncpy(curSocket->socketName, name, strlen(name)+1);
  curSocket->handler=NULL;
  curSocket->port=port;

  struct sockaddr_un saun;
  if((curSocket->s=socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
  {
    log_error("Error creating socket");
    return NULL;
  }

  /* get IP address from name */
  struct hostent* hostinfo=gethostbyname(name);
  long hostaddress;
  memcpy(&hostaddress, hostinfo->h_addr, hostinfo->h_length);

  struct sockaddr_in sockInet;
  sockInet.sin_addr.s_addr=hostaddress;
  sockInet.sin_port=htons(port);
  sockInet.sin_family=AF_INET;

  if(connect(curSocket->s, (struct sockaddr*)&sockInet, sizeof(struct sockaddr_in)) < 0)
  {
    log_error("Error connecting to socket");
    return NULL;
  }
  log_info("Successfully connected to socket=[%s]", curSocket->socketName);
  return curSocket;
}

unsigned char *esfsocket_clientTCPSend(esfsocket_ptr curSocket, unsigned char *msg)
{
  //FILE *fp;
  //fp=fdopen(curSocket->s, "r");
  send(curSocket->s, msg, strlen(msg)+1,0);

  //char str[1024];
  char *str=(char*)mem_alloc(1024*sizeof(char));
  read(curSocket->s,str,1024);
  log_trace("ok, received from socket=[%s]\n", str);
  return str;
}

