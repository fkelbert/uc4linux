/**
 * @file
 * @brief
 *
 * @author Ricardo Neisse
 **/
#include <unistd.h>
#include "timestamp.h"
#include "log_timestamp_test_c.h"

int main(int argc, char **argv) {

/*
  timestamp_ptr t = timestamp_new();
  timestamp_log("",t);
*/
  log_trace("");

  time_amount_ptr timestep_size = time_amount_new(3, SECOND);
  time_amount_log("Timestep size: ", timestep_size);

  time_interval_ptr time_interval = time_interval_new();
  time_amount_log("Time interval length: ", time_interval->time_amount);
  
  sleep(2);

  time_interval_end_now(time_interval);
  time_amount_log("Time interval length: ", time_interval->time_amount);

  if (time_interval->time_amount->usec > timestep_size->usec) {
    log_info("Interval is bigger");
  } else {
    log_info("Interval is smaller");
  }

  time_interval_free(time_interval);
  time_amount_free(timestep_size);
  return 0;
}
