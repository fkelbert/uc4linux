/**
 * @file  timestamp.h
 * @brief Times related data types and methods
 *
 * Time in technical understanding is a complex issue as the word may refer to different semantics with
 * respect to time:\n
 * 'Calendar time' or 'absolute time' is a point in the time continuum, e.g. November 4, 1990 at 18:02.5 UTC.\n
 * 'Date' is inherent in a calendar time.\n
 * 'Time interval', continuous part of time between two specific 'calendar times',
 *                  the hour between 9:00 and 10:00 on July 4, 1980.
 * 'Elapsed time' is the length of an interval, 4 days.\n
 * 'Time amount' is a sum of elapsed times, which need not be of any specific intervals.\n
 * 'Time period' is the elapsed time of an interval between two events, especially when they are part of
 *               a sequence of regularly repeating events.\n
 * See: http://www.imodulo.com/GNU/glibc/Time-Basics.html
 *
 * @author cornelius moucha, Ricardo Neisse
 **/
#ifndef _timestamp_h
#define _timestamp_h

#include <sys/time.h>
#include <time.h>
#include "base.h"
#include "memUtils.h"

/// discrete time reference
#define TIMESTEP     0
///  10 e-9 seconds // ==> not supported
#define NANOSECOND   1
/// 10 e-6 seconds
#define MICROSECOND  2
/// 10 e-3 seconds
#define MILLISECOND  3
/// usual second
#define SECOND       4
/// 60 seconds
#define MINUTE       5
/// 60 minutes
#define HOUR         6
/// 24 hours
#define DAY          7
/// 7 days
#define WEEK         8
/// 30 days
#define MONTH        9
/// 12 months
#define YEAR        10

/// static textual representation of time units; used for logging
static char* const timeUnitStr[]={"timestep", "nanosecond", "microsecond", "millisecond", "second",
                                  "minute", "hour", "day", "week", "month", "year"};

// Forward type definitions
typedef struct timestamp_s     timestamp_t;
typedef        timestamp_t    *timestamp_ptr;

typedef struct timeAmount_s   timeAmount_t;
typedef        timeAmount_t   *timeAmount_ptr;

typedef struct timeInterval_s  timeInterval_t;
typedef        timeInterval_t *timeInterval_ptr;

/**
 * @struct timestamp_s
 * @brief Microseconds since the start of the unix epoch
 *
 * The timestamp structure holds the amount of microseconds since the start of the unix epoch,
 * i.e. 1.1.1970.
**/
struct timestamp_s
{
  /// microseconds since January 1, 1970
  uint64_t       usec;
  /// required as time() only gives seconds since 1.1.1970
  struct timeval tv;
};

/**
 * @struct timeAmount_s
 * @brief Information about a sum of elapsed times
 *
 * The timeAmount structure holds information about an 'amount of time',
 * i.e. a sum of elapsed times, which need not be of any specific intervals.
**/
struct timeAmount_s
{
  /// amount of units for this structure
  uint64_t amount;
  /// time unit
  uint8_t  unit;
  /// microseconds used for comparisons
  uint64_t usec;
};

/**
 * @struct timeInterval_s
 * @brief Specify a continuous part of time between two specific 'calendar times'
 *
 * The timeInterval specifies an interval of time,
 * i.e. a continuous part of time between two specific 'calendar times',
 * the hour between 9:00 and 10:00 on July 4, 1980.
**/
struct timeInterval_s
{
  timestamp_ptr  startTime;
  timestamp_ptr  endTime;
  timeAmount_ptr timeAmount;
};

uint64_t      usecNow();

/**
 * Calculates the amount of microseconds for the given time unit;
 * e.g. one minute are 60 seconds, which are 60.000 milliseconds,
 * therefore one minute are 60.000.000 microseconds
 *
 * @param   timeUnitType textual representation of a timeunit
 * @return  amount of microseconds for this time unit
 */
uint64_t      getTimeUnitMultiplier(const char *timeUnitType);

timestamp_ptr timestampNew();
timestamp_ptr timestampNewEmpty();
void          timestampSetNow(timestamp_ptr timestamp);
void          timestampCopy(const timestamp_ptr source, timestamp_ptr target);
void          timestampFree(timestamp_ptr timestamp);
void          timestampLog(const char *msg, timestamp_ptr timestamp);

uint64_t        convertToMicroSeconds(const struct timeval* tv);
struct timeval* convertToTimeval(const uint64_t usec, struct timeval* tv);
void            timevalLog(const char *msg, struct timeval* tv);

/// @todo timeAmount and timeInterval not fully tested; and never used??
timeAmount_ptr timeAmountNew(uint64_t amount, int unit);
void           timeAmountSet(timeAmount_ptr timeAmount, uint64_t amount, int unit);
void           timeAmountLog(const char *msg, timeAmount_ptr timeAmount);
void           timeDifference(timestamp_ptr timestampEnd, timestamp_ptr timestampStart, timeAmount_ptr timeAmount);
void           timeAmountFree(timeAmount_ptr timeAmount);

timeInterval_ptr timeIntervalNew();
void             timeIntervalStartNow(timeInterval_ptr timeInterval);
void             timeIntervalEndNow(timeInterval_ptr timeInterval);
void             timeIntervalFree(timeInterval_ptr timeInterval);


#endif
