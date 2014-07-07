
#ifndef TRACEEMANAGER_H_
#define TRACEEMANAGER_H_

#include <stddef.h>
#include <stdlib.h>
#include <glib.h>
#include "tracee.h"


extern GHashTable *tracees;



void tmInit();
struct tracee *tmNewTracee(int pid);
int tmDeleteTracee(int pid);
struct tracee *tmGetTracee(int pid);
int tmIsEmpty();

void tmDestroyKey(gpointer data);
void tmDestroyValue(gpointer data);

#endif /* TRACEEMANAGER_H_ */
