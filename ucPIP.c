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

FILE *ucPIP_outstream;

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
 * @result the container ID associated with f(identifier). On error, UC_INVALID_CONTID is returned.
 */
ucContainerID ucPIP_getContainer(ucIdentifier identifier, int create) {
	ucContainerID *contID;

	if (INVALID_IDENTIFIER(identifier)) {
		return (UC_INVALID_CONTID);
	}

	contID = (ucContainerID*) g_hash_table_lookup(f, identifier);
	if (contID && VALID_CONTID(*contID)) {
		return (*contID);
	}


	if (create) {
		containerDup(contID, ucPIP_newContainerID());
		g_hash_table_insert(f, strdup(identifier), contID);

		return (*contID);
	}

	return (UC_INVALID_CONTID);
}





/**
 * Retrieves the data set associated with the specified container ID, i.e. s(containerID)
 * @param containerID the container of which the data set is retrieved
 * @param create whether to create a new (empty) data set, in case none is associated. 1 = create, 0 = do not create
 * @return the data set associated with containerID, UC_INVALID_DATASET on error
 */
ucDataSet ucPIP_getDataSet(ucIdentifier identifier, int create) {
	ucDataSet dataSet;
	ucContainerID containerID;
	ucContainerID *containerIDCopy;

	if (INVALID_CONTID(containerID = ucPIP_getContainer(identifier, create))) {
		return UC_INVALID_DATASET;
	}

	if (VALID_DATASET(dataSet = g_hash_table_lookup(s, &containerID))) {
		return (dataSet);
	}

	if (create) {
		dataSetNew(dataSet);
		containerDup(containerIDCopy, containerID);

		g_hash_table_insert(s, containerIDCopy, dataSet);

		return (dataSet);
	}

	return UC_INVALID_DATASET;
}

ucAliasSet ucPIP_getAliasSet(ucIdentifier identifier, int create) {
	ucAliasSet aliasSet;
	ucContainerID containerID;
	ucContainerID *containerIDCopy;

	if (INVALID_CONTID(containerID = ucPIP_getContainer(identifier, create))) {
		return UC_INVALID_ALIASSET;
	}

	if (VALID_ALIASSET(aliasSet = g_hash_table_lookup(l, &containerID))) {
		return (aliasSet);
	}

	if (create) {
		aliasSetNew(aliasSet);
		containerDup(containerIDCopy, containerID);

		g_hash_table_insert(l, containerIDCopy, aliasSet);

		return (aliasSet);
	}

	return UC_INVALID_ALIASSET;
}


/**
 * Adds a new (additional) identifier for the container that has so far
 * been identified with the specified old identifier. If the old identifier
 * is not yet associated with a container, then a new container is created
 * transparently and it is assigned to both oldIdentifier and newIdentifier.
 * In any case, the identified container is returned; UC_INVALID_CONTID on error.
 *
 * If oldIdentifier == newIdentifier or
 * if oldIdentifier == NULL and newIdentifier != NULL or
 * if newIdentifier == NULL and oldIdentifier != NULL,
 * then we have in fact only one identifier. In this case
 * it is checked whether a container for that identifier already exists.
 * If this is the case, then it is returned. If not, then a container will be
 * transparently created and assigned the corresponding identifier.
 *
 * @param oldIdentifier the old identifier identifying the container
 * @param newIdentifier the new (additional) identifier for that same container
 * @return the identified container or UC_INVALID_CONTID on error
 */
ucContainerID ucPIP_addIdentifier(ucIdentifier oldIdentifier, ucIdentifier newIdentifier) {
	ucContainerID contID;
	ucContainerID *contIDCopy;

	if (INVALID_IDENTIFIER(oldIdentifier) && INVALID_IDENTIFIER(newIdentifier)) {
		return (UC_INVALID_CONTID);
	}
	else if (INVALID_IDENTIFIER(oldIdentifier)) {
		oldIdentifier = newIdentifier;
	}
	else if (INVALID_IDENTIFIER(newIdentifier)) {
		newIdentifier = oldIdentifier;
	}

	if (INVALID_CONTID(contID = ucPIP_getContainer(oldIdentifier, 1))) {
		return (UC_INVALID_CONTID);
	}

	containerDup(contIDCopy, contID);

	g_hash_table_insert(f, strdup(newIdentifier), contIDCopy);

	return (contID);
}








/*
 * Removes a container and its associated data set completely.
 */
