/**
 * @file  socketLinux.c
 * @brief Method definition header for socket communication in Linux
 *
 * @author cornelius moucha
 **/

#include "socketLinux.h"
#include "log_socketLinux_pef.h"

#ifndef __WIN32__

// ##############################################################
// Unix domain sockets
/*
pefSocket_ptr pefsocket_init(char *socketName, void *socketHandler)
{
  //pefSocket_ptr curSocket=pefSocketNew(socketName, socketHandler, -1);pefSocketServerNew
  pefSocket_ptr curSocket=pefSocketServerNew(PEFSOCKET_UNIXDOMAIN, socketName, socketHandler, -1);

  if((curSocket->s = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    log_error("Error creating socket");
    return NULL;
  }
  struct sockaddr_un saun;
  saun.sun_family = AF_UNIX;
  strcpy(saun.sun_path, curSocket->name);

  unlink(curSocket->name);   // remove existing bindings to address (only for development!)
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
  pthread_create(&attachingThread, NULL, (void*)pefsocket_listener, curSocket);
  return curSocket;
}

void pefsocket_listener(pefSocket_ptr curSocket)
{
  while(1)
  {
    int fromlen, ns;
    struct sockaddr_un fsaun;
    if((ns = accept(curSocket->s, (struct sockaddr*)&fsaun, &fromlen)) < 0)
    {
      log_error("Error accepting incoming connection");
      perror("Error accept");
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

    char *response=curSocket->handler(str);

    send(ns, response, strlen(response)+1, 0);
    free(str);
    free(response);
  }
}

// ##############################################################
// Unix domain socket client
pefSocket_ptr pefsocket_clientConnect(char *name)
{
  pefSocket_ptr curSocket=(pefSocket_ptr)mem_alloc(sizeof(pefSocket_t));
  checkNullPtr(curSocket, "Could not allocate memory for socket");

  curSocket->s=0;
  curSocket->name=(char*)mem_calloc(strlen(name)+1, sizeof(char));
  checkNull(curSocket->name, NULL, {free(curSocket);}, "Could not allocate memory for socket name");
  strncpy(curSocket->name, name, strlen(name)+1);
  curSocket->handler=NULL;

  struct sockaddr_un saun;
  if((curSocket->s=socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
  {
    log_error("Error creating socket");
    return NULL;
  }

  saun.sun_family=AF_UNIX;
  strcpy(saun.sun_path, curSocket->name);
  if(connect(curSocket->s, (struct sockaddr*)&saun, sizeof(saun.sun_family) + strlen(saun.sun_path)+1) < 0)
  {
    log_error("Error connecting to socket");
    return NULL;
  }
  log_info("Successfully connected to socket=[%s]", curSocket->name);
  return curSocket;
}

char *pefsocket_clientSend(pefSocket_ptr curSocket, char *msg)
{
  FILE *fp;
  fp=fdopen(curSocket->s, "r");
  send(curSocket->s, msg, strlen(msg)+1,0);

  // waiting for answer
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
*/

#endif //__WIN32__



/*
pefsocket_ptr pefsocket_clientNew(char *name)
{
  pefsocket_ptr nsocket=(pefsocket_ptr)mem_alloc(sizeof(pefsocket_t));
  checkNullPtr(nsocket, "Could not allocate memory for socket");

  nsocket->s=0;
  nsocket->socketName=(char*)mem_calloc(strlen(name)+1, sizeof(char));
  check_null(nsocket->socketName, NULL, {free(nsocket);}, "Could not allocate memory for socket name");
  strncpy(nsocket->socketName, name, strlen(name)+1);
  nsocket->handler=NULL;

  log_debug("pefClientsocket successfully allocated for socketName=[%s]", name);
  return nsocket;
}
*/
