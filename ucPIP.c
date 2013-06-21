/*
 * ucPIP.c
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 */

#include "ucPIP.h"

ucContainerID nextContainerID = 1;
ucDataID nextDataID = 1;

GHashTable *s;
GHashTable *l;
GHashTable *f;

void ucPIP_init() {
	s = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, destroyValueHashTable);
	l = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, destroyValueHashTable);
	f = g_hash_table_new_full(g_str_hash, g_str_equal, destroyKey, destroyValuePrimitive);
}

void destroyKey(gpointer data) {
	if (data) free(data);
}

void destroyValueHashTable(gpointer data) {
	if (data) g_hash_table_destroy(data);
}

void destroyValuePrimitive(gpointer data) {
	if (data) free(data);
}

ucDataID ucPIP_newDataID() {
	if (!nextDataID) {
		ucPIP_errorExit("run out of data IDs. Consider implementing data IDs using another data type.");
	}
	return (nextDataID++);
}

ucContainerID ucPIP_newContainerID() {
	if (!nextContainerID) {
		ucPIP_errorExit("Run out of container IDs. Consider implementing container IDs using another data type.");
	}
	return (nextContainerID++);
}


/**
 * Adds a new mapping to f: Identifier -> ContainerID.
 * If containerID is NULL or 0, then a new container ID is generated.
 * @return the added or created container ID
 */
ucContainerID ucPIP_f_add(ucIdentifier identifier, ucContainerID containerID) {
	if (!containerID) {
		containerID = ucPIP_newContainerID();
	}
	printf("%ld\n",containerID);

	ucIdentifier identifierCopy = strdup(identifier);
	ucContainerID *containerIDCopy = calloc(1, sizeof(ucContainerID));
	*containerIDCopy = containerID;

	g_hash_table_insert(f, identifierCopy, containerIDCopy);
	ucContainerID *c = g_hash_table_lookup(f, identifierCopy);

	return (containerID);
}