void ucPIP_removeContainer(ucIdentifier identifier) {
	ucContainerID cont;

	if (VALID_CONTID(cont = ucPIP_getContainer(identifier, 0))) {
		// this will automatically destroy the hashtable associated with that container
		g_hash_table_remove(s, &cont);
	}
}




/**
 * Removes the specified identifier mapping in function f().
 * If this is the last identifier for the container identified by this identifier,
 * then this container will be transparently removed as well.
 */
void ucPIP_removeIdentifier(ucIdentifier identifier) {
	// Also remove the container if this was the last identifier for it.

	// TODO: This operation is expensive, because countIdentifiers() loops over all identifiers
	if (ucPIP_countIdentifiers(identifier) == 1) {
		ucPIP_removeAllAliasesTo(identifier);
		ucPIP_removeAllAliasesFrom(identifier);
		ucPIP_removeContainer(identifier);
	}

	if (VALID_IDENTIFIER(identifier)) {
		g_hash_table_remove(f, identifier);
	}
}

/**
 * Clone all aliases of the container identified by srcIdentifier to
 * the container identified by dstIdentifier. If no container is identified
 * with srcIdentifier, then nothing will happen. If there is no container
 * that is identified by dstIdentifier, then one is transparently created.
 */
void ucPIP_copyAliases(ucIdentifier srcIdentifier, ucIdentifier dstIdentifier) {
	ucAliasSet srcAliasSet;
	ucAliasSet dstAliasSet;
	GHashTableIter iter;
	gpointer aliasedContainer;
	ucContainerID *aliasedContainerCopy;

	// No aliases in source container. Nothing to do.
	if (INVALID_ALIASSET(srcAliasSet = ucPIP_getAliasSet(srcIdentifier, 0)) || aliasSetIsEmpty(srcAliasSet)) {
		return;
	}

	// Get the destination alias set
	if (INVALID_ALIASSET(dstAliasSet = ucPIP_getAliasSet(dstIdentifier, 1))) {
		return;
	}


	// copy each and every entry from source container to destination container
	g_hash_table_iter_init(&iter, srcAliasSet);
	while (g_hash_table_iter_next (&iter, &aliasedContainer, NULL)) {
		containerDup(aliasedContainerCopy, * (ucContainerID*) aliasedContainer);
		aliasSetAdd(dstAliasSet, aliasedContainerCopy);
	}
}



/**
 * Find out which containers have an alias to the container identified by stencil. All
 * these containers will also alias the container identified by stenciled.
 */
void ucPIP_alsoAlias(ucIdentifier stencilIdentifier, ucIdentifier stenciledIdentifier) {
	GHashTableIter iter;
	ucContainerID stencilCont;
	ucContainerID stenciledCont;
	ucContainerID *stenciledContCopy;
	ucAliasSet aliasSet;

	if (INVALID_CONTID(stencilCont = ucPIP_getContainer(stencilIdentifier, 0))) {
		return;
	}

	if (INVALID_CONTID(stenciledCont = ucPIP_getContainer(stenciledIdentifier, 0))) {
		return;
	}

	g_hash_table_iter_init(&iter, l);
	while (g_hash_table_iter_next (&iter, (void **)&aliasSet, NULL)) {
		if (g_hash_table_contains(aliasSet, &stencilCont)) {
			containerDup(stenciledContCopy, stenciledCont);
			aliasSetAdd(aliasSet, stenciledContCopy);
		}
	}
}


/**
 * Copy all data in the container identified by srcIdentifier to
 * the container identified by dstIdentifier. If no container is identified
 * with srcIdentifier, then nothing will happen. If there is no container
 * that is identified by dstIdentifier, then one is transparently created.
 *
 * If retDataSet != NULL, then this data set will be populated with all data
 * that has been copied, i.e. with all data that is/was in the container identified
 * by srcIdentifier.
 */
void ucPIP_copyData(ucIdentifier srcIdentifier, ucIdentifier dstIdentifier, ucDataSet retDataSet) {
	ucDataSet srcDataSet;
	ucDataSet dstDataSet;
	GHashTableIter iter;
	gpointer dataID;
	ucDataID *dataIDCopy;
	ucDataID *dataIDCopy2;

	// No sensitive data in source container. Nothing to do.
	if (INVALID_DATASET(srcDataSet = ucPIP_getDataSet(srcIdentifier, 0)) || dataSetIsEmpty(srcDataSet)) {
		return;
	}

	// Get the destination data set
	if (INVALID_DATASET(dstDataSet = ucPIP_getDataSet(dstIdentifier, 1))) {
		return;
	}


	// copy each and every entry from source container to destination container
	g_hash_table_iter_init(&iter, srcDataSet);
	while (g_hash_table_iter_next (&iter, &dataID, NULL)) {
		dataDup(dataIDCopy, * (ucDataID*) dataID);
		dataSetAdd(dstDataSet, dataIDCopy);

		// also populate the return data set, if provided
		if (VALID_DATASET(retDataSet)) {
			dataDup(dataIDCopy2, * (ucDataID*) dataID);
			dataSetAdd(retDataSet, dataIDCopy2);
		}
	}
}



