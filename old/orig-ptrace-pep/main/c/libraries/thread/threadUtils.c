/**
 * @file  threadUtils.c
 * @brief Implementation of auxiliary utilities for thread and mutex handling
 * @see   threadUtils.h
 *
 * @author cornelius moucha
 **/

#include "threadUtils.h"
#include "log_threadUtils_pef.h"

#ifdef __WIN32__
// using Windows API for thread management...

// LPTHREAD_START_ROUTINE: typedef DWORD (WINAPI *LPTHREAD_START_ROUTINE)(LPVOID);
// HANDLE WINAPI CreateThread(__in_opt LPSECURITY_ATTRIBUTES lpThreadAttributes,__in SIZE_T dwStackSize, __in LPTHREAD_START_ROUTINE lpStartAddress,
//                            __in_opt LPVOID lpParameter, __in DWORD dwCreationFlags, __out_opt  LPDWORD lpThreadId);
//Do not declare this callback function with a void return type and cast the function pointer to
//LPTHREAD_START_ROUTINE when creating the thread. Code that does this is common, but it can crash on 64-bit Windows.
pefThread_ptr pefThreadNew(LPTHREAD_START_ROUTINE handler, void* userData)
{
  checkNullPtr(handler, "Require start method for new thread!");
  pefThread_ptr thread=(pefThread_ptr)memAlloc(sizeof(pefThread_t));
  checkNullPtr(thread, "Could not allocate memory for pefThread structure!");

  thread->lthread=0;
  thread->sleepValue=0;
  thread->adjustSleepValue=adjustSleepValue;
  thread->sleep=threadSleepMicro;
  thread->prepareThreadKill=prepareThreadKill;
  thread->start=startThread;
  thread->handler=handler;
  thread->userData=userData;

  //thread->lthread=CreateThread(NULL, 0, thread->handler, userData, 0, &thread->threadID);
  //checkNullPtr(thread->lthread, "Failure creating new thread!");
  log_debug("Created new thread with handler=[%p] => threadID=[%lu]", thread->handler, thread->threadID);

  return thread;
}

unsigned int startThread(pefThread_ptr thread)
{
  checkNullInt(thread, "Cannot start NULL-thread");

  thread->lthread=CreateThread(NULL, 0, thread->handler, thread->userData, 0, &thread->threadID);
  checkNullInt(thread->lthread, "Failure creating new thread!");

  // Lock suspend mutex; required for condition evaluation
  //ret=pthread_mutex_lock(&thread->suspendMutex);
  //if(ret!=0) {log_error("Error locking suspend mutex for thread"); return R_ERROR;}
  //thread->suspend(thread);
  log_debug("Thread successfully started");
  return R_SUCCESS;
}

unsigned int pefThreadFree(pefThread_ptr thread)
{
  checkNullInt(thread, "Cannot deallocate NULL-thread!");
  free(thread);
  return R_SUCCESS;
}

unsigned int prepareThreadKill(void* exitHandler)
{ // not required for win32; TerminateThread will kill thread anyway
  return R_SUCCESS;
}

//Like the Win32 ExitThread API, _endthreadex does not close the thread handle.
//Therefore, when you use _beginthreadex and _endthreadex, you must close the thread handle by calling the Win32 CloseHandle API.
//VOID WINAPI ExitThread(_in  DWORD dwExitCode);

// exiting example:http://msdn.microsoft.com/en-us/library/windows/desktop/ms686915%28v=vs.85%29.aspx
// we can use waitForSingleOBject with timeout to process sleep and check return value whether thread should be
// exited or mechanism updated
// eventmutex refer to CreateEvent() from WinAPI
// other alternative: waitforsingleobject(eventMutex, timeout=sleepValue)
// by now use TerminateThread()

/// @todo check for memory leaks doing thread termination this way!
unsigned int pefThreadKill(pefThread_ptr thread)
{
  int ret=TerminateThread(thread->lthread, 0);
  if(ret==0) log_warn("Killing thread returned error code=[%d]", GetLastError());
  return !ret;
}

//note: in windows usleep works only up to 999.999 microseconds sleeping! otherwise no sleeping at all!!
uint64_t adjustSleepValue(pefThread_ptr thread, uint64_t sleepValue)
{
  if(sleepValue>999999)
  {
    thread->sleepValue=div(sleepValue, 1000).quot;
    log_debug("Adjusted thread usleep=%llu ms", thread->sleepValue);
    thread->sleep=threadSleepMilli;
  }
  else thread->sleepValue=sleepValue;

  return thread->sleepValue;
}

void threadSleepMilli(unsigned long sleepValue)
{
  Sleep(sleepValue);
}

void threadSleepMicro(unsigned long sleepValue)
{
  usleep(sleepValue);
}

