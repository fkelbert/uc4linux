/*
 * param.h
 *
 *  Created on: Jun 10, 2013
 *      Author: user
 */

#ifndef PARAM_H_
#define PARAM_H_

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

// Structure used to describe an event parameter
typedef struct param_s {
  char *name;
  char *value;
} param_t;

typedef param_t* param_ptr;

param_ptr paramCreate(char *name, char* value);
void paramDestroy(param_ptr param);


#endif /* PARAM_H_ */
