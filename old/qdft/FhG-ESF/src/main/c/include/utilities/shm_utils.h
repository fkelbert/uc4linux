#pragma once
#include <sys/ipc.h>
#include <sys/shm.h>

typedef struct shm_s
{
  key_t key;
  int sharedMemoryID;
  char *shmSegment;
} shm_t;
typedef shm_t *shm_ptr;
