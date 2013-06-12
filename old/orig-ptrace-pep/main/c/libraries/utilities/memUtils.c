/**
 * @file  memUtils.c
 * @brief memory allocation; intended for later usage in memory profiling
 *
 * @author cornelius moucha
 **/

#include "memUtils.h"
#include "log_memUtils_pef.h"

/// the global amount of allocated memory; intended for later usage in memory profiling
uint64_t totalMemoryUsage=0;

void *memAlloc(size_t size)
{
  //totalMemoryUsage+=size;
  return malloc(size);
}

void* memCalloc(size_t nmemb, size_t size)
{
  //totalMemoryUsage+=(nmemb*size);
  return calloc(nmemb, size);
}

void memFree(void *ptr)
{
  //totalMemoryUsage-=???;
  if(ptr!=NULL) free(ptr);
}

#ifndef __WIN32__
  char *memMapFile(char *filename)
  {
    checkNullPtr(filename, "Cannot map a NULL-file into memory!");

    int fd;
    char *map;
    struct stat filestat;

    fd=open(filename, O_RDONLY);
    if(fd==-1) {perror("Error opening file for reading"); return NULL;}
    fstat(fd, &filestat);

    map=mmap(0, filestat.st_size, PROT_READ, MAP_SHARED, fd, 0);
    if(map==MAP_FAILED)
    {
      close(fd); perror("Error mmapping the file");
      return NULL;
    }
    close(fd);
    log_debug("Successfully mapped file into memory");
    return map;
  }
#else
  char *memMapFile(char *filename)
  {
    checkNullPtr(filename, "Cannot map a NULL-file into memory!");

    HANDLE hFile=CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
    if(hFile==INVALID_HANDLE_VALUE) {log_error("Error opening the file for mapping"); return NULL;}

    DWORD fileSizeHigh=0;
    DWORD fileSizeLow=GetFileSize(hFile, &fileSizeHigh);
    log_debug("FileSize of file for mapping [%d, %d]\n", fileSizeHigh, fileSizeLow);

    char *ret=NULL;
    HANDLE hFileMap=CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 1, NULL);
    if(hFileMap)
    {
      ret=MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
      if(ret==NULL) {log_error("Error mapping the file!"); return NULL;}
    }
    else {log_error("Error creating a mapping object"); return NULL;}
    CloseHandle(hFileMap);
    CloseHandle(hFile);

    //int a;
    //for(a=0; a<fileSizeLow; a++)
    //  printf("%c", ret[a]);
    //printf("\n");

    // Cleanup if mapping not needed any more
    //UnmapViewOfFile(ret);
    return ret;
  }
#endif
