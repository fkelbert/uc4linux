#ifndef UCTHRIFT_H_
#define UCTHRIFT_H_

#include <stdbool.h>
#include "defs.h"
#include "ucTypes.h"
#include "ucPep2Pdp.h"
#include "ucLog.h"

void ucInitThrift();

inline void notifyEventToPdpThrift(event *ev) {
	notifyEventToPdpThriftCpp(ev);
}


#endif
