#include "ucThrift.h"

#if UC_THRIFT

void ucInitThrift() {
	initPep2PdpThriftClient(UC_THRIFT_PDP_PORT);

	while (!connectPep2PdpThriftClient()) {
		uc_log("Trying to connect to PDP.\n");
		sleep(1);
	}
}

inline void notifyEventToPdpThrift(event *ev) {
	notifyEventToPdpThriftCpp(ev);
}


#endif
