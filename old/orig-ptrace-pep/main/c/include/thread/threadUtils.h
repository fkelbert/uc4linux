/**
 * @file  threadUtils.h
 * @brief Auxiliary utilities for thread and mutex handling
 *
 * Auxiliary utilities for thread and mutex handling\n
 * Provide platform depended implementation for thread handling and mutexes.
 *
 * @author cornelius moucha
 **/

#ifndef THREADUTILS_H_
#define THREADUTILS_H_

#include <unistd.h>
#include "base.h"
#include "memUtils.h"

typedef struct pefThread_s  pefThread_t;
typedef pefThread_t        *pefThread_ptr;

typedef struct pefMutex_s   pefMutex_t;
typedef pefMutex_t         *pefMutex_ptr;

#ifdef __WIN32__
  // using Windows API for thread and mutex management...

  /// WinAPI expectd DWORD (unsigned long) as return value of handler method
  #define THREADRETURN 0;

  /**
   * @struct pefThread_s
   * @brief Structure for thread handling
  **/
  struct pefThread_s
  {
    HANDLE                 lthread;
    LPTHREAD_START_ROUTINE handler;
    void*                  userData;

    DWORD                  threadID;
    uint64_t               sleepValue;
    uint64_t               (*adjustSleepValue)(pefThread_ptr thread, uint64_t sleepValue);
    void                   (*sleep)(DWORD sleepValue);
    unsigned int           (*prepareThreadKill)(void* exitHandler);
    unsigned int           (*start)(pefThread_ptr thread);
  };

  /**
   * @struct pefMutex_s
   * @brief Structure for mutex handling
  **/
  struct pefMutex_s
  {
    HANDLE       lmutex;
    char         *name;
    unsigned int (*lock)(pefMutex_ptr mutex, unsigned long timeout);
    unsigned int (*unlock)(pefMutex_ptr mutex);
  };
#else

  // using pthread for thread and mutex management...
  #include <pthread.h>
  #include <signal.h>

  typedef void* (*LPTHREAD_START_ROUTINE)(void*);
  #define THREADRETURN NULL;

  struct pefThread_s
  {
    pthread_t              lthread;
    LPTHREAD_START_ROUTINE handler;
    void*                  userData;

    unsigned long          threadID;
    uint64_t               sleepValue;
    uint64_t               (*adjustSleepValue)(pefThread_ptr thread, uint64_t sleepValue);
    void                   (*sleep)(unsigned long sleepValue);
    unsigned int           (*prepareThreadKill)(void* exitHandler);
    unsigned int           (*start)(pefThread_ptr thread);

    //unsigned int           (*suspend)(pefThread_ptr thread);
    //unsigned int           (*resume)(pefThread_ptr thread);
    //unsigned int           suspendVal;
    //pthread_mutex_t        suspendMutex;;
    //pthread_cond_t         suspendCondition;
    //LPTHREAD_START_ROUTINE sleepHandler;
    //LPTHREAD_START_ROUTINE workHandler;
  };

  #define INFINITE 0
  struct pefMutex_s
  {
    pthread_mutex_t *lmutex;
    char            *name;
    unsigned int    (*lock)(pefMutex_ptr mutex, unsigned long timeout);
    unsigned int    (*unlock)(pefMutex_ptr mutex);
  };

#endif

/**
 * Creates a new thread structure\n
 * Note: The created thread is not started automatically! It has to be started with startThread, otherwise
 *       handling routine may already access thread structure which is not yet returned to the thread invoking
 *       pefThreadNew!
 *
 * @return  reference to created thread structure on success or NULL otherwise
**/
pefThread_ptr pefThreadNew();

/**
 * Deallocates memory of a thread structure\n
 *
 * @param   thread    thread structure for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  pefThreadFree(pefThread_ptr thread);

/**
 * Adjusts the thread's sleeping value. This is required because Window's usleep-implementation only
 * works with sleep-values up to 999.999 microseconds. Above that the thread will not sleep at all! Therefore
 * The sleeping will be done with Sleep instead of usleep in this case.
 *
 * @param   thread     thread structure for deallocation
 * @param   sleepValue the desired amount of microseconds the thread should sleep
 * @return  the modified value of time the thread will sleep
**/
uint64_t      adjustSleepValue(pefThread_ptr thread, uint64_t sleepValue);

/**
 * Sleeps the given amount of microseconds
 * @param   sleepValue     the amount of microseconds the thread should sleep
**/
void          threadSleepMicro(unsigned long sleepValue);

/**
 * Sleeps the given amount of milliseconds
 * @param   sleepValue     the amount of milliseconds the thread should sleep
**/
void          threadSleepMilli(unsigned long sleepValue);

/**
 * Terminate the thread execution. This is done either with sending a SIGUSR to the thread or in Windows with
 * simply terminating the thread.
 * @param   thread     reference to the thread for terminating
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  pefThreadKill(pefThread_ptr thread);

/**
 * Prepare the required methods etc. for a later termination of the thread, i.e. register signal handler
 * @param   exitHandler     function pointer of the handler method for receiving SIGUSR signals
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  prepareThreadKill(void* exitHandler);

//unsigned int suspendThread(pefThread_ptr thread);
//unsigned int resumeThread(pefThread_ptr thread);
//unsigned int isSuspended(pefThread_ptr thread);
//void threadSuspend();

/**
 * Starts the given thread execution
 * @param   thread     reference to the thread which should be started
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int startThread(pefThread_ptr thread);


/**
 * Creates a new mutex structure\n
 *
 * @param   name     name of the mutex; used for logging output
 * @return  reference to created mutex structure on success or NULL otherwise
**/
pefMutex_ptr  pefMutexNew(const char *name);

/**
 * Deallocates memory of a mutex structure\n
 * @param   mutex    mutex structure for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  pefMutexFree(pefMutex_ptr mutex);

/**
 * Locks the given mutex
 * @param   mutex    mutex structure for deallocation
 * @param   timeout  timeout for the locking; only working in Windows
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  mutexLock(pefMutex_ptr mutex, unsigned long timeout);

/**
 * Releases the given mutex
 * @param   mutex    mutex structure for deallocation
 * @return  R_SUCCESS on success or
 *          R_ERROR otherwise
**/
unsigned int  mutexUnlock(pefMutex_ptr mutex);

#endif /* THREADUTILS_H_ */









