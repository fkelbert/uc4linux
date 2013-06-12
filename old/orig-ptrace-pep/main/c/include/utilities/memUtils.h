/**
 * @file  memUtils.h
 * @brief memory allocation; intended for later usage in memory profiling
 *
 * @author cornelius moucha
 **/

#ifndef _memUtils_h
#define _memUtils_h

#include <stdlib.h>
#ifndef __WIN32__
  #include <fcntl.h>
  #include <sys/mman.h>
  #include <sys/stat.h>
#endif
#include "base.h"

/**
 * Request a memory slice of the given size
 *
 * @param   size      requested size of memory slice
 * @return  the allocated memory slice on success or NULL otherwise
 */
void *memAlloc(size_t size);

/**
 * Request nmemb times a memory slice of the given size
 *
 * @param   nmemb     the amount of memory slices
 * @param   size      the size of one memory slice for allocation
 * @return  the allocated memory slice on success or NULL otherwise
 */
void *memCalloc(size_t nmemb, size_t size);

/**
 * Deallocates an memory pointer\n
 *
 * @param   ptr      the memory pointer for deallocation
 */
void  memFree(void *ptr);

/**
 * Provide a mapping of the file content given by filename in memory\n
 * Implementation is platform-dependent
 *
 * @param   filename   the filename for the memory mapping
 * @return  the prepared file mapping in the memory on success or NULL otherwise
 */
char *memMapFile(char *filename);

#endif
