#include "traceeManager.h"

GHashTable *tracees;

/**
 * Initialize management of tracees
 */
void tmInit() {
	tracees = g_hash_table_new_full(g_int_hash, g_int_equal, tmDestroyKey, tmDestroyValue);
}


void tmNewTracee(int pid) {
	struct tracee *tracee;

	if ((tracee = g_hash_table_lookup(tracees, &pid)) == NULL) {
		// FIXME: this call may return null, handle accordingly
		tracee = traceeCreate(pid);

		// FIXME malloc may fail. handle that case
		int *pidCopy = (int*) malloc(sizeof(int));
		*pidCopy = pid;

		g_hash_table_insert(tracees, pidCopy, tracee);
	}
}

void tmDeleteTracee(int pid) {
	g_hash_table_remove(tracees, &pid);
}


struct tracee *tmGetTracee(int pid) {
  return (g_hash_table_lookup(tracees, &pid));
}

int tmIsEmpty() {
	return (g_hash_table_size(tracees) == 0);
}

void tmDestroyKey(gpointer data) {
	free(data);
}

void tmDestroyValue(gpointer data) {
	traceeDestroy(data);
}
