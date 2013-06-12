/*
 * str_utils.h
 *
 *      Author: Ricardo Neisse, Cornelius Moucha
 */

#ifndef str_utils_h
#define str_utils_h

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "base.h"

typedef struct char_buffer_s {
  int block_size;
  char *buffer;
} char_buffer;

int substr(char *str1, char *str2);

void trim(char *s);

int count_non_empty(char *s);
unsigned char *byteToBinary(unsigned int);

#endif
