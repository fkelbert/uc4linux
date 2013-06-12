/**
 * @file   timestamp.c  
 * @brief  Implementation of time related behaviour
 *
 * @see timestamp.h
 *
 * @author cornelius moucha, Ricardo Neisse
 **/

#include "timestamp.h"
#include "log_timestamp_pef.h"


uint64_t usecNow()
{
  struct timeval t;
  (void) gettimeofday(&t, (struct timezone *) 0);
  // cast mandatory! otherwise result will be something unpredictable!
  // e.g. 18446744067709567241 usecs since 1.1.1970, which is a date far beyond the year 60425
  return ((uint64_t) t.tv_sec * 1000000 + t.tv_usec);
}

timestamp_ptr timestampNewEmpty()
{
  timestamp_ptr timestamp=(timestamp_ptr)memAlloc(sizeof(timestamp_t));
  checkNullPtr(timestamp, "Error allocating memory for timestamp");
  timestamp->usec=0;
  return timestamp;
}

timestamp_ptr timestampNew()
{
  timestamp_ptr timestamp=(timestamp_ptr) memAlloc(sizeof(timestamp_t));
  checkNullPtr(timestamp, "Error allocating memory for timestamp");
  (void) gettimeofday(&timestamp->tv, (struct timezone *) 0);
  timestamp->usec=convertToMicroSeconds(&timestamp->tv);
  log_trace("Creating timestamp [%llu]", timestamp->usec);
  return timestamp;
}

void timestampCopy(const timestamp_ptr source, timestamp_ptr target)
{
  if(source==NULL || target==NULL) {log_warn("%s - Could not copy NULL timestamps!", __func__); return;}
  target->usec=source->usec;
  target->tv.tv_sec=source->tv.tv_sec;
  target->tv.tv_usec=source->tv.tv_usec;
  return;
}

void timestampSetNow(timestamp_ptr timestamp)
{
  if(timestamp==NULL) {log_warn("%s - Could not set timestamp to NULL", __func__); return;}
  (void) gettimeofday(&timestamp->tv, (struct timezone *) 0);
  timestamp->usec=convertToMicroSeconds(&timestamp->tv);
  return;
}

void timestampFree(timestamp_ptr timestamp)
{
  if(timestamp==NULL) return;
  log_trace("Freeing timestamp [%p]", timestamp);
  if(timestamp!=NULL) memFree(timestamp);
  return;
}

void timestampLog(const char *msg, timestamp_ptr timestamp)
{
  if(msg==NULL || timestamp==NULL) {log_warn("%s - Logging NULL pointers impossible", __func__); return;}
  log_trace("%s [%llu]", msg, timestamp->usec);
  return;
}

uint64_t convertToMicroSeconds(const struct timeval* tv)
{
  if(tv==NULL) {log_error("Error: NULL parameter given for timeval"); return 0;}
  return ((uint64_t) tv->tv_sec * 1000000 + tv->tv_usec);
}

struct timeval* convertToTimeval(const uint64_t usec, struct timeval* tv)
{
  checkNullPtr(tv, "Error: NULL parameter given for timeval");
  tv->tv_sec=usec / 1000000;
  tv->tv_usec=usec % 1000000;
  return tv;
}

void timevalLog(const char *msg, struct timeval* tv)
{
  if(msg==NULL || tv==NULL) {log_error("Error: NULL parameters for logging"); return;}
  log_trace("%s [%ld.%06ld]", msg, tv->tv_sec, tv->tv_usec);
  return;
}

timeAmount_ptr timeAmountNew(uint64_t amount, int unit)
{
  log_trace("Creating time amount");
  timeAmount_ptr timeAmount=(timeAmount_ptr)memAlloc(sizeof(timeAmount_t));
  timeAmountSet(timeAmount, amount, unit);
  return timeAmount;
}

void timeAmountSet(timeAmount_ptr timeAmount, uint64_t amount, int unit)
{
  if(timeAmount==NULL) {log_error("Error: NULL parameter for timeAmount"); return;}
  log_trace("Setting time amount to %lld %s(s)", amount, timeUnitStr[unit]);
  timeAmount->amount=amount;
  timeAmount->unit=unit;
  // convert to microseconds
  if(unit==MICROSECOND)      timeAmount->usec=amount;
  else if(unit==MILLISECOND) timeAmount->usec=amount*1000;
  else if(unit==SECOND)      timeAmount->usec=amount*1000000;
  else if(unit==MINUTE)      timeAmount->usec=amount*1000000*60;
  else if(unit==HOUR)        timeAmount->usec=amount*1000000*3600;
}

