#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "log_server_c.h"

#include "base.h"


typedef struct msg_s {
  int length;
  char *content;
} msg_t;
typedef msg_t* msg_ptr;

typedef struct socket_s {
  // file path
  char *path; 
  // socket descriptor
  int sd; 
  // socket address
  struct sockaddr_un addr;
  // address length
  int addr_len;
  // state flag
  bool is_connected;
  // state flag
  bool is_binded;
} socket_t;
typedef socket_t* socket_ptr;

typedef struct server_s {
  // socket
  socket_ptr socket;
  // state flag
  bool is_listening;
  // listening thread
  pthread_t listening_thread; 
  // callback for request processing
  msg_ptr (*callback)(msg_ptr);
} server_t;
typedef server_t* server_ptr;

typedef struct client_s {
  // socket
  socket_ptr socket;
} client_t;
typedef client_t* client_ptr;

typedef struct handle_client_args_s {
  server_ptr server;
  socket_ptr csocket;
} handle_client_args_t;
typedef handle_client_args_t* handle_client_args_ptr;

void socket_free(socket_ptr socket) {
  log_trace("Freeing socket [%s]", socket->path);
  if (socket->path!=NULL) {
    free(socket->path);
  }
  if (socket->is_connected) {
    close (socket->sd);
  }
  free(socket);
}

msg_ptr pdp_process(msg_ptr request);

unsigned int socket_connect(socket_ptr socket) {
  if (connect(socket->sd, (struct sockaddr *)&socket->addr, socket->addr_len) == -1) {
    log_error("Error connecting to socket [%s]", socket->path);
    return R_ERROR;
  }
  log_trace("Socket [%s] connected", socket->path);
  socket->is_connected = TRUE;
  return R_SUCCESS;
}

unsigned int socket_bind(socket_ptr socket) {
    log_trace("Binding socket [%s]", socket->path);
    // when binding delete file
    unlink(socket->addr.sun_path); 
    if (bind(socket->sd, (struct sockaddr *)&socket->addr, socket->addr_len) == -1) {
        log_error("Error binding socket [%s]", socket->path);
        return R_ERROR;
    }
    socket->is_binded = TRUE;
    return R_SUCCESS;
}

unsigned int socket_create_endpoint(socket_ptr asocket) {
  log_trace("Creating socket endpoint [%s]", asocket->path);
  if (asocket->path == NULL) {
    log_error("Can't create socket endpoint for NULL path");
  }
  if ((asocket->sd = socket(AF_UNIX, SOCK_STREAM, 0)) == -1) {
    log_error("Error creating socket endpoint");
    return R_ERROR;
  }
  asocket->addr.sun_family = AF_UNIX;
  strcpy(asocket->addr.sun_path, asocket->path);
  asocket->addr_len = strlen(asocket->addr.sun_path) + sizeof(asocket->addr.sun_family) + 1;
}

msg_ptr msg_new(char *text) {
  msg_ptr msg = malloc(sizeof(msg_t));
  // Include NULL terminator
  if (text != NULL) {
    msg->length = strlen (text) + 1; 
    msg->content = malloc(sizeof(char) * msg->length);
    memcpy(msg->content, text, msg->length);
  } else {
    msg->length=0;
    msg->content=NULL;
  }
  return msg;
}

void msg_free(msg_ptr msg) {
  log_trace("Freeing message length=[%d]", msg->length);
  if (msg->content != NULL) {
    free(msg->content);
  }
  free(msg);
}

socket_ptr socket_new(char *path) {
  socket_ptr asocket;
  log_trace("Creating socket [%s]", path);
  asocket = (socket_ptr) malloc(sizeof(socket_t));
  if (path!=NULL) {
    asocket->path = strdup(path);
  }
  asocket->is_connected = FALSE;
  asocket->is_binded = FALSE;
  asocket->sd = -1;
  return asocket;
}

void server_free(server_ptr server) {
  log_trace("Freeing server [%s]", server->socket->path);
  socket_free(server->socket);
  free(server);
}

void client_free(client_ptr client) {
  log_trace("Freeing client [%s]", client->socket->path);
  socket_free(client->socket);
  free(client);
}

void socket_write_msg(socket_ptr asocket, msg_ptr msg) {
  write(asocket->sd, &msg->length, sizeof (msg->length));
  write(asocket->sd, msg->content, msg->length);
}

