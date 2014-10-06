#include "ucThrift.h"

#if UC_THRIFT

void ucInitThrift() {
	initPep2PdpThriftClient(21003);

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
