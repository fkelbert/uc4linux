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
		ucPIP_errorExit("Run out of data IDs. Consider implementing data IDs using another data type.");
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
		ucPIP_errorExitMemory();
	}

	if (!(containerIDCopy = calloc(1, sizeof(ucContainerID)))) {
		ucPIP_errorExitMemory();
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


void ucPIP_f_remove(ucIdentifier identifier) {
	if (identifier) {
		g_hash_table_remove(f, &identifier);
	}
}


void ucPIP_s_add(ucContainerID containerID, ucDataID* dataIDs, int count) {
	GHashTable *dataSet;
	ucContainerID *containerIDCopy;
	ucDataID *dataIDsCopy;
	int i;

	if (!containerID || !dataIDs || count <= 0) {
		return;
	}

	if (!(dataSet = g_hash_table_lookup(s, &containerID))) {
		// We did not find an associated data set. Create one.
		// We do not care about the values of that data set -- always insert NULL values
		dataSet = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, NULL);

		if (!(containerIDCopy = calloc(1, sizeof(ucContainerID)))) {
			ucPIP_errorExitMemory();
		}
		*containerIDCopy = containerID;

		g_hash_table_insert(s, containerIDCopy, dataSet);
	}

	dataIDsCopy = calloc(count, sizeof(ucDataID));
	memcpy(dataIDsCopy, dataIDs, count * sizeof(ucDataID));
	for (i = 0; i < count; i++) {
		g_hash_table_insert(dataSet, &dataIDsCopy[i], NULL);
	}
}

/**
 * Retrieves the data IDs associated with the specified container ID, i.e. s(containerID).
 * The data IDs are returned. count is set to the number of entries. This function allocates the necessary memory which
 * must then be freed by the caller using free().
 * @return count the number of entries, -1 on error. -1 is also returned, if dataIDs != NULL.
 * @return the allocated memory area
 */
//ucDataID *ucPIP_s_get(ucContainerID containerID, int *count) {
//	GHashTable *dataSet;
//
//	if (!containerID || dataIDs) {
//		return -1;
//	}
//
//	if ((dataSet = g_hash_table_lookup(s, &containerID)) == NULL) {
//		return 0;
//	}
//
//	*count = g_hash_table_size(dataSet);
//
//	ucDataID *dataIDs;
//
//	if ((dataIDs = calloc(*count, sizeof(ucDataID))) == NULL) {
//		ucPIP_errorExit("Unable to allocate enough memory");
//	}
//
//	int i;
//	for (i = 0; i < *count; i++) {
//		dataIDs[i] =
//	}
//}

//void ucPIP_s_remove(ucContainerID) {
//
//}


/**
 * Adds a new (additional) identifier for the container that has so far
 * been identified with the specified old identifier. If the old identifier
 * is not yet associated with a container, then a new container is created
 * transparently.
 * @param oldIdentifier the old identifier identifying the container
 * @param newIdentifier the new (additional) identifier for that same container
 *
 */
void ucPIP_addIdentifier(ucIdentifier oldIdentifier, ucIdentifier newIdentifier) {
	ucContainerID *container;
	ucContainerID *container2;

	if (!oldIdentifier || !newIdentifier) {
		return;
	}

	if (!(container = g_hash_table_lookup(f, oldIdentifier))) {
		if (!(container = calloc(1, sizeof(ucContainerID)))) {
			ucPIP_errorExitMemory();
		}

		*container = ucPIP_newContainerID();

		g_hash_table_insert(f, strdup(oldIdentifier), container);
	}

	if (!(container2 = calloc(1, sizeof(ucContainerID)))) {
		ucPIP_errorExitMemory();
	}

	*container2 = *container;

	g_hash_table_insert(f, strdup(newIdentifier), container2);
}


/**
 * Removes the specified identifier mapping in function f().
 */
void ucPIP_removeIdentifier(ucIdentifier identifier) {
	if (identifier) {
		g_hash_table_remove(f, identifier);
	}
}


void ucPIP_copyDataEntry(gpointer key, gpointer value, gpointer dstStorage) {
	ucDataID *keyCopy = calloc(1, sizeof(ucDataID));
	*keyCopy = * (ucDataID*) key;
	g_hash_table_insert(dstStorage, keyCopy, NULL);
}


/**
 * Copy all data in the container identified by srcIdent to
 * the container identified by dstIdent. If there is no container
 * that is identified by dstIdent, then one is transparently created.
 */
void ucPIP_copyData(ucIdentifier srcIdentifier, ucIdentifier dstIdentifier) {
	ucContainerID *srcContainer;
	ucContainerID *dstContainer;
	GHashTable *srcStorage;
	GHashTable *dstStorage;

	if (!srcIdentifier || !dstIdentifier) {
		return;
	}

	if (!(srcContainer = g_hash_table_lookup(f, srcIdentifier))) {
		return;
	}

	// No sensitive data in source container. Nothing to do.
	if (!(srcStorage = g_hash_table_lookup(s, srcContainer)) || !g_hash_table_size(srcStorage)) {
		return;
	}

	if (!(dstContainer = g_hash_table_lookup(f, dstIdentifier))) {
		if (!(dstContainer = calloc(1, sizeof(ucContainerID)))) {
			ucPIP_errorExitMemory();
		}

		*dstContainer = ucPIP_newContainerID();

		g_hash_table_insert(f, strdup(dstIdentifier), dstContainer);
	}

	if (!(dstStorage = g_hash_table_lookup(s, dstContainer))) {
		if (!(dstStorage = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, NULL))) {
			ucPIP_errorExitMemory();
		}

		// TODO: this will probably fail if dstContainer gets deleted afterwards.
		// Create a copy of dstContainer for this case first.
		// see above, addIdentifier()
		g_hash_table_insert(s, dstContainer, dstStorage);
	}

	g_hash_table_foreach(srcStorage, ucPIP_copyDataEntry, dstStorage);
}



void ucPIP_printFentry(gpointer key, gpointer value, gpointer data) {
	printf(" %s --> %d\n", (char*) key, * ((ucContainerID*) value));
}

void ucPIP_printF() {
	printf("Function f():\n");
	g_hash_table_foreach(f, ucPIP_printFentry, NULL);
	printf("\n");
}

void ucPIP_printSentryentry(gpointer key, gpointer value, gpointer data) {
	printf("%d, ", *(ucDataID*)key);
}

void ucPIP_printSentry(gpointer key, gpointer value, gpointer data) {
	printf(" %d --> {", *(ucContainerID*) key);
	g_hash_table_foreach(value, ucPIP_printSentryentry, NULL);
	printf("}\n");
}

void ucPIP_printS() {
	printf("Function s():\n");
	g_hash_table_foreach(s, ucPIP_printSentry, NULL);
	printf("\n");
}


void ucPIP_init() {
	s = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, destroyValueHashTable);
	l = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, destroyValueHashTable);
	f = g_hash_table_new_full(g_str_hash, g_str_equal, destroyKey, destroyValuePrimitive);

	ucPIP_addIdentifier("", "/tmp/foo");
	ucPIP_removeIdentifier("");

	printf("%d\n",ucPIP_f_get("/tmp/foo"));

	int *foo = malloc(sizeof(int));

	*foo = ucPIP_f_get("/tmp/foo");

	GHashTable *x = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, NULL);

	g_hash_table_insert(s, foo, x);

	int *y = calloc(1, sizeof(int));
	*y = 5;

	g_hash_table_insert(x, y,  NULL);
	int *z = calloc(1, sizeof(int));
	*z = 6;

	g_hash_table_insert(x, z,  NULL);

	ucPIP_printF();
	ucPIP_printS();
}
