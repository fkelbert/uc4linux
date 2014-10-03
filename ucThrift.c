#include "ucThrift.h"

#if UC_THRIFT

bool ucInitThrift() {
	initPep2PdpThriftClient(21003);

	if (connectPep2PdpThriftClient()) {
		printf("success\n");
	}
	else {
		printf("failure\n");
	}

	return false;
}



inline void notifyEventToPdpThrift(event *ev) {

}


#endif
