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

// TODO: need a method to remove a process. that one deletes the corresponding has table. it gets called if a process exits.

// TODO: for convenience also have a method in case a process gets started( clone()). In this case we do the administrative part to insret the hash table, etc.

void ucDeclass_splus_add(pid_t pid, ucDataSet dataSetToAdd) {
	GTree *times;
	pid_t *pidCopy;
	struct timeval *time;
	GHashTableIter iter;
	ucDataSet dataSet;
	gpointer container;
	ucContainerID *containerCopy;

	if (pid <= 0 || INVALID_DATASET(dataSetToAdd) || ucPIP_isEmptyDataSet(dataSetToAdd)) {
		return;
	}

	// get the set of "times" for the specified process;
	// create a new tree if not found
	if (!(times = g_hash_table_lookup(processes, &pid))) {
		pidDup(pidCopy, pid);
		times = g_tree_new_full(intCmp, NULL, free, (GDestroyNotify) g_hash_table_destroy);
		g_hash_table_insert(processes, pidCopy, times);
	}

	if (!(time = calloc(1, sizeof(struct timeval)))) {
		ucDeclass_errorExitMemory();
	}
	gettimeofday(time, NULL );

	printf("data flow into proc %d @time %ld.%ld: ", pid, time->tv_sec, time->tv_usec);

	if (INVALID_DATASET(dataSet = g_tree_lookup(times, time))) {
		// No associated data set found. Just take the one we got.
		g_tree_insert(times, time, dataSetToAdd);
	}
	else {
		// There was an entry. Copy each and every entry.
		// This will usually not happen in a sequential PEP implementation
		g_hash_table_iter_init(&iter, dataSetToAdd);
		while (g_hash_table_iter_next(&iter, &container, NULL )) {
			containerDup(containerCopy, * (ucContainerID*) container);
			g_hash_table_insert(dataSet, containerCopy, NULL );
		}
	}
	dataSetPrint(stdout, g_tree_lookup(times, time));
	printf("\n");
}


gboolean ucDeclass_printSPlusElement(gpointer key, gpointer value, gpointer data) {
	struct timeval timeStep = *(struct timeval *) key;
	ucDataSet dataSetRead = (ucDataSet) value;
	struct printSPlusData d = *(struct printSPlusData *) data;

	fprintf(d.out, "  %d: %ld.%ld: ", d.pid, timeStep.tv_sec, timeStep.tv_usec);
	dataSetPrint(d.out, dataSetRead);
	fprintf(d.out, "\n");
	return (0);
}


void ucDeclass_printSPlus_impl(FILE *out, pid_t pid) {
	GTree *processChangeTimes;

	// all times at which the processes' status has changed
	if (!(processChangeTimes = g_hash_table_lookup(processes, &pid))) {
		return;
	}

	// data to pass to the function that handles every single entry
	struct printSPlusData data;
	data.out = out;
	data.pid = pid;

	// print it!
	fprintf(out, "Function s+:\n");
	g_tree_foreach(processChangeTimes, ucDeclass_printSPlusElement, &data);
	fprintf(out, "\n");
}
