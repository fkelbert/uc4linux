/*
 * event.c
 *
 *  Created on: Jun 10, 2013
 *      Author: user
 */


#include "event.h"
#include "param.h"

gint mystrcmp(gconstpointer a, gconstpointer b, gpointer user) {
	return (g_strcmp0(a,b));
}

event_ptr eventCreate() {
	event_ptr event = calloc(1, sizeof(struct event_s));

	if (event != NULL) {
		event->params = g_tree_new_full(mystrcmp, NULL, eventDestroyParamKey, eventDestroyParamValue);
	}


	return (event);
}


void eventDestroy(event_ptr event) {
	if (event) {
		g_tree_destroy(event->params);
		free(event);
	}
}


void eventAddParam(event_ptr event, char *key, char *value) {
	g_tree_insert(event->params, strdup(key), strdup(value));
}

void eventDeleteParam(event_ptr event, char *key) {
	g_tree_remove(event->params, key);
}


void eventDestroyParamKey(gpointer data) {
	if (data) free(data);
}

void eventDestroyParamValue(gpointer value) {
	if (value) free(value);
}

int eventParamPrint(gpointer key, gpointer value, gpointer data) {
	printf(" %s --> %s\n", (char*) key, (char*) value);
	return (0);		// traversal is stopped in case something different from 0 is returned
}

char *eventGetParam(event_ptr event, char *key) {
	return (g_tree_lookup(event->params, key));
}

void eventPrint(event_ptr event) {
	if (event) {
		printf("syscall\n");
		g_tree_foreach(event->params, (GTraverseFunc) eventParamPrint, NULL);
	}
}

