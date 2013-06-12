/*
 * time_utils.c
 *
 *      Author: Ricardo Neisse
 */

#include "time_utils.h"
#include "log_time_utils_c.h"

int64_t usec_now(void)
{
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
  //plog(log_level, "%s [%ld.%06ld]", msg, tv->tv_sec, tv->tv_usec );
  log_info("%s [%ld.%06ld]", msg, tv->tv_sec, tv->tv_usec );
}

void plog_time(int log_level, char *msg, int64_t time) {
  struct timeval tv;
  usec_to_timeval(time, &tv);
  //plog(log_level, "%s [%ld.%06ld]", msg, tv.tv_sec, tv.tv_usec );
  log_info("%s [%ld.%06ld]", msg, tv.tv_sec, tv.tv_usec );
}

// shift to time_utils.c!
uint64_t usec_getTimeUnitMultiplier(const char *timeUnitType)
{
  if(!(strncasecmp(timeUnitType,"MICROSECONDS",12)))     return 1;
  else if(!(strncasecmp(timeUnitType,"MILISECONDS",11))) return 1000;
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
