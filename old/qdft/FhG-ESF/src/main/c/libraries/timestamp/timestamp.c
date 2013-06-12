
/**
 * @file   timestamp.c  
 * @brief  Functions to manage time structs.
 *
 * @author Ricardo Neisse
 **/

#include "timestamp.h"
#include "log_timestamp_c.h"

/**
 * Converts a timeval struct with seconds and microseconds to microseconds.
 * 
 * @param tv timeval struct
 * @return   total microseconds in a 64 bit int
 */
uint64_t timeval_to_micro_sec( const struct timeval* tv ) {
    uint64_t usec;
    usec = tv->tv_sec;
    usec *= 1000000;
    usec += tv->tv_usec;
    return usec;
}

/**
 * Converts microseconds to a timeval struct with seconds and microseconds.
 * 
 * @param usec microseconds
 * @param tv   timeval struct pointer update with the result
 * @return     The updated timeval struct pointer
 */
struct timeval* micro_sec_to_timeval(uint64_t usec, struct timeval* tv ) {
    tv->tv_sec = usec / 1000000 ;
    tv->tv_usec = usec % 1000000 ;
    return tv ;
}

/**
 * Write the timeval struct to the log in seconds together with a user message
 */ 
void timeval_log(char *msg, struct timeval* tv) {
  log_trace("%s [%ld.%06ld]", msg, tv->tv_sec, tv->tv_usec);
  return;
}

timestamp_ptr timestamp_new() {
  timestamp_ptr timestamp = (timestamp_ptr) mem_alloc(sizeof(timestamp_t));
  // timezone is no longer used in OpenBSD
  (void) gettimeofday(&timestamp->tv, (struct timezone *) 0);
  timestamp->usec = timeval_to_micro_sec(&timestamp->tv);
  timestamp_log("Creating timestamp", timestamp);
  return timestamp;
}

// TODO: make this a macro?
void timestamp_copy(timestamp_ptr source, timestamp_ptr target) {
  target->usec       = source->usec;
  target->tv.tv_sec  = source->tv.tv_sec;
  target->tv.tv_usec = source->tv.tv_usec;
  return;
}

void timestamp_set_now(timestamp_ptr timestamp) {
  (void) gettimeofday(&timestamp->tv, (struct timezone *) 0);
  timestamp->usec = timeval_to_micro_sec(&timestamp->tv);
  return;
}

void timestamp_free(timestamp_ptr timestamp) {
  log_trace("Freeing timestamp");
  mem_free(timestamp);
  return;
}

void timestamp_log(char *msg, timestamp_ptr timestamp) {
  timeval_log(msg, &timestamp->tv);
  return;
}

time_amount_ptr time_amount_new(uint64_t amount, int unit) {
  log_trace("Creating time amount");
  time_amount_ptr time_amount = (time_amount_ptr) mem_alloc(sizeof(time_amount_t));
  time_amount_set(time_amount, amount, unit);
  return time_amount;
}

void time_amount_set(time_amount_ptr time_amount, uint64_t amount, int unit) {
  log_trace("Setting time amount to %lld %s(s)", amount, time_unit_str[unit] );
  time_amount->amount = amount;
  time_amount->unit = unit;
  // convert to microseconds
  if (unit == MICROSECOND) {
    time_amount->usec = amount;
  } else if (unit == MILISECOND) {
    time_amount->usec = amount * 1000;
  } else if (unit == SECOND) {
    time_amount->usec = amount * 1000000;
  } else if (unit == MINUTE) {
    time_amount->usec = amount * 1000000 * 60;
  } else if (unit == HOUR) {
    time_amount->usec = amount * 1000000 * 60 * 60;
  }
}


void time_amount_log(char *msg, time_amount_ptr time_amount) {
  struct timeval tv;
  micro_sec_to_timeval(time_amount->usec, &tv);
  timeval_log(msg, &tv);
  return;
}

// We always use MICROSECOND, which is the highest
// granularity we support.
void time_diff(timestamp_ptr end_timestamp, timestamp_ptr start_timestamp, time_amount_ptr time_amount) { 
  timestamp_log("Start time diff:", start_timestamp);
  timestamp_log("End time diff  :", end_timestamp);
  uint64_t diff = end_timestamp->usec - start_timestamp->usec;
  time_amount_set(time_amount, diff, MICROSECOND);
  return;
}

void time_amount_free(time_amount_ptr time_amount) {
  log_trace("Freeing time amount");
  mem_free(time_amount);
}

time_interval_ptr time_interval_new() {
  log_trace("Creating time interval");
  time_interval_ptr time_interval = (time_interval_ptr) mem_alloc(sizeof(time_interval_t));
  // Create timestamps
  time_interval->start_time = timestamp_new();
  time_interval->end_time = timestamp_new();
  // End time is equal to start time
  timestamp_copy(time_interval->start_time, time_interval->end_time);
  // Create and update time amount
  time_interval->time_amount = time_amount_new(0, SECOND);  
  time_diff(time_interval->end_time, time_interval->start_time, time_interval->time_amount);
  return time_interval;
}

void time_interval_start_now(time_interval_ptr time_interval) {
  // update start time
  timestamp_set_now(time_interval->start_time);
  // End time is exactly equal to start time
  time_interval->end_time->usec= time_interval->start_time->usec;
  time_interval->end_time->tv.tv_sec= time_interval->start_time->tv.tv_sec;
  time_interval->end_time->tv.tv_usec= time_interval->start_time->tv.tv_usec;
  // update time_amount
  time_diff(time_interval->end_time, time_interval->start_time, time_interval->time_amount);
}

void time_interval_end_now(time_interval_ptr time_interval) {
  // update end_time
  timestamp_set_now(time_interval->end_time);
  // update time_amount
  time_diff(time_interval->end_time, time_interval->start_time, time_interval->time_amount);
}


void time_interval_free(time_interval_ptr time_interval) {
  log_trace("Freeing time interval");
  timestamp_free(time_interval->start_time);
  timestamp_free(time_interval->end_time);
  time_amount_free(time_interval->time_amount);
  mem_free(time_interval);
}
