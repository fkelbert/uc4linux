#include "traceeManager.h"

GHashTable *tracees;

void tmInit() {
	tracees = g_hash_table_new(g_int_hash, g_int_equal);
}

void tmNewTracee(int pid) {
	struct tracee *inc = g_hash_table_lookup(tracees, &pid);
	struct passwd *user_info;

	if (inc == NULL) {
		inc = (struct tracee *) malloc(sizeof(struct tracee));

		int *x = (int*) malloc(sizeof(int));
		*x = pid;

		user_info = (struct passwd *) getUserInfo(pid);
		int *y = (int*) malloc(sizeof(int));
		*y = pid;

		inc->status = SYSIN;
		inc->user_info = user_info;

		g_hash_table_insert(tracees, x, inc);
	}
}

void tmDeleteTracee(int pid) {
	// FIXME: free memory here!?
	g_hash_table_remove(tracees, &pid);
}


struct tracee *tmGetTracee(int pid) {
  return g_hash_table_lookup(tracees, &pid);
}

int tmIsEmpty() {
	return g_hash_table_size(tracees) == 0;
}
