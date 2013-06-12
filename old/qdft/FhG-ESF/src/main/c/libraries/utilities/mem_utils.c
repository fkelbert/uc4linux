/**
 * @file
 * @brief
 *
 * @author Ricardo Neisse
 **/
#include "mem_utils.h"
#include "log_mem_utils_c.h"

uint64_t mem_usage = 0;

void * mem_alloc(size_t size) {
  // TODO: keep track of all memory allocation and deallocation
  // to identify memory leaks.
  mem_usage += size;
  return malloc(size);
}

void * mem_calloc(size_t nmemb, size_t size) {
  // TODO: keep track of all memory allocation and deallocation
  // to identify memory leaks.
  mem_usage += (nmemb*size);
  return calloc(nmemb, size);
}

void mem_free(void *ptr) {
  // Do not free null pointers
  if (ptr!=NULL)
  {
    free(ptr);
  }
}
