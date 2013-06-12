/*
 * time_utils.h
 *
 *      Author: Ricardo Neisse
 */

#ifndef _time_utils_h
#define _time_utils_h

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>


int64_t usec_now(void);
int64_t timeval_to_usec( const struct timeval* tv );
struct timeval* usec_to_timeval( int64_t usec, struct timeval* tv );
void timeval_print(struct timeval *tv);
void plog_timeval(int log_level, char *msg, struct timeval *tv);
void plog_time(int log_level, char *msg, int64_t time);
uint64_t usec_getTimeUnitMultiplier(const char *);

#endif

