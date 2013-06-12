/**
 * @file  pdpInSocket.c
 * @brief Implementation of PDP communication interface using TCP sockets
 *
 * Provide input interface for policy management (deployment, revocation, mechanism listing) and for
 * event notifications. Furthermore an output communication interface to the PIP is provided for
 * initialization as well as evaluation requests.
 *
 * @todo pdpInSocket not fully implemented and tested!
 *
 * @author cornelius moucha
 **/

#include "pdpCommInterfaces.h"
#include "log_pdpInSocket_pef.h"

// PDP IN interface via TCP socket
#if PDP_TCPSOCKET > 0
  pefSocket_ptr pdpInTCPsocket;

  char *pdpSocketHandler(char *msg)
  {
    log_debug("socket-msg=[%d][%s]",msg[0],msg);

    if(msg[0]==     PDP_START) return returnStr[pdpStart()];
    else if(msg[0]==PDP_STOP)  return returnStr[pdpStop()];
    if(msg[0]==     PDP_DEPLOYPOLICY)
    {
      log_debug("Invoking deployPolicy");
      char *policyString=msg+1;
      log_trace("Mechanism received via TCP-socket :\n%s",policyString);
      unsigned int ret=pdpDeployPolicyString(policyString, "namespace");
      log_warn("deploying returned with=[%d]", ret);

      char *responseString=malloc(8*sizeof(char));
      if(ret==R_SUCCESS) snprintf(responseString, 8, "success");
      else snprintf(responseString, 8, "error");
      return responseString;
    }
    else if(msg[0]==PDP_REVOKEMECHANISM)
    {
      log_debug("received policy for revoking...");
      return "notYetImplemented";
    }
    else if(msg[0]==PDP_NOTIFYEVENT)
    {
      log_debug("Invoking notifyEvent");
      char *eventString=msg+1;
      unsigned char *response=pdpNotifyEventXML(eventString);
      return response;
    }
    if(msg[0]==PDP_LISTMECHANISMS)
    {
      log_debug("Invoking listDeployedMechanisms");
      char *policies=pdpListDeployedMechanisms();
      return policies;
    }
    else
    {
      log_warn("Received unsupported method via TCP socket?!");
      char *response=malloc(29*sizeof(char));
      snprintf(response, 29, "Unsupported method received!");
      return response;
    }
  }
#endif // PDP_TCPSOCKET == 1


