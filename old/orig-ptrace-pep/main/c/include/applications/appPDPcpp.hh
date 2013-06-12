/*
 * appPDPcpp.hh
 *
 *  Created on: Jul 5, 2012
 *      Author: cornelius moucha
 */

#ifndef APPPDPCPP_HH_
#define APPPDPCPP_HH_

#include "pdp.h"
#include "testPXPclass.hh"

class pdpExample: public pxpTemplate
{
  public:
    pdpExample();
    ~pdpExample();
    unsigned int init();
    unsigned int deploy();
    unsigned int test();
};

#endif /* APPPDPCPP_HH_ */
