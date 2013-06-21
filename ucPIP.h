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

#include <stdio.h>
#include <glib.h>

typedef unsigned long ucDataID;
typedef unsigned long ucContainerID;
typedef	char* ucIdentifier;

void 			ucPIP_s_add(ucContainerID, ucDataID*, int count);
ucDataID*		ucPIP_s_get(ucContainerID, int *count);
void			ucPIP_s_remove(ucContainerID);

void 			ucPIP_l_add(ucContainerID, ucContainerID*, int count);
ucContainerID*	ucPIP_l_get(ucContainerID, int *count);
void			ucPIP_l_remove(ucContainerID);

void 			ucPIP_f_add(ucIdentifier, ucContainerID);
ucContainerID	ucPIP_f_get(ucIdentifier);
void 			ucPIP_f_remove(ucIdentifier);

ucDataID		ucPIP_newDataID();
ucContainerID	ucPIP_newContainerID();

#endif /* UCPIP_H_ */