msg_ptr socket_read_msg(socket_ptr asocket) {
  msg_ptr msg = msg_new(NULL);
  // Read the length of the text message from the socket. 
  // If read returns zero, the client closed the connection.
  if (read (asocket->sd, &msg->length, sizeof (msg->length)) == 0) {
    return 0;
  }  
  // Allocate content
  msg->content = (char*) malloc(sizeof(char) * msg->length);
  // Read content
  read (asocket->sd, msg->content, msg->length);
  return msg;
}

/*
  Request/response message exchange
  Client sends request message and 
  expects response message from server
*/
msg_ptr socket_msg_exchange(socket_ptr asocket, msg_ptr request) {
  msg_ptr response;
  socket_connect(asocket);
  socket_write_msg(asocket, request);
  response = socket_read_msg(asocket);
  return response;
}

client_ptr client_new(char *path) {
  log_trace("Creating client [%s]", path);
  client_ptr client = (client_ptr) malloc(sizeof(client_t));
  client->socket = socket_new(path);
  socket_create_endpoint(client->socket);
  return client;
}

void *server_handle_client(void *aargs) {
  handle_client_args_ptr args = (handle_client_args_ptr) aargs;
  server_ptr server  = (server_ptr)args->server;
  socket_ptr csocket = (socket_ptr)args->csocket;
  msg_ptr request, response;
  log_trace("Handling client...");
  request = socket_read_msg(csocket);
  response = server->callback(request);
  socket_write_msg(csocket, response);
  // free the request/response messages
  msg_free(request);
  msg_free(response);
  // free aargs
  free(args);
  // free the csocket
  socket_free(csocket);
}

void *server_listen(void *aserver) {
  server_ptr server = (server_ptr)aserver;
  server->is_listening = TRUE;
  socket_ptr ssocket = server->socket;
  socket_ptr csocket; // client socket
  pthread_t hc_thread;
  handle_client_args_ptr args;
  
  log_trace("Starting server listening thread [%s]", ssocket->path);
  // TODO: max clients = 10, read from conf file
  if (listen(ssocket->sd, 10) == -1) {
    log_error("Error listening [%s]", ssocket->path);    
  } else {
    log_trace("Starting accept loop [%s]", ssocket->path);
    while(1) {
      csocket = socket_new(NULL);
      csocket->addr_len = sizeof(csocket->addr);
      csocket->sd = accept (ssocket->sd, &csocket->addr, &csocket->addr_len);
      args = (handle_client_args_ptr) malloc(sizeof(handle_client_args_t));
      args->server = server;
      args->csocket = csocket;
      int rc = pthread_create(&hc_thread, NULL, server_handle_client, (void *)args);
      if (rc) {
        log_error("Handle client thread can not be created, return code is [%d]", rc);
      }
    }
  }

  log_trace("Listening thread terminated");
  server->is_listening = FALSE;
  return NULL;
}

void server_start_listening(server_ptr server) {
    int rc = pthread_create(&server->listening_thread, NULL, server_listen, (void *)server);
    if (rc) {
      log_error("Listening thread can not be created, return code is [%d]", rc);
      return;
    }
    server->is_listening = TRUE;
}

server_ptr server_new(char *path, msg_ptr (*callback)(msg_ptr)) {
    log_trace("Creating server [%s]", path);
    server_ptr server = (server_ptr) malloc(sizeof(server_t));
    server->callback = callback;
    server->socket = socket_new(path);
    socket_create_endpoint(server->socket);
    socket_bind(server->socket);
    server->is_listening = FALSE;
    server_start_listening(server);
    return server;
}

msg_ptr pdp_process(msg_ptr request) {
  msg_ptr response;
  log_info ("Request received: [%s]", request->content);
  response = msg_new("SUCCESS");
  return response;
}

int main(void) {
    server_ptr server;
    client_ptr client;
    msg_ptr request, response;

    // create a socket, and redirect requests to callback function
    server = server_new("pdp_socket", pdp_process);
    server_start_listening(server);

    log_trace("--");
    log_trace("-- Client:");

    // client can only be use for one request/response
    client = client_new("pdp_socket");
    response = socket_msg_exchange(client->socket, msg_new("Ricardo"));
    log_info ("Response received: [%s]", response->content);
    client_free(client);

    client = client_new("pdp_socket");
    response = socket_msg_exchange(client->socket, msg_new("Neisse"));
    log_info ("Response received: [%s]", response->content);
    client_free(client);

    if (server->is_listening) {
      // wait for listening thread to finish
      pthread_join(server->listening_thread, NULL);
    }

    return 0;
}
