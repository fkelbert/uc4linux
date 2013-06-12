/**
 * @file  pdpSocketClient.c
 * @brief Exemplary test for a client communication with PDP over TCP sockets
 *
 * Needs to be updated!
 *
 * @author cornelius moucha
**/

#include "socketUtils.h"
#include "log_pdpSocketClient_pef.h"

int main(int argc, char **argv)
{
  log_info("Starting PDP communication test using TCP socket 9876");

  unsigned char *result;
  int errno;


  pefSocket_ptr lsocket=pefSocketClientNew(PEFSOCKET_TCP, "localhost", atoi(argv[1]));
  getchar();

  //unsigned char *policy="0@<?xml version='1.0'?><policySet xmlns='http://www.iese.fhg.de/esf/1.0/enforcementLanguage'><preventiveMechanism name='xx1'><description>xx1</description><timestep amount='1' unit='SECONDS'/><trigger action='open' isTry='true'></trigger><condition><XPathEval>/event/parameter[@name='command']/@value='cat'</XPathEval></condition><authorizationAction><inhibit/></authorizationAction></preventiveMechanism></policySet>\r\n";
  unsigned char *policy="0@<?xml version='1.0'?><policySet xmlns='http://www.iese.fhg.de/esf/1.0/enforcementLanguage'><preventiveMechanism name='xx1'><description>xx1</description><timestep amount='1' unit='SECONDS'/><trigger action='open' isTry='true'></trigger><condition><XPathEval>/event/parameter[@name='command']/@value='cat'</XPathEval></condition><authorizationAction><inhibit/></authorizationAction></preventiveMechanism></policySet>";
  result=lsocket->clientSend(lsocket, policy);
  log_info("received response=[%s]", result);
  getchar();

  log_info("Sending policy again to PDP");
  result=lsocket->clientSend(lsocket, policy);
  log_info("received response=[%s]", result);

  getchar();
  //char *event="2@<?xml version='1.0' encoding='UTF-8'?><event isTry='true' action='open'><parameter name='command' value='cat'/></event>\r\n";
  char *event="2@<?xml version='1.0' encoding='UTF-8'?><event isTry='true' action='open'><parameter name='command' value='cat'/></event>";
  while(1)
  {
    log_info("Sending event to PDP");
    result=lsocket->clientSend(lsocket, event);
    log_info("received response=[%s]", result);
    getchar();
  }

  return 0;
}


