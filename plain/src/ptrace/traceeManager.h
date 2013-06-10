
#ifndef PIDFDTABLE_H_
#define PIDFDTABLE_H_

#include <stddef.h>
#include <stdlib.h>
#include <glib/ghash.h>
#include "constants.h"
#include "tracee.h"


extern GHashTable *tracees;



void tmInit();
void tmNewTracee(int pid);
void tmDeleteTracee(int pid);
struct tracee *tmGetTracee(int pid);
int tmIsEmpty();

void tmDestroyKey(gpointer data);
void tmDestroyValue(gpointer data);

#endif /* PIDFDTABLE_H_ */
