#include "ucThrift.h"

#if UC_THRIFT

void ucInitThrift() {
	initPep2PdpThriftClient(UC_THRIFT_PDP_PORT);

	if (connectPep2PdpThriftClient()) {
		uc_log("success\n");
	}
	else {
		uc_log("failure\n");
		exit(1);
	}
}



inline void notifyEventToPdpThrift(event *ev) {
	notifyEventToPdpThriftCpp(ev);
}


#endif
