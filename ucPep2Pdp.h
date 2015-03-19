#ifndef UC_PEP2PDP
#define UC_PEP2PDP

#ifdef __cplusplus

#include <iostream>
#include <chrono>
#include <unistd.h>
#include <thrift/transport/TSocket.h>
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/transport/TTransportUtils.h>
#include "TPep2Pdp.h"
#include "ucTypes.h"
#include "time.h"

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