//HANDLE WINAPI CreateMutex(__in_opt  LPSECURITY_ATTRIBUTES lpMutexAttributes,__in      BOOL bInitialOwner (FALSE => not initially owned),
//                          __in_opt  LPCTSTR lpName);
pefMutex_ptr pefMutexNew(const char *name)
{
  pefMutex_ptr mutex=(pefMutex_ptr)memAlloc(sizeof(pefMutex_t));
  checkNullPtr(mutex, "Could not allocate memory for pefMutex structure!");

  mutex->name=NULL;
  if(name!=NULL) mutex->name=strdup(name);
  mutex->lmutex=CreateMutex(NULL, FALSE, NULL);
  checkNullPtr(mutex->lmutex, "Could not create new Mutex!");
  mutex->lock=mutexLock;
  mutex->unlock=mutexUnlock;
  log_debug("Successfully created mutex [%s]", (mutex->name!=NULL?mutex->name:"unknown"));
  return mutex;
}

unsigned int pefMutexFree(pefMutex_ptr mutex)
{
  checkNullInt(mutex, "Cannot deallocate NULL-mutex!");
  log_trace("Deallocating mechanism mutex [%s]", (mutex->name!=NULL?mutex->name:"unknown"));
  int ret=0;
  ret=CloseHandle(mutex->lmutex);
  if(ret==0) log_error("Error deallocating mutex");
  free(mutex->name);
  free(mutex);

  log_debug("Mechanism mutex successfully deallocated.");
  return R_SUCCESS;
}

// Wait for all threads to terminate
// WaitForMultipleObjects(THREADCOUNT, aThread, TRUE, INFINITE);
// DWORD WINAPI WaitForSingleObject(__in  HANDLE hHandle, __in  DWORD dwMilliseconds);
unsigned int mutexLock(pefMutex_ptr mutex, unsigned long timeout)
{
  //log_trace("Locking mutex [%s]", (mutex->name!=NULL?mutex->name:"unknown"));
  return WaitForSingleObject(mutex->lmutex, timeout);
}
unsigned int mutexUnlock(pefMutex_ptr mutex)
{
  //log_trace("Releasing mutex [%s]", (mutex->name!=NULL?mutex->name:"unknown"));
  return ReleaseMutex(mutex->lmutex);
}

#else

// using pthread for thread and mutex management...
pefThread_ptr pefThreadNew(LPTHREAD_START_ROUTINE handler, void* userData)
{
  checkNullPtr(handler, "Require start method for new thread!");
  pefThread_ptr thread=(pefThread_ptr)memAlloc(sizeof(pefThread_t));
  checkNullPtr(thread, "Could not allocate memory for pefThread structure!");

  thread->lthread=0;
  thread->sleepValue=0;
  thread->adjustSleepValue=adjustSleepValue;
  thread->sleep=threadSleepMicro;
  thread->prepareThreadKill=prepareThreadKill;
  thread->start=startThread;
  thread->handler=handler;
  thread->userData=userData;

  //thread->suspend=suspendThread;
  //thread->resume=resumeThread;
  //thread->suspendVal=0;
  //int ret=pthread_mutex_init(&thread->suspendMutex,NULL);
  //if(ret!=0) {log_error("Error initializing suspendMutex for thread."); return NULL;}
  //ret=pthread_cond_init(&thread->suspendCondition,NULL);
  //if(ret!=0) {log_error("Error initializing suspend condition"); return NULL;}

  log_debug("Created new thread with handler=[%p]", thread->handler);
  return thread;
}

unsigned int startThread(pefThread_ptr thread)
{
  checkNullInt(thread, "Cannot start NULL-thread");

  int ret=pthread_create(&thread->lthread, NULL, thread->handler, thread->userData);
  if(ret!=0) {log_error("Error creating thread (return code=[%d])!", ret); return R_ERROR;}

  // Lock suspend mutex; required for condition evaluation
  //ret=pthread_mutex_lock(&thread->suspendMutex);
  //if(ret!=0) {log_error("Error locking suspend mutex for thread"); return R_ERROR;}
  //thread->suspend(thread);
  log_debug("Thread successfully started");
  return R_SUCCESS;
}

unsigned int isSuspended(pefThread_ptr thread)
{
  /*checkNullInt(thread, "Cannot check NULL-thread");
  int ret;
  ret=pthread_mutex_trylock(&thread->suspendMutex);
  if(ret==0)
  {
    log_trace("Thread is suspended");
    ret=pthread_mutex_unlock(&thread->suspendMutex);
    return 1;
  }
  log_trace("Thread is not suspended");*/
  return 0;
}

unsigned int suspendThread(pefThread_ptr thread)
{
  /*checkNullInt(thread, "Cannot suspend NULL-thread");
  log_trace("Suspending thread...");
  if(isSuspended(thread)) return R_ERROR;
  thread->suspendVal=1;*/
  return R_SUCCESS;
}

unsigned int resumeThread(pefThread_ptr thread)
{
  /*checkNullInt(thread, "Cannot resume NULL-thread");
  int ret;
  if(isSuspended(thread)==0) return R_ERROR;
  log_trace("Thread is suspended; trying to resume");
  ret=pthread_cond_signal(&thread->suspendCondition);
  if(ret!=0) {log_error("Error resuming thread sending"); return R_ERROR;}*/
  return R_SUCCESS;
}

