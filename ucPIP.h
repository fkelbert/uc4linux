/*
 * ucPIP.h
 *
 *  Created on: Jun 21, 2013
 *      Author: Florian Kelbert
 *
 * A PIP.
 *
 * Big thing: this code is independent from strace.
 */

#ifndef UCPIP_H_
#define UCPIP_H_

#include <stdlib.h>
#include <stdio.h>
#include <glib.h>
#include <string.h>

#define UC_PIP_PRINT_EMPTY_CONTAINERS 1

typedef guint ucDataID;
typedef guint ucContainerID;
typedef	gchar* ucIdentifier;
typedef GHashTable* ucDataSet;
typedef GHashTable* ucAliasSet;

#define UC_INVALID_CONTID 0
#define UC_INVALID_DATAID 0
#define UC_INVALID_IDENTIFIER NULL
#define UC_INVALID_DATASET NULL
#define UC_INVALID_ALIASSET NULL

#define INVALID_CONTID(contid) ((contid) == UC_INVALID_CONTID)
#define VALID_CONTID(contid) ((contid) != UC_INVALID_CONTID)

#define INVALID_DATAID(dataid) ((dataid) == UC_INVALID_DATAID)
#define VALID_DATAID(dataid) ((dataid) != UC_INVALID_DATAID)

#define INVALID_IDENTIFIER(ident) ((ident) == UC_INVALID_IDENTIFIER)
#define VALID_IDENTIFIER(ident) ((ident) != UC_INVALID_IDENTIFIER)

#define INVALID_DATASET(dataset) ((dataset) == UC_INVALID_DATASET)
#define VALID_DATASET(dataset) ((dataset) != UC_INVALID_DATASET)

#define INVALID_ALIASSET(aliasset) ((aliasset) == UC_INVALID_ALIASSET)
#define VALID_ALIASSET(aliasset) ((aliasset) != UC_INVALID_ALIASSET)


void			ucPIP_init();

#define dataSetNew(set) set = g_hash_table_new_full(g_int_hash, g_int_equal, free, NULL)
#define dataSetAdd(set, value) g_hash_table_insert(set, value, NULL)
void dataSetPrint(FILE *f, ucDataSet set);

#define aliasSetNew(set) dataSetNew(set)
#define aliasSetAdd(set, value) dataSetAdd(set, value)

#define containerDup(contPtr, value) if (!(contPtr = (ucContainerID*) malloc(sizeof(ucContainerID)))) { ucPIP_errorExitMemory(); } *contPtr = value
#define dataDup(dataPtr, value) if (!(dataPtr = (ucDataID*) malloc(sizeof(ucDataID)))) { ucPIP_errorExitMemory(); } *dataPtr = value

ucDataID		ucPIP_newDataID();
ucContainerID	ucPIP_newContainerID();



void destroyKey(gpointer data);

void destroyValueHashTable(gpointer data);

void destroyValuePrimitive(gpointer data);

#define ucPIP_errorExit(msg) \
			fprintf(stderr, "%s\n", msg); \
			fprintf(stderr, "Happened in: %s:%d\n", __FILE__, __LINE__); \
			fprintf(stderr, "Exiting.\n"); \
			exit (1);

#define ucPIP_errorExitMemory() ucPIP_errorExit("Unable to allocate enough memory")

#endif /* UCPIP_H_ */