void timeAmountLog(const char *msg, timeAmount_ptr timeAmount)
{
  if(msg==NULL || timeAmount==NULL) {log_error("Error: NULL parameters for logging"); return;}
  struct timeval tv;
  convertToTimeval(timeAmount->usec, &tv);
  timevalLog(msg, &tv);
  return;
}

void timeDifference(timestamp_ptr timestampEnd, timestamp_ptr timestampStart, timeAmount_ptr timeAmount)
{
  if(timestampEnd==NULL || timestampStart==NULL || timeAmount==NULL) {log_error("Error: NULL parameters"); return;}
  timestampLog("Start time diff:", timestampStart);
  timestampLog("End time diff  :", timestampEnd);
  uint64_t diff=timestampEnd->usec - timestampStart->usec;
  timeAmountSet(timeAmount, diff, MICROSECOND);
  return;
}

void timeAmountFree(timeAmount_ptr timeAmount)
{
  log_trace("Freeing time amount");
  memFree(timeAmount);
}

timeInterval_ptr timeIntervalNew()
{
  log_trace("Creating time interval");
  timeInterval_ptr time_interval=(timeInterval_ptr) memAlloc(sizeof(timeInterval_t));
  time_interval->startTime=timestampNew();
  time_interval->endTime=timestampNew();
  // End time is equal to start time
  timestampCopy(time_interval->startTime, time_interval->endTime);
  // Create and update time amount
  time_interval->timeAmount=timeAmountNew(0, SECOND);
  timeDifference(time_interval->endTime, time_interval->startTime, time_interval->timeAmount);
  return time_interval;
}

void timeIntervalStartNow(timeInterval_ptr timeInterval)
{
  if(timeInterval==NULL) return;
  // update start time
  timestampSetNow(timeInterval->startTime);
  // End time is exactly equal to start time
  timeInterval->endTime->usec=timeInterval->startTime->usec;
  timeInterval->endTime->tv.tv_sec=timeInterval->startTime->tv.tv_sec;
  timeInterval->endTime->tv.tv_usec=timeInterval->startTime->tv.tv_usec;
  // update timeAmount
  timeDifference(timeInterval->endTime, timeInterval->startTime, timeInterval->timeAmount);
}

void timeIntervalEndNow(timeInterval_ptr timeInterval)
{
  if(timeInterval==NULL) return;
  // update endTime
  timestampSetNow(timeInterval->endTime);
  // update timeAmount
  timeDifference(timeInterval->endTime, timeInterval->startTime, timeInterval->timeAmount);
}

void timeIntervalFree(timeInterval_ptr timeInterval)
{
  if(timeInterval==NULL) return;
  log_trace("Freeing time interval");
  timestampFree(timeInterval->startTime);
  timestampFree(timeInterval->endTime);
  timeAmountFree(timeInterval->timeAmount);
  memFree(timeInterval);
}

uint64_t getTimeUnitMultiplier(const char *timeUnitType)
{
  if(timeUnitType==NULL) return 0;

  if(!(strncasecmp(timeUnitType,"MICROSECONDS",12)))     return 1;
  else if(!(strncasecmp(timeUnitType,"MILLISECONDS",12)))return 1000;
  else if(!(strncasecmp(timeUnitType,"SECONDS",7)))      return 1000000;
  else if(!(strncasecmp(timeUnitType,"MINUTES",7)))      return 60000000;
  else if(!(strncasecmp(timeUnitType,"HOURS",5)))        return 3600000000UL;
  else if(!(strncasecmp(timeUnitType,"DAYS",4)))         return 86400000000ULL;
  else if(!(strncasecmp(timeUnitType,"WEEKS",5)))        return 604800000000ULL;
  else if(!(strncasecmp(timeUnitType,"MONTHS",6)))       return 2592000000000ULL;
  else if(!(strncasecmp(timeUnitType,"YEARS",5)))        return 31104000000000ULL;
  else if(!(strncasecmp(timeUnitType,"timesteps",9))) {log_error("unexpected timeunit found!"); return 1;}
  else return 1;
}