/*LPTHREAD_START_ROUTINE threadSuspend(void *data)
{
  pefThread_ptr thread=(pefThread_ptr)data;
  if(thread==NULL) {log_error("Error: parameter is NULL in threadSuspend!"); return NULL;}
  while(1)
  {
    if(thread->sleepValue==1)
    {
      log_trace("sleepVal is 1");
      thread->sleepValue=0;
      log_trace("cond_wait");
      pthread_cond_wait(&thread->suspendCondition,&thread->suspendMutex);
    }

    thread->handler=thread->workHandler;
  }
}*/

unsigned int pefThreadFree(pefThread_ptr thread)
{
  checkNullInt(thread, "Cannot deallocate NULL-thread!");
  free(thread);
  return R_SUCCESS;
}

unsigned int prepareThreadKill(void* exitHandler)
{
  //#ifdef PTHREAD_CANCEL
  //  pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
  //  pthread_setcanceltype(PTHREAD_CANCEL_DEFERRED, NULL);
  //  pthread_cleanup_push(exitHandler, (void*)NULL);
  //#else
    struct sigaction mechanismUpdaterExitSigAction;
    memset(&mechanismUpdaterExitSigAction, 0, sizeof(struct sigaction));
    memset(&mechanismUpdaterExitSigAction.sa_mask, 0, sizeof *(&mechanismUpdaterExitSigAction.sa_mask));
    mechanismUpdaterExitSigAction.sa_flags=0;
    mechanismUpdaterExitSigAction.sa_handler=exitHandler;
    int ret=sigaction(SIGUSR1, &mechanismUpdaterExitSigAction, NULL);
    if(ret!=0) log_warn("Error registering exit signal handler");
    else log_debug("Mechanism update thread signal handler registered");
  //#endif
  return R_SUCCESS;
}

unsigned int pefThreadKill(pefThread_ptr thread)
{
  checkNullInt(thread, "Cannot terminate NULL-thread");
  if(thread->lthread==0) {log_error("Cannot terminate NULL-thread"); return R_ERROR;}

  int ret=0;
  //#ifdef PTHREAD_CANCEL
  // int ret=pthread_cancel(mech->updateThread);
  //  if(ret!=0) log_warn("Error with pthread_cancel"); //perror("Cancelation error");
  //  ret=pthread_join(mech->updateThread, NULL);
  //  if(ret!=0) log_warn("Error with pthread_join");
  //  else log_info("Mechanism update thread successfully terminated (w/ PTHREAD_CANCEL)");
  //#else
    ret=pthread_kill(thread->lthread, SIGUSR1);
    if(ret!=0) log_warn("Error with pthread_kill"); //perror("Cancelation error");
    else log_debug("Exiting signal sent to updateThread (SIGUSR1)...");
    ret=pthread_join(thread->lthread, NULL);  // joining required to avoid memory leak with libpthread
    if(ret!=0) log_warn("Error with pthread_join");
    else log_info("Mechanism update thread successfully terminated (w/o PTHREAD_CANCEL)");
  //#endif

  return 0;
}

uint64_t adjustSleepValue(pefThread_ptr thread, uint64_t sleepValue)
{
  thread->sleepValue=sleepValue;
  return thread->sleepValue;
}

void threadSleepMilli(unsigned long sleepValue) {}
void threadSleepMicro(unsigned long sleepValue)
{
  usleep(sleepValue);
}

pefMutex_ptr pefMutexNew(const char *name)
{
  pefMutex_ptr mutex=(pefMutex_ptr)memAlloc(sizeof(pefMutex_t));
  checkNullPtr(mutex, "Could not allocate memory for pefMutex structure!");

  mutex->name=NULL;
  if(name!=NULL) mutex->name=strdup(name);
  mutex->lmutex=(pthread_mutex_t*)memAlloc(sizeof(pthread_mutex_t));
  if(pthread_mutex_init(mutex->lmutex, NULL)!=0) {log_warn("Error initializing mechanism mutex!"); return NULL;}

  mutex->lock=mutexLock;
  mutex->unlock=mutexUnlock;
  log_debug("Successfully created mutex [%s]", (mutex->name!=NULL?mutex->name:"unknown"));
  return mutex;
}

unsigned int pefMutexFree(pefMutex_ptr mutex)
{
  checkNullInt(mutex, "Cannot deallocate NULL-mutex");
  log_trace("Deallocating mechanism mutex [%s]", (mutex->name!=NULL?mutex->name:"unknown"));
  int ret=0;
  if((ret=pthread_mutex_destroy(mutex->lmutex))!=0) log_warn("Error deallocating mechanism mutex [%d]!", ret);
  free(mutex->lmutex);
  free(mutex->name);
  free(mutex);

  log_debug("Mechanism mutex successfully deallocated.");
  return R_SUCCESS;
}

unsigned int mutexLock(pefMutex_ptr mutex, unsigned long timeout)
{
  //log_trace("Locking mutex [%s]", (mutex->name!=NULL?mutex->name:"unknown"));
  return pthread_mutex_lock(mutex->lmutex);
}
unsigned int mutexUnlock(pefMutex_ptr mutex)
{
  //log_trace("Releasing mutex [%s]", (mutex->name!=NULL?mutex->name:"unknown"));
  return pthread_mutex_unlock(mutex->lmutex);
}

#endif

