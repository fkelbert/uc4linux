/**
 * @file  logInternal.c
 * @brief Implementation of PEF logger module; internal initialization and destruction methods
 * @see logger.h
 *
 * @author cornelius moucha
 **/

#include "logger.h"

extern pefMutex_ptr logmtx;

#ifndef __WIN32__
  void logInit()
  {
    logmtx=(pefMutex_ptr)memAlloc(sizeof(pefMutex_t));
    if(logmtx==NULL) {fprintf(stderr, "Could not allocate memory for pefMutex structure!"); return;}

    logmtx->name=strdup("pefLogger");
    logmtx->lmutex=(pthread_mutex_t*)memAlloc(sizeof(pthread_mutex_t));
    if(pthread_mutex_init(logmtx->lmutex, NULL)!=0) {fprintf(stderr, "Could not create new mutex!"); return;}

    fprintf(stdout, "Successfully created mutex [%s]\n", logmtx->name);
    fflush(stdout);
  }
  void logDestroy()
  {
    if(logmtx!=NULL)
    {
      int ret=0;
      if((ret=pthread_mutex_destroy(logmtx->lmutex))!=0) fprintf(stderr, "Error deallocating logger mutex [%d]!", ret);
      free(logmtx->lmutex);
      free(logmtx->name);
      free(logmtx);

      fprintf(stdout, "Logger mutex successfully deallocated.\n");
    }
  }
  void loggerMutexLock()   {pthread_mutex_lock(logmtx->lmutex);}
  void loggerMutexUnlock() {pthread_mutex_unlock(logmtx->lmutex);}
#else
  void logInit()
  {
    logmtx=(pefMutex_ptr)memAlloc(sizeof(pefMutex_t));
    if(logmtx==NULL) {fprintf(stderr, "Could not allocate memory for pefMutex structure!"); return;}

    logmtx->name=strdup("pefLogger");
    logmtx->lmutex=CreateMutex(NULL, FALSE, NULL);
    if(logmtx==NULL) {fprintf(stderr, "Could not create new mutex!"); return;}
    fprintf(stdout, "Successfully created mutex [%s]\n", logmtx->name);
    fflush(stdout);
  }

  void logDestroy()
  {
    if(logmtx!=NULL)
    {
      int ret=0;
      ret=CloseHandle(logmtx->lmutex);
      if(ret==0) fprintf(stderr, "Error deallocating mutex");
      free(logmtx->name);
      free(logmtx);
      fprintf(stdout, "Logger mutex successfully deallocated.\n");
    }
  }
  void loggerMutexLock()   {WaitForSingleObject(logmtx->lmutex, INFINITE);}
  void loggerMutexUnlock() {ReleaseMutex(logmtx->lmutex);}
#endif