void ucPIP_getAllReflexivelyAliasedDataSets_rec(ucContainerID contID, ucDataSet *datasets, ucContainerID *collectedContainers, int *count) {
	ucDataSet ds;
	ucAliasSet as;
	int i;

	if (INVALID_CONTID(contID)) {
		return;
	}

	// return if that container was already added
	for (i = 0; i < *count; i++) {
		if (EQUAL_CONTID(collectedContainers[i], contID)) {
			return;
		}
	}

	if (INVALID_DATAID(ds = g_hash_table_lookup(s, &contID))) {
		return;
	}

	// add this container's data set
	(*count)++;
	datasets = realloc(datasets, *count * sizeof(ucDataSet));
	collectedContainers = realloc(collectedContainers, *count * sizeof(ucContainerID));
	datasets[*count - 1] = ds;
	collectedContainers[*count - 1] = contID;

	if (INVALID_ALIASSET(as = g_hash_table_lookup(l, &contID))) {
		return;
	}

	// go over all its aliases and add them recursively
	GHashTableIter iter;
	gpointer aliasedContainer;
	g_hash_table_iter_init(&iter, as);
	while (g_hash_table_iter_next (&iter, &aliasedContainer, NULL)) {
		ucPIP_getAllReflexivelyAliasedDataSets_rec( *(ucContainerID*) aliasedContainer, datasets, collectedContainers, count);
	}
}


/**
 * Returns all data sets that are reflexively aliased by the identified container.
 * The data sets are returned and the corresponding memory needs to be freed by the caller using free().
 * count specified how many entries there are.
 *
 * This functions returns NULL if no data sets were returned (i.e. if count == 0),
 * or a pointer to that dataset.
 *
 * Recommended usage:
 * int c;
 * ucIdentifier ident;
 * ucDataSet *dataset = ucPIP_getAllReflexivelyAliasedDataSets(ident, &c);
 *
 * for (int i = 0; i < c; i++) {
 *   doSth();
 * }
 * free(dataset);
 *
 */
ucDataSet *ucPIP_getAllReflexivelyAliasedDataSets(ucIdentifier identifier, int *count) {
	ucContainerID cont;
	*count = 0;


	if (INVALID_CONTID(cont = ucPIP_getContainer(identifier, 0))) {
		return NULL;
	}

	ucDataSet *datasets = NULL;
	ucPIP_getAllReflexivelyAliasedDataSets_rec(cont, datasets, NULL, count);

	if (datasets == NULL && *count == 0) {
		return NULL;
	}

	if (datasets != NULL && *count > 0) {
		return (datasets);
	}

	ucPIP_errorExit("Something went wrong when trying to get reflexively aliased data sets.");
	return NULL;
}


/**
 * Delete all aliases _from_ the container specified by identifier.
 */
void ucPIP_removeAllAliasesFrom(ucIdentifier identifier) {
	ucContainerID cont;

	if (VALID_CONTID(cont = ucPIP_getContainer(identifier, 0))) {
		// this will automatically destroy the hashtable associated with that container
		g_hash_table_remove(l, &cont);
	}
}


/**
 * Delete all aliases _to_ the container specified by identifier.
 */
void ucPIP_removeAllAliasesTo(ucIdentifier identifier) {
	ucContainerID cont;
	GHashTableIter iterAliasSets;
	ucAliasSet aliasSet;

	if (INVALID_CONTID(cont = ucPIP_getContainer(identifier, 0))) {
		return;
	}

	g_hash_table_iter_init(&iterAliasSets, l);
	while (g_hash_table_iter_next (&iterAliasSets, NULL, (void **) &aliasSet)) {
		g_hash_table_remove(aliasSet, &cont);
	}
}



/**
 * Adds an alias from the container identified by identifierFrom the
 * container identified by identifierTo. Both containers must exist,
 * otherwise nothing will happen.
 */
