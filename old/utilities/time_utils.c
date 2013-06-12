/*
 * time_utils.c
 *
 *      Author: Ricardo Neisse
 */

#include "time_utils.h"

int64_t usec_now(void) {
  struct timeval t;
  int64_t  m;
  (void) gettimeofday(&t, (struct timezone *) 0);
  m = t.tv_sec;
  m *= 1000000;
  m += t.tv_usec;
  return (m);
}

int64_t timeval_to_usec( const struct timeval* tv ) {
    return( (int64_t)tv->tv_sec * 1000000 + tv->tv_usec ) ;
}

struct timeval* usec_to_timeval(int64_t usec, struct timeval* tv ) {
    tv->tv_sec = usec / 1000000 ;
    tv->tv_usec = usec % 1000000 ;
    return tv ;
}

void timeval_print(struct timeval *tv) {
    char buffer[30];
    time_t curtime;
    printf("%ld.%06ld", tv->tv_sec, tv->tv_usec);
    curtime = tv->tv_sec;
    strftime(buffer, 30, "%m-%d-%Y  %T", localtime(&curtime));
    printf(" = %s.%06ld\n", buffer, tv->tv_usec);
}

void plog_timeval(int log_level, char *msg, struct timeval *tv) {
  plog(log_level, "%s [%ld.%06ld]", msg, tv->tv_sec, tv->tv_usec );
}

void plog_time(int log_level, char *msg, int64_t time) {
  struct timeval tv;
  usec_to_timeval(time, &tv);
  plog(log_level, "%s [%ld.%06ld]", msg, tv.tv_sec, tv.tv_usec );
}



