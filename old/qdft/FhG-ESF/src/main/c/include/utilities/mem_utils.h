/**
 * @file  mem_utils.h
 * @brief Custom memory allocation
 *
 * @author Ricardo Neisse
 **/

#ifndef _mem_utils_h
#define _mem_utils_h

#include <stdlib.h>
#include "base.h"

void *mem_alloc(size_t size);
void *mem_calloc(size_t nmemb, size_t size);
void  mem_free(void *ptr);

#endif
