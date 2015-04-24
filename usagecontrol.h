#ifndef USAGECONTROL_H
#define USAGECONTROL_H

#if UC_JNI
#include <jni.h>
#endif

#include <stdbool.h>
#include <pthread.h>
#include "defs.h"
#include "ucEvents.h"
#include "ucLog.h"
#include "ucSettings.h"
#include "ucTypes.h"
#include "ucJni.h"
#include "ucThrift.h"

void ucInit();
void ucEnd();
void notifySyscall(struct tcb *tcp);

#endif /* USAGECONTROL_H */

