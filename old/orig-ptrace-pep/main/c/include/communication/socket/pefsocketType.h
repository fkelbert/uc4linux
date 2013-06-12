/**
 * @file  pefsocketType.h
 * @brief Type defintions for socket communications
 *
 * Provide a generic interface for socket communication. In general socket communication is platform dependent,
 * using this interface type, the internal processing is hidden.
 *
 * @author cornelius moucha
 **/

#ifndef PEFSOCKETTYPE_H_
#define PEFSOCKETTYPE_H_

#include "threadUtils.h"

/// TCP socket
#define PEFSOCKET_TCP        0
/// Unix domain sockets; currently not yet implemented
#define PEFSOCKET_UNIXDOMAIN 1
/// Windows named pipes; currently not yet implemented
#define PEFSOCKET_NAMEDPIPES 2

/// Method signature for socket handling thread
typedef char *(*PEFSOCKET_HANDLER)(char *);

typedef struct pefSocket_s  pefSocket_t;
typedef        pefSocket_t *pefSocket_ptr;

/**
 * @struct pefSocket_s
 * @brief Structure for socket communication
 *
 * This structure is used for encapsulating the platform-dependent socket communication.
**/
struct pefSocket_s
{
  unsigned int  type;
  /// socket file descriptor
  int           s;
  /// socket name (for unix domain sockets)
  char*         name;
  /// socket port (for TCP)
  unsigned int  port;
  /// handler method, called after receiving a message
  char*         (*handler)(char *msg);
  /// socket handling thread
  pefThread_ptr socketThread;

  // auxiliary method: client sending a message and waiting for a response
  char*         (*clientSend)(pefSocket_ptr curSocket, char *msg);
  // auxiliary method: sending a message to the communication partner (NOT waiting for response)
  int           (*send)(int fd, char *msg);
  // auxiliary method: reading a message from the socket; blocking!
  char*         (*read)(int fd);
};

#endif /* PEFSOCKETTYPE_H_ */





