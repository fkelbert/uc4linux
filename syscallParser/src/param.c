/*
 * param.c
 *
 *  Created on: Jun 10, 2013
 *      Author: user
 */

#include "param.h"


param_ptr paramCreate(char *name, char *value) {
	param_ptr param = calloc(1, sizeof(param_t));

	if (param != NULL) {
		param->name = strdup(name);
		param->value = strdup(value);
	}

	return (param);
}

void paramDestroy(param_ptr param) {
	free(param->name);
	free(param->value);
	free(param);
}
