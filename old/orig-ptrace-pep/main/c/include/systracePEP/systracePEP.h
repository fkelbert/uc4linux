/**
 * @file  systracePEP.h
 * @brief Policy Enforcement Point for OpenBSD using Systrace system call interposition
 *
 * @author cornelius moucha
**/

#ifndef systracePEP_h
#define systracePEP_h

#include <poll.h>
#include "pefEvent.h"
#include "openbsdUtils.h"
#include "systraceUtils.h"
#include "syscallHandler.h"
#include "socketUtils.h"
#include "pdp.h"
#include "action.h"

extern actionDescStore_ptr pdp->actionDescStore;

#endif
