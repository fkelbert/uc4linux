/**
 * @file  timestamp.h
 * @brief Times related data types and functions
 *
 * @author Ricardo Neisse
 **/
#ifndef _timestamp_h
#define _timestamp_h

#include <sys/time.h>
#include <time.h>

#include "base.h"
#include "mem_utils.h"

/*
   Discussing time in a technical manual can be difficult because the word �time�
   in English refers to lots of different things.  

   "Calendar time" or "absolut time" is a point in the time continuum, for example,
   November 4, 1990 at 18:02.5 UTC. �Date� is inherent in a calendar time.
   - timestamp
   
   "Time interval", contiguous part of time between two specific "calendar times",
   for example the hour between 9:00 and 10:00 on July 4, 1980.
   - time_interval

   "Elapsed time" is the length of an interval, 4 days. "Amount of time" is a sum of
   elapsed times, which need not be of any specific intervals. "Time period" is the
   elapsed time of an interval between two events, especially when they are part of
   a sequence of regularly repeating events.
   - time_amount

   See: http://www.imodulo.com/GNU/glibc/Time-Basics.html
*/

// NANOSECOND is not supported
#define TIMESTEP     0 // discrete time reference
#define NANOSECOND   1 // 10 e -9 seconds
#define MICROSECOND  2 // 10 e -6 seconds
#define MILISECOND   3 // 10 e -3 seconds
#define SECOND       4 
#define MINUTE       5 // 60 seconds
#define HOUR         6 // 60 minutes
#define DAY          7 // 24 hours
#define WEEK         8 // 7 days
#define MONTH        9 // 30 days
#define YEAR        10 // 12 months

// The following array is indexed according to
// the defines above.
static char* const time_unit_str[] = {
  "timestep",
  "nanosecond",
  "microsecond",
  "milisecond",
  "second",
  "minute",
  "hour",
  "day",
  "week",
  "month", // 30 days
  "year"   // 12 months, or 360 days
};

typedef struct timestamp_s {
  uint64_t usec; // microseconds since January 1, 1970
  struct timeval tv;
} timestamp_t;
typedef timestamp_t *timestamp_ptr;

typedef struct time_amount_s {
  uint64_t amount;
  uint8_t unit;
  uint64_t usec; // usec for comparisons
} time_amount_t;
typedef time_amount_t *time_amount_ptr;

typedef struct time_interval_s {
  timestamp_ptr start_time;
  timestamp_ptr end_time;
  time_amount_ptr time_amount;
} time_interval_t;
typedef time_interval_t *time_interval_ptr;

uint64_t timeval_to_micro_sec( const struct timeval* tv );
struct timeval* micro_sec_to_timeval(uint64_t usec, struct timeval* tv );
void timeval_log(char *msg, struct timeval* tv);

timestamp_ptr timestamp_new();
void timestamp_set_now(timestamp_ptr timestamp);
void timestamp_copy(timestamp_ptr source, timestamp_ptr target);
void timestamp_free(timestamp_ptr timestamp);
void timestamp_log(char *msg, timestamp_ptr timestamp);

time_amount_ptr time_amount_new(uint64_t amount, int unit);
void time_amount_set(time_amount_ptr time_amount, uint64_t amount, int unit);
void time_amount_log(char *msg, time_amount_ptr time_amount);
void time_diff(timestamp_ptr end_timestamp, timestamp_ptr start_timestamp, time_amount_ptr time_amount); 
void time_amount_free(time_amount_ptr time_amount);

time_interval_ptr time_interval_new();
void time_interval_start_now(time_interval_ptr time_interval);
void time_interval_end_now(time_interval_ptr time_interval);
void time_interval_free(time_interval_ptr time_interval);


#endif
