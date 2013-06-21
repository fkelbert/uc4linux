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

typedef guint ucDataID;
typedef unsigned short ucContainerID;
typedef	gchar* ucIdentifier;

void			ucPIP_init();

void 			ucPIP_s_add(ucContainerID, ucDataID*, int count);
ucDataID*		ucPIP_s_get(ucContainerID, int *count);
void			ucPIP_s_remove(ucContainerID);

void 			ucPIP_l_add(ucContainerID, ucContainerID*, int count);
ucContainerID*	ucPIP_l_get(ucContainerID, int *count);
void			ucPIP_l_remove(ucContainerID);

ucContainerID	ucPIP_f_add(ucIdentifier, ucContainerID);
ucContainerID	ucPIP_f_get(ucIdentifier);
void 			ucPIP_f_remove(ucIdentifier);

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

#endif /* UCPIP_H_ */

