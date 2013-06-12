/**
 * @file  soupUtils.h
 * @brief Utility methods for providing an interface for XML-RPC (using libsoup)
 *
 * @author Ricardo Neisse, cornelius moucha
 **/

#ifndef _soup_utils_h
#define _soup_utils_h

#include <pthread.h>
#include <libsoup/soup.h>
#include "base.h"
#include "memUtils.h"

typedef struct pefxmlrpc_s  pefxmlrpc_t;
typedef pefxmlrpc_t *pefxmlrpc_ptr;

struct pefxmlrpc_s
{
  SoupServer *server;
  int         port;
  char       *service_name;
  GMainLoop  *gloop;
  pthread_t   gloop_thread;
  char*      (*handler)();
};


void     initGLib();
int      msgPost(SoupMessage *msg);
int      parseMethodCall(SoupMessage *msg, char **method_name, GValueArray **params);
void     typeError(SoupMessage *msg, GType expected, GValueArray *params, int bad_value);
void     argsError(SoupMessage *msg, GValueArray *params, int expected);
gboolean checkXMLrpc(GValue *value, GType type, ...);
gboolean doXMLrpc(SoupSession *session, char* uri, const char *method, GValue *retval, ...);

pefxmlrpc_ptr xmlrpcInit(char *service_name, int port, void *eventHandler);
unsigned int  xmlrpcFree(pefxmlrpc_ptr xmlrpcInterface);

#endif
