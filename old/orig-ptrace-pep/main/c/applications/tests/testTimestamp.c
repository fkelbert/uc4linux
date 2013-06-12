/**
 * @file  testTimestamp.c
 * @brief Exemplary tests for time implementation: timestamp.c, timestamp.h
 *
 * @author cornelius moucha, Ricardo Neisse
 **/
#include <unistd.h>
#include "timestamp.h"
#include "log_testTimestamp_pef.h"

#ifdef __WIN32__
  #define sleep Sleep
#endif

int main(int argc, char **argv)
{
  uint64_t now=usecNow();
  log_info("now=[%llu]", now);
  uint64_t mul=getTimeUnitMultiplier(NULL);
  log_info("[%llu]", mul);
  mul=getTimeUnitMultiplier("bliblablub");
  log_info("[%llu]", mul);
  mul=getTimeUnitMultiplier("WEEKS");
  log_info("[%llu]", mul);

  timestamp_ptr stamp=timestampNew();
  timestamp_ptr stamp2=timestampNewEmpty();
  timestampLog("stamp: ", stamp);
  timestampLog("stamp2: ", stamp2);

  timestampLog(NULL, NULL);
  timestampLog(NULL, stamp);
  timestampLog("msg: ", NULL);

  timestampFree(NULL);
  timestampCopy(NULL, NULL);
  timestampCopy(NULL, stamp2);
  timestampCopy(stamp, NULL);
  timestampCopy(stamp, stamp2);
  timestampLog("stamp2: ", stamp2);

  timestampSetNow(NULL);
  timestampSetNow(stamp2);
  timestampLog("stamp2: ", stamp2);

  timeAmount_ptr timestep_size=timeAmountNew(3, SECOND);
  timeAmountLog("Timestep size: ", timestep_size);

  timeInterval_ptr time_interval=timeIntervalNew();
  timeAmountLog("Time interval length: ", time_interval->timeAmount);

  sleep(2);

  timeIntervalEndNow(time_interval);
  timeAmountLog("Time interval length: ", time_interval->timeAmount);

  if(time_interval->timeAmount->usec > timestep_size->usec) log_info("Interval is larger");
  else log_info("Interval is smaller");

  timeIntervalFree(time_interval);
  timeAmountFree(timestep_size);
  return 0;
}