void ucPIP_addAlias(ucIdentifier identifierFrom, ucIdentifier identifierTo) {
	ucContainerID containerTo;
	ucContainerID *containerToCopy;
	ucAliasSet aliasSet;

	if (INVALID_CONTID(containerTo = ucPIP_getContainer(identifierTo, 0))) {
		return;
	}

	if (INVALID_ALIASSET(aliasSet = ucPIP_getAliasSet(identifierFrom, 1))) {
		return;
	}

	containerDup(containerToCopy, containerTo);
	aliasSetAdd(aliasSet, containerToCopy);
}



/*
 * Adds a new initial data ID to the container identified by the specified identifier
 * and returns the added data ID. In case no such container is found, then it will
 * be transparently created.
 * @param identifier the container's identifier to which a new data ID is added.
 * @return the added data ID or UC_INVALID_DATAID on error
 */
ucDataID ucPIP_addInitialData(ucIdentifier identifier) {
	ucDataSet dataSet;
	ucDataID *dataID = UC_INVALID_DATAID;

	if (VALID_DATASET(dataSet = ucPIP_getDataSet(identifier, 1))) {
		dataDup(dataID, ucPIP_newDataID());
		dataSetAdd(dataSet, dataID);
	}

	return (*dataID);
}


/**
 * Count the number of identifiers associated with the container
 * that is identified by the specified identifier.
 * @param identifier one of the identifiers of the container whose number of identifiers is to be counted
 * @return the total number of identifiers identifying this container
 */
int ucPIP_countIdentifiers(ucIdentifier identifier) {
	ucContainerID container;
	GHashTableIter iter;
	int count = 0;
	GList *li;

	// TODO: This may be more efficient by implementing f^-1, ie f_rev ==> a reverse lookup hashtable for f.
	// Here we iterate over all values in that table :-(
	if (VALID_CONTID(container = ucPIP_getContainer(identifier, 0))) {
		GList *conts = g_hash_table_get_values(f);

		for (li = conts; li != NULL; li = g_list_next(li)) {
			if (* (ucContainerID*) li->data == container) {
		        count++;
			}
		}
		g_list_free(conts);
	}

	return (count);
}


void ucPIP_printF_impl() {
	GHashTableIter iterateIdentifiers;
	gpointer identifier, contID;

	fprintf(ucPIP_outstream, "Function f():\n");

	g_hash_table_iter_init (&iterateIdentifiers, f);
	while (g_hash_table_iter_next (&iterateIdentifiers, &identifier, &contID)) {
		if (UC_PIP_PRINT_EMPTY_CONTAINERS || !dataSetIsEmpty(ucPIP_getDataSet(identifier, 0))) {
			fprintf(ucPIP_outstream, "  %60s --> %d\n", (char*) identifier, * ((ucContainerID*) contID));
		}
	}

	fprintf(ucPIP_outstream, "\n");
	fflush(ucPIP_outstream);
}


void ucPIP_printS_impl() {
	GHashTableIter iterateContainers;
	GHashTableIter iterateData;
	gpointer contID, dataSet, dataID;

	fprintf(ucPIP_outstream, "Function s():\n");

	g_hash_table_iter_init (&iterateContainers, s);
	while (g_hash_table_iter_next (&iterateContainers, &contID, &dataSet)) {
		if (!dataSetIsEmpty(dataSet)) {
			fprintf(ucPIP_outstream, "  %10d --> ", *(ucContainerID*) contID);
			dataSetPrint(ucPIP_outstream, dataSet);
			fprintf(ucPIP_outstream, "\n");
		}
	}

	fprintf(ucPIP_outstream, "\n");
	fflush(ucPIP_outstream);
}




void ucPIP_init() {
	s = g_hash_table_new_full(g_int_hash, g_int_equal, free, (GDestroyNotify) g_hash_table_destroy);
	l = g_hash_table_new_full(g_int_hash, g_int_equal, free, (GDestroyNotify) g_hash_table_destroy);
	f = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
	ucPIP_outstream = stdout;
}


void dataSetPrint(FILE *f, ucDataSet set) {
	GHashTableIter iter;
	gpointer dataID;
	int count;

	if (!f || !set || INVALID_DATASET(set)) {
		return;
	}

	fprintf(f, "{");

	g_hash_table_iter_init (&iter, set);
	for (count = 0; count < g_hash_table_size(set) - 1; count++) {
		g_hash_table_iter_next (&iter, &dataID, NULL);
		fprintf(f, "%d, ", *(ucDataID*)dataID);
	}

	g_hash_table_iter_next (&iter, &dataID, NULL);
	fprintf(f, "%d", *(ucDataID*)dataID);

	fprintf(f, "}");
}
