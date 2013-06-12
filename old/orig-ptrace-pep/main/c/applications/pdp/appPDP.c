/**
 * @file  appPDP.c
 * @brief Exemplary execution test for native PDP
 *
 * @author cornelius moucha
**/

#include <sys/stat.h>
#include <unistd.h>
#include "pdp.h"

#include "log_appPDP_pef.h"

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

int main(int argc, char **argv)
{
  log_warn("Starting native Test-PDP");
  int ret=pdpStart();
  log_trace("PDP started with result=[%s]",returnStr[ret]);

  pdpInterface_ptr linterface=pdpInterfaceNew("appPDPpep", PDP_INTERFACE_NATIVE);
  linterface->pepSubscribe=pepSubscribeNative;

  pdpInterface_ptr linterface2=pdpInterfaceNew("appPDPpxp", PDP_INTERFACE_NATIVE);
  linterface2->pxpExecute=pxpExecuteNative;

  // register this 'PXP'
  ret=pdpRegisterPXP("appPDPpxp", linterface2);
  log_trace("PXP registration => [%s]",returnStr[ret]);

  ret=pdpRegisterExecutor("notify", "appPDPpxp");
  log_trace("Executor registration => [%s]",returnStr[ret]);

  char *policyFileName="/home/moucha/workspace/pef/src/main/xml/examples/test.xml";

  ret=pdpDeployPolicy(policyFileName);
  log_trace("deploy returned=[%d]",ret);
  getchar();

  log_warn("stopping PDP=[%d]\n", pdpStop());
  return 0;
}
