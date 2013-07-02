/*
 * ucDeclass.c
 *
 *  Created on: Jul 1, 2013
 *      Author: user
 */


#include "ucDeclass.h"

GHashTable *processes;

gint intCmp(gconstpointer a, gconstpointer b, gpointer user_data) {
	// should sort in reverse order
	const struct timeval *ta = a;
	const 	struct timeval *tb = b;

	return ((ta->tv_sec != tb->tv_sec)
			? (tb->tv_sec - ta->tv_sec)
			: (tb->tv_usec - ta->tv_usec));
}



void ucDeclass__init() {
	processes = g_hash_table_new_full(g_int_hash, g_int_equal, free, (GDestroyNotify) g_tree_destroy);
}

void ucDeclass_splus_add(pid_t pid, ucDataSet dataSetToAdd) {
	GTree *times;
	pid_t *pidCopy;
	struct timeval *time;
	GHashTableIter iter;
	ucDataSet dataSetNew;
	gpointer container;
	ucContainerID *containerCopy;

	if (pid <= 0 || INVALID_DATASET(dataSetToAdd)) {
		return;
	}

	// get the set of "times" for the specified process;
	// create a new tree if not found
	if (!(times = g_hash_table_lookup(processes, &pid))) {
		if (!(pidCopy = calloc(1, sizeof(pid_t)))) {
			ucDeclass_errorExit("Unable to allocate enough memory");
		}
		*pidCopy = pid;
		times = g_tree_new_full(intCmp, NULL, free, (GDestroyNotify) g_hash_table_destroy);

		g_hash_table_insert(processes, pidCopy, times);
	}

	if (!(time = calloc(1, sizeof(struct timeval)))) {
		ucDeclass_errorExit("Unable to allocate enough memory.");
	}
	gettimeofday(time, NULL);

	if (INVALID_DATASET(dataSetNew = g_tree_lookup(times, time))) {
		dataSetNew = g_hash_table_new_full(g_int_hash, g_int_equal, free, NULL);
		g_tree_insert(times, time, dataSetNew);
	}

	// copy each and every entry
	g_hash_table_iter_init(&iter, dataSetToAdd);
	while (g_hash_table_iter_next (&iter, &container, NULL)) {

		if (INVALID_CONTID(containerCopy = calloc(1, sizeof(ucContainerID)))) {
			ucPIP_errorExit("Unable to allocate enough memory");
		}
		*containerCopy = * (ucContainerID*) container;
		g_hash_table_insert(dataSetNew, containerCopy, NULL);
	}
}
