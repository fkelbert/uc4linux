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
 * Add a new mapping to f: Identifier -> Container, i.e. f(Identifier) = containerID
 * If the specified containerID is NULL, then a new container ID is transparently generated.
 *
 * @param identifier the identifier
 * @param containerID the container
 * @return the specified or created container ID. On error, 0 is returned.
 */
ucContainerID ucPIP_f_add(ucIdentifier identifier, ucContainerID containerID) {
	ucIdentifier identifierCopy;
	ucContainerID *containerIDCopy;

	if (!identifier) {
		return (0);
	}

	if (!containerID) {
		containerID = ucPIP_newContainerID();
	}

	if (!(identifierCopy = strdup(identifier))) {
		ucPIP_errorExit("Unable to allocate enough memory");
	}

	if (!(containerIDCopy = calloc(1, sizeof(ucContainerID)))) {
		ucPIP_errorExit("Unable to allocate enough memory");
	}
	*containerIDCopy = containerID;

	g_hash_table_insert(f, identifierCopy, containerIDCopy);

	return (containerID);
}


/**
 * Retrieve the container ID associated with the specified identifier, i.e. return f(identifier).
 * @param identifier the identifier
 * @result the container ID associated with f(identifier). On error, 0 is returned.
 */
ucContainerID ucPIP_f_get(ucIdentifier identifier) {
	gpointer retval;
	if (identifier && (retval = g_hash_table_lookup(f, identifier))) {
		return (*(ucContainerID*) retval);
	}

	return (0);
}
