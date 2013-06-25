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
 * Retrieve the container ID associated with the specified identifier, i.e. return f(identifier).
 * @param identifier the identifier
 * @param create whether to create a new container in case none is found, 1 = create, 0 = do not create
 * @result the container ID associated with f(identifier). On error, 0 is returned.
 */
ucContainerID ucPIP_getContainer(ucIdentifier identifier, int create) {
	ucContainerID *contID;

	if (!identifier) {
		return (0);
	}


	if ((contID = (ucContainerID*) g_hash_table_lookup(f, identifier))) {
		return (*contID);
	}


	if (create) {
		if (!(contID = (ucContainerID*) calloc(1, sizeof(ucContainerID)))) {
			ucPIP_errorExitMemory();
		}
		*contID = ucPIP_newContainerID();
		g_hash_table_insert(f, strdup(identifier), contID);

		return (*contID);
	}

	return (0);
}


/**
 * Retrieves the data set associated with the specified container ID, i.e. s(containerID)
 * @param containerID the container of which the data set is retrieved
 * @param create whether to create a new (empty) data set, in case none is associated. 1 = create, 0 = do not create
 * @return the data set associated with containerID, NULL on error
 */
ucDataSet ucPIP_getDataSet(ucIdentifier identifier, int create) {
	ucDataSet dataSet;
	ucContainerID containerID;
	ucContainerID *containerIDCopy;

	if (!(containerID = ucPIP_getContainer(identifier, create))) {
		return NULL;
	}

	if ((dataSet = g_hash_table_lookup(s, &containerID))) {
		return (dataSet);
	}

	if (create) {
		dataSet = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, NULL);

		if (!(containerIDCopy = calloc(1, sizeof(ucContainerID)))) {
			ucPIP_errorExitMemory();
		}
		*containerIDCopy = containerID;

		g_hash_table_insert(s, containerIDCopy, dataSet);

		return (dataSet);
	}

	return NULL;
}


void ucPIP_f_remove(ucIdentifier identifier) {
	if (identifier) {
		g_hash_table_remove(f, &identifier);
	}
}


//void ucPIP_s_add(ucContainerID containerID, ucDataID* dataIDs, int count) {
//	ucDataSet dataSet;
//	ucDataID *dataIDsCopy;
//	int i;
//
//	if (!containerID || !dataIDs || count <= 0) {
//		return;
//	}
//
//	if (!(dataSet = ucPIP_getDataSet(containerID, 1))) {
//		return;
//	}
//
//	dataIDsCopy = calloc(count, sizeof(ucDataID));
//	memcpy(dataIDsCopy, dataIDs, count * sizeof(ucDataID));
//	for (i = 0; i < count; i++) {
//		g_hash_table_insert(dataSet, &dataIDsCopy[i], NULL);
//	}
//}

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
	ucContainerID contID;
	ucContainerID *contIDCopy;

	if (!oldIdentifier || !newIdentifier) {
		return;
	}

	if (!(contID = ucPIP_getContainer(oldIdentifier, 1))) {
		return;
	}

	if (!(contIDCopy = calloc(1, sizeof(ucContainerID)))) {
		ucPIP_errorExitMemory();
	}

	*contIDCopy = contID;

	g_hash_table_insert(f, strdup(newIdentifier), contIDCopy);
}





/**
 * Removes the specified identifier mapping in function f().
 */
void ucPIP_removeIdentifier(ucIdentifier identifier) {
	if (identifier) {
		g_hash_table_remove(f, identifier);
	}
}


int ucPIP_isEmptyDataSet(ucDataSet dataSet) {
	return (!dataSet || !g_hash_table_size(dataSet));
}

void ucPIP_removeDataSet(ucIdentifier identifier) {
	ucDataSet ds = ucPIP_getDataSet(identifier, 0);
	if (ds) {
		g_hash_table_remove(s, ds);
	}
}


/**
 * Copy all data in the container identified by srcIdentifier to
 * the container identified by dstIdentifier. If there is no container
 * that is identified by dstIdentifier, then one is transparently created.
 */
void ucPIP_copyData(ucIdentifier srcIdentifier, ucIdentifier dstIdentifier) {
	ucDataSet srcDataSet;
	ucDataSet dstDataSet;

	// No sensitive data in source container. Nothing to do.
	if (!(srcDataSet = ucPIP_getDataSet(srcIdentifier, 0)) || ucPIP_isEmptyDataSet(srcDataSet)) {
		return;
	}

	// Get the destination data set
	if (!(dstDataSet = ucPIP_getDataSet(dstIdentifier, 1))) {
		return;
	}


	GHashTableIter iter;
	gpointer dataID;

	// copy each and every entry from source container to destination container
	g_hash_table_iter_init(&iter, srcDataSet);
	while (g_hash_table_iter_next (&iter, &dataID, NULL)) {
		ucDataID *dataIDCopy = calloc(1, sizeof(ucDataID));
		*dataIDCopy = * (ucDataID*) dataID;
		g_hash_table_insert(dstDataSet, dataIDCopy, NULL);
	}
}



void ucPIP_printF() {
	GHashTableIter iterateIdentifiers;
	gpointer identifier, contID;

	printf("Function f():\n");

	g_hash_table_iter_init (&iterateIdentifiers, f);
	while (g_hash_table_iter_next (&iterateIdentifiers, &identifier, &contID)) {
		if (UC_PIP_PRINT_EMPTY_CONTAINERS || !ucPIP_isEmptyDataSet(ucPIP_getDataSet(identifier, 0))) {
			printf("  %60s --> %d\n", (char*) identifier, * ((ucContainerID*) contID));
		}
	}

	printf("\n");
}


void ucPIP_printS() {
	GHashTableIter iterateContainers;
	GHashTableIter iterateData;
	gpointer contID, dataSet, dataID;

	printf("Function s():\n");

	g_hash_table_iter_init (&iterateContainers, s);
	while (g_hash_table_iter_next (&iterateContainers, &contID, &dataSet)) {
		if (!ucPIP_isEmptyDataSet(dataSet)) {
			printf("  %10d --> {", *(ucContainerID*) contID);

			g_hash_table_iter_init (&iterateData, dataSet);
			while (g_hash_table_iter_next (&iterateData, &dataID, NULL)) {
				printf("%d, ", *(ucDataID*)dataID);
			}
			printf("}\n");
		}
	}


	printf("\n");
}




void ucPIP_init() {
	s = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, destroyValueHashTable);
	l = g_hash_table_new_full(g_int_hash, g_int_equal, destroyKey, destroyValueHashTable);
	f = g_hash_table_new_full(g_str_hash, g_str_equal, destroyKey, destroyValuePrimitive);

	ucPIP_addIdentifier("", "/tmp/foo");
	ucPIP_removeIdentifier("");

	printf("%d\n",ucPIP_getContainer("/tmp/foo", 1));



	int *foo = malloc(sizeof(int));

	*foo = ucPIP_getContainer("/tmp/foo", 1);

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
