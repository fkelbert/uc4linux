#ifndef USAGECONTROL_H
#define USAGECONTROL_H


#include "defs.h"
#include "ucPIP_main.h"

#define UC_ENABLED 1
#define UC_DEBUG_MODE 1

#define UC_PDP_ALLOW 1
#define UC_PDP_INHIBIT 2
#define UC_PDP_MODIFY 3
#define UC_PDP_DELAY 4
#define ucPDPask(tcp) UC_PDP_ALLOW

void ucInit();

int ucDesired(struct tcb *tcp);
int ucActual(struct tcb *tcp);




#endif /* USAGECONTROL_H */
