#pragma once

#include <glib/ghash.h>
#include <pthread.h>
#include <unistd.h>

#include "soup_utils.h"
#include "socket_utils.h"
#include "mechanism.h"
#include "base.h"

#ifdef _JNI_PDP_
  // JNI connection header
  #include "PolicyDecisionPointNative.h"
#endif

#define PDP_XMLRPC 1
#define PDP_SOCKET 1

#define DEFAULT_POLICY "allow"

#define DEFAULT_PDP_XMLPORT (8081)
#define DEFAULT_PDP_SOCKPORT (9876)

action_desc_store_ptr pdp_action_desc_store;

// information flow stuff
#include "data_flow_monitor.h"

typedef struct notifyResponse_s      notifyResponse_t;
typedef        notifyResponse_t     *notifyResponse_ptr;

LIBEXPORT bool pdpStart(unsigned long xml_port, unsigned long sock_port);
LIBEXPORT bool pdpStop();
LIBEXPORT unsigned int pdpRevokeMechanism(const unsigned char *mechName);
LIBEXPORT unsigned int pdpDeployMechanism(const unsigned char*);
LIBEXPORT unsigned char *pdpNotifyEventXML(const unsigned char*);
LIBEXPORT notifyResponse_ptr pdpNotifyEvent(event_ptr);

// XML-RPC handler
void event_handler(SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data);

// Socket handler
unsigned char *pdpSocketHandler(unsigned char *msg);

unsigned int mechanism_add(xmlNodePtr);
unsigned int load_mechanisms(const unsigned char *);
unsigned int load_mechanisms_xml(xmlNodePtr);
void searchMechanismNodes(xmlNodePtr rootNode, bool revoke);

void        pdp_startUpdateThread(uint64_t);
void        pdp_thread(uint64_t*);

bool update_mechanism(const unsigned char *, mechanism_ptr, event_ptr);
void* lookup_mechanism(const unsigned char*);

gboolean mechanismTriggerEvent(const unsigned char *name, mechanism_ptr mech, event_ptr levent);
void mechanismTriggerEvent2(mechanism_ptr mech, notifyResponse_ptr response);


struct notifyResponse_s {
  event_ptr             event;
  mechanism_actions_ptr authorizationAction;
  unsigned int          n_actions_to_execute;
  action_ptr           *actions_to_execute;
};

void pdp_logNotifyResponse(notifyResponse_ptr response);
notifyResponse_ptr pdp_newNotifyResponse(event_ptr);
bool pdp_addMechanismResponse(notifyResponse_ptr, mechanism_ptr);
char *notifyResponseSerialize(notifyResponse_ptr response);



