/**
 * @file  appPDPcpp.cc
 * @brief Exemplary execution test for PDP with linking for c++
 *
 * @author cornelius moucha
**/

#include <stdio.h>
#include <appPDPcpp.hh>
#include "log_appPDPcpp_pef.h"

extern pdp_ptr pdp;

unsigned int pepSubscribeNative(pdpInterface_ptr linterface, char *name, unsigned int unsubscribe)
{
  log_info("pepSubscribeNative invoked for name=[%s] and unsubscribe=[%d]...", name, unsubscribe);
  return R_SUCCESS;
}

/*unsigned int pxpExecuteNative(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params)
{
  log_info("pxpExecuteNative invoked for name=[%s] with [%d] params...", name, cntParams);
  return R_SUCCESS;
}*/

//class pdpExample
//{
//
//  public:
    pdpExample::pdpExample()
    {
      int ret=pdpStart();
      log_trace("PDP started with result=[%d]",ret);
    }

    pdpExample::~pdpExample()
    {
      log_debug("destructor called\n");
      log_warn("stopping PDP=[%d]\n", pdpStop());
    }

    unsigned int pdpExample::init()
    {
      pdpInterface_ptr linterface=pdpInterfaceNew("appPDPcppPEP", PDP_INTERFACE_NATIVE);
      linterface->pepSubscribe=pepSubscribeNative;

      pdpInterface_ptr linterface2=pdpInterfaceNew("appPDPcppPXP", PDP_INTERFACE_NATIVE);
      linterface2->pxpExecute=this;
      //linterface2->pxpExecute=(unsigned int (*)(pdpInterface_t*, char*, unsigned int, parameterInstance_t**))(&this->pxpExecuteNative);

      // register this 'PXP'
      unsigned int ret=pdpRegisterPXP("appPDPcppPXP", linterface2);
      log_trace("PXP registration => [%s]",returnStr[ret]);

      ret=pdpRegisterExecutor("notify", "appPDPcppPXP");
      log_trace("Executor registration => [%s]",returnStr[ret]);

      return ret;
    }

    unsigned int pdpExample::deploy()
    {
      //char *policyFileName="c:\\local\\rd\\usr\\home\\moucha\\workspace\\pef\\src\\main\\xml\\examples\\test.xml";
      char *policyFileName="/home/rd/workspace/pef/src/main/xml/examples/test.xml";

      unsigned int ret=pdpDeployPolicy(policyFileName);
      log_trace("deploy returned=[%d]",ret);

      char *deployedPolicies=pdpListDeployedMechanisms();
      log_trace("deployed mechanisms: [%s]", deployedPolicies);
      return ret;
    }

    unsigned int pdpExample::test()
    {
      char *memEvent4b="<event isTry=\"true\" action=\"action2\"><parameter name=\"val2\" value=\"value2\"/></event>";
      char *actionResponse=pdpNotifyEventXML(memEvent4b);
      log_trace("response=[%s]", actionResponse);
      free(actionResponse);
      return 0;
    }

    /*unsigned int pxpExecuteNativeIntern(pdpInterface_ptr linterface, char *name, unsigned int cntParams, parameterInstance_ptr *params)
    {
      log_trace("\n\npxpExecuteNativeIntern called.\n\n");

      return 0;
    }*/
//};

int main(int argc, char **argv)
{
  log_debug("Starting native Test-PDP for cpp");
  pdpExample *lpdp=new pdpExample();
  log_debug("\n\ninitializing...");
  lpdp->init();
  lpdp->deploy();
  getchar();
  lpdp->test();
  getchar();

  getchar();
  log_debug("Exiting...");
  delete lpdp;
  return 0;
}
