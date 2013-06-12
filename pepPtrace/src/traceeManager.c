#include "traceeManager.h"

GHashTable *tracees;

/**
 * Initialize management of tracees
 */
void tmInit() {
	tracees = g_hash_table_new_full(g_int_hash, g_int_equal, tmDestroyKey,
			tmDestroyValue);
}

/*
 * Starts management of a new tracee for the specified process id.
 * Returns the created tracee, NULL on failure.
 */
struct tracee *tmNewTracee(int pid) {
	struct tracee *tracee;
	int *pidCopy;

	if ((tracee = g_hash_table_lookup(tracees, &pid)) == NULL) {
		if ((tracee = traceeCreate(pid)) != NULL) {

			if ((pidCopy = (int*) calloc(1, sizeof(int))) != NULL) {
				*pidCopy = pid;
			}

			g_hash_table_insert(tracees, pidCopy, tracee);
		}
	}

	return (tracee);
}

/**
 * Deletes the tracee with the specified process id.
 * Returns whether deletion was successful.
 */
int tmDeleteTracee(int pid) {
	return (g_hash_table_remove(tracees, &pid));
}

struct tracee *tmGetTracee(int pid) {
	return (g_hash_table_lookup(tracees, &pid));
}

/**
 * Returns a value equivalent to TRUE, if there are no tracees
 */
int tmIsEmpty() {
	return (g_hash_table_size(tracees) == 0);
}

void tmDestroyKey(gpointer data) {
	free(data);
}

void tmDestroyValue(gpointer data) {
	traceeDestroy(data);
}
