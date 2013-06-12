/*
 * soup_utils.h
 *      Author: Ricardo Neisse
 */

#ifndef _soup_utils_h
#define _soup_utils_h

#include <pthread.h>
#include <libsoup/soup.h>

typedef struct esfxmlrpc_s
{
  SoupServer   *server;
  int           port;
  char         *service_name;
  GMainLoop    *gloop;
  pthread_t     gloop_thread;
  unsigned char * (*handler)();
} esfxmlrpc_t;
typedef esfxmlrpc_t *esfxmlrpc_ptr;

void init_glib();
int msg_post(SoupMessage *msg);
int parse_method_call(SoupMessage *msg, char **method_name, GValueArray **params);
void type_error (SoupMessage *msg, GType expected, GValueArray *params, int bad_value);
void args_error (SoupMessage *msg, GValueArray *params, int expected);
gboolean check_xmlrpc (GValue *value, GType type, ...);
gboolean do_xmlrpc (SoupSession *session, char* uri, const char *method, GValue *retval, ...);
esfxmlrpc_ptr initXmlRPC(char *service_name, int port, void *eventHandler);

#endif
