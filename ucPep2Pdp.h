#ifndef UC_PEP2PDP
#define UC_PEP2PDP

#ifdef __cplusplus

#include <iostream>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "TPep2Pdp.h"
#include "ucTypes.h"

extern "C" {
#endif

extern void initPep2PdpThriftClient(int port);
extern bool connectPep2PdpThriftClient();
extern bool disconnectPep2PdpThriftClient();
extern void notifyEventToPdpThriftCpp(event *ev);

#ifdef __cplusplus
}
#endif


#endif
