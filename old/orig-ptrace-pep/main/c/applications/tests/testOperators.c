/**
 * @file  testOperators.c
 * @brief Exemplary tests for evaluation of OSL operators
 *
 * @author cornelius moucha
 **/

#include <sys/stat.h>
#include <unistd.h>
#include "pdp.h"
#include "log_testOperators_pef.h"

unsigned int pepSubscribeNative(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe)
{
  log_info("pepSubscribeNative invoked for name=[%s] and unsubscribe=[%d]...", name, unsubscribe);
  return R_SUCCESS;
}

unsigned int pxpExecuteNative(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params)
{
  log_info("pxpExecuteNative invoked for name=[%s] with [%d] params...", name, cntParams);
  return R_SUCCESS;
}

char *memEvent4 ="<event isTry=\"true\" action=\"action1\"><parameter name=\"val1\" value=\"value1\"/></event>";
char *memEvent4b="<event isTry=\"true\" action=\"action2\"><parameter name=\"val2\" value=\"value2\"/></event>";

void lsleep(uint64_t val)
{
  #ifndef __WIN32__
    usleep(val);
  #else
    Sleep(val/1000);
  #endif
}

void testAlways()
{
  char *policyFileName="/home/moucha/workspace/pef/src/main/xml/examples/testAlways.xml";
  int ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);

  getchar();
  int a;
  for(a=0; a<5; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);

    lsleep(3000000);
  }
}

void testBefore()
{
  char *policyFileName="/home/moucha/workspace/pef/src/main/xml/examples/testBefore.xml";
  //char *policyFileName="c:\\local\\rd\\usr\\home\\raindrop\\workspace\\pef\\src\\main\\xml\\examples\\testBefore.xml";
  int ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);

  getchar();
  int a;
  for(a=0; a<3; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);

    lsleep(3000000);
  }
}

void testDuring()
{
  char *policyFileName="/home/moucha/workspace/pef/src/main/xml/examples/testDuring.xml";
  //char *policyFileName="c:\\local\\rd\\usr\\home\\raindrop\\workspace\\pef\\src\\main\\xml\\examples\\testDuring.xml";
  int ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);

  getchar();
  int a;
  for(a=0; a<5; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);

    lsleep(3000000);
  }
}

void testWithin()
{
  char *policyFileName="/home/moucha/workspace/pef/src/main/xml/examples/testWithin.xml";
  //char *policyFileName="c:\\local\\rd\\usr\\home\\raindrop\\workspace\\pef\\src\\main\\xml\\examples\\testWithin.xml";
  int ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);

  getchar();
  int a;
  for(a=0; a<5; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);

    lsleep(3000000);
  }
}

void testRepmax()
{
  char *policyFileName="/home/moucha/workspace/pef/src/main/xml/examples/testRepmax.xml";
  //char *policyFileName="c:\\local\\rd\\usr\\home\\raindrop\\workspace\\pef\\src\\main\\xml\\examples\\testRepmax.xml";
  int ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);

  getchar();
  int a;
  for(a=0; a<5; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);

    lsleep(3000000);
  }
}

void testReplim()
{
  char *policyFileName="/home/moucha/workspace/pef/src/main/xml/examples/testReplim.xml";
  //char *policyFileName="c:\\local\\rd\\usr\\home\\raindrop\\workspace\\pef\\src\\main\\xml\\examples\\testReplim.xml";
  int ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);

  getchar();
  int a;
  for(a=0; a<5; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);

    lsleep(3000000);
  }
  lsleep(9000000);

  for(a=0; a<5; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);

    lsleep(3000000);
  }
}

void testRepsince()
{
  char *policyFileName="/home/moucha/workspace/pef/src/main/xml/examples/testRepSince.xml";
  int ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);

  int a;
  //lsleep(1000000);
  //for(a=0; a<3; a++)
  //{
  //  char *actionResponse=pdpNotifyEventXML(memEvent4b);
  //  log_trace("response=[%s]", actionResponse);
  //  free(actionResponse);
  //  lsleep(3000000);
  //}

  getchar();
  char *actionResponse=pdpNotifyEventXML(memEvent4);
  log_trace("response=[%s]", actionResponse);
  free(actionResponse);

  getchar();
  for(a=0; a<6; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4b);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);
    lsleep(3000000);
  }
  getchar();

  actionResponse=pdpNotifyEventXML(memEvent4);
  log_trace("response=[%s]", actionResponse);
  free(actionResponse);

  for(a=0; a<3; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4b);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);

    lsleep(3000000);
  }
}

void testSince()
{
  char *policyFileName="/home/moucha/workspace/pef/src/main/xml/examples/testSince.xml";
  //char *policyFileName="c:\\local\\rd\\usr\\home\\raindrop\\workspace\\pef\\src\\main\\xml\\examples\\testSince.xml";
  int ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);

  lsleep(1000000);

  int a;
  for(a=0; a<3; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4b);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);
    lsleep(3000000);
  }

  getchar();
  char *actionResponse=pdpNotifyEventXML(memEvent4);
  log_trace("response=[%s]", actionResponse);
  free(actionResponse);

  getchar();

  actionResponse=pdpNotifyEventXML(memEvent4);
  log_trace("response=[%s]", actionResponse);
  free(actionResponse);

  for(a=0; a<6; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4b);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);
    lsleep(3000000);
  }
  getchar();

  actionResponse=pdpNotifyEventXML(memEvent4);
  log_trace("response=[%s]", actionResponse);
  free(actionResponse);

  for(a=0; a<3; a++)
  {
    char *actionResponse=pdpNotifyEventXML(memEvent4b);
    log_trace("response=[%s]", actionResponse);
    free(actionResponse);

    lsleep(3000000);
  }
}

int main(int argc, char **argv)
{
  log_warn("Starting testpdp application for operator-tests");
  int ret=pdpStart();
  log_trace("PDP started with result=[%s]",returnStr[ret]);

  // register this 'PEP'
  pdpInterface_ptr linterface=pdpInterfaceNew("testpdp", PDP_INTERFACE_NATIVE);
  linterface->pepSubscribe=pepSubscribeNative;
  linterface->pxpExecute=pxpExecuteNative;

  ret=pdpRegisterPEP("testpdp", linterface);
  log_trace("PEP registration => [%s]",returnStr[ret]);

  ret=pdpRegisterAction("action1", "testpdp");
  log_trace("Action registration => [%s]",returnStr[ret]);
  ret=pdpRegisterAction("action2", "testpdp");
  log_trace("Action registration => [%s]",returnStr[ret]);

  // register this 'PXP'
  ret=pdpRegisterPXP("testpdp", linterface);
  log_trace("PXP registration => [%s]",returnStr[ret]);

  ret=pdpRegisterExecutor("notify", "testpdp");
  log_trace("Executor registration => [%s]",returnStr[ret]);

  //testBefore();
  //testDuring();
  //testWithin();
  //testRepmax();
  //testReplim();
  //testAlways();
  //testSince();
  testRepsince();

  getchar();

  //log_warn("stopping PDP=[%d]\n", pdpStop());
  return 0;
}
