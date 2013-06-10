/*
 * event.h
 *
 *  Created on: Jun 10, 2013
 *      Author: user
 */

#ifndef EVENT_H_
#define EVENT_H_

#include <glib.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <glib/gprintf.h>


// Structure used to describe an event
// To allocate params array use:
// params = (param_t *)malloc(sizeof(param_t) * num_params)
typedef struct event_s {
  GTree *params;
} event_t;

typedef event_t* event_ptr;


event_ptr eventCreate();
void eventDestroy();
void eventAddParam(event_ptr event, char *key, char *value);
void eventDeleteParam(event_ptr event, char *key);

void eventPrint(event_ptr event);

void eventDestroyParamKey(gpointer data);
void eventDestroyParamValue(gpointer value);

#endif /* EVENT_H_ */
