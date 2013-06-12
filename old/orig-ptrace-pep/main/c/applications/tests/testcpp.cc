/**
 * @file  testcpp.cc
 * @brief Exemplary execution test for PDP with linking for c++
 *
 * @author cornelius moucha
**/

#include <stdio.h>
#include <string.h>
#include "pdp.h"
#include "log_testcpp_pef.h"

char *memEvent2b="<event isTry=\"true\" action=\"dataflow:receive\" index=\"ALL\"><parameter name=\"status\" value=\"Active\"/><parameter name=\"role\" value=\"Consumer\"/><parameter name=\"payload\" value=\"lt;data expires='0' timestamp='1329815490305' type='hsrHeaderRequest' xsi:schemaLocation='homeui_messages.xsd' xmlns:ns2='http://iese.fhg.de/ami/information/model' xmlns='http://iese.fhg.de/ami/homeapp/messages' xmlns:xsi='http://www.w3.org/2001/XMLSchema-instance'&gt;\
    &lt;hsrHeaderResponse&gt;\
        &lt;header read='false' title='T�glicher Gesundheitsstatus' timestamp='1329692455386' id='0'/&gt;\
        &lt;header read='false' title='Kritischer Gesundheitsstatus' timestamp='1329606055386' id='1'/&gt;\
        &lt;header read='false' title='T�glicher Gesundheitsstatus' timestamp='1329519655386' id='2'/&gt;\
        &lt;header read='false' title='T�glicher Gesundheitsstatus' timestamp='1329433255386' id='3'/&gt;\
        &lt;header read='false' title='T�glicher Gesundheitsstatus' timestamp='1329346855386' id='4'/&gt;\
        &lt;header read='false' title='T�glicher Gesundheitsstatus' timestamp='1329260455386' id='5'/&gt;\
        &lt;header read='false' title='T�glicher Gesundheitsstatus' timestamp='1329174055386' id='6'/&gt;\
        &lt;header read='false' title='T�glicher Gesundheitsstatus' timestamp='1329087655386' id='7'/&gt;\
    &lt;/hsrHeaderResponse&gt;\
&lt;/data&gt;\"/><parameter name=\"taint\" value=\"$TAINTVAL\" /><parameter name=\"target\" value=\"10.128.14.238:61613\"/><parameter name=\"in\" value=\"blablabla&lt;orderId&gt;$ORDERID&lt;/orderId&gt;blablabla\" /></event>";

void exitHandler(int sig)
{
  printf("exiting mechanism update thread...\n\n");
  fflush(stdout);
  //log_warn("stopping PDP=[%d]\n", pdpStop());
  //exit(1);
}

int main()
{
  log_error("Start c++ test");

  struct sigaction sigAction;
  memset(&sigAction, 0, sizeof(struct sigaction));
  memset(&sigAction.sa_mask, 0, sizeof *(&sigAction.sa_mask));
  sigAction.sa_flags=0;
  sigAction.sa_handler=&exitHandler;
  int ret2=sigaction(SIGTERM, &sigAction, NULL);
  if(ret2!=0) log_warn("Error registering exit signal handler");
  else log_debug("Mechanism update thread signal handler registered");


  log_debug("Starting native Test-PDP for cpp");
  int ret=pdpStart();
  log_trace("PDP started with result=[%d]",ret);

  char *policyFileName="/home/moucha/test1/aalpol4.xml";
  ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);

  char *deployedPolicies=pdpListDeployedMechanisms();
  log_trace("deployed mechanisms: [%s]", deployedPolicies);

  char *actionResponse=pdpNotifyEventXML(memEvent2b);
  log_trace("response=[%s]", actionResponse);

  getchar();
  getchar();
  getchar();
  getchar();
  getchar();
  //log_warn("stopping PDP=[%d]\n", pdpStop());

  return 0;
}

