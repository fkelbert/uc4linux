#ifndef USAGECONTROL_H
#define USAGECONTROL_H


#include <jni.h>
#include <stdbool.h>
#include <pthread.h>
#include "defs.h"
#include "ucEvents.h"
#include "ucLog.h"
#include "ucSettings.h"
#include "ucTypes.h"
#include "ucJniBridge.h"

bool ucInit();
void ucEnd();

#endif /* USAGECONTROL_H */

