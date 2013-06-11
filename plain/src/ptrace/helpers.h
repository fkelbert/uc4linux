

#ifndef HELPERS_H_
#define HELPERS_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>


#define int_to_str(i,buf,len) snprintf(buf,len,"%d",i)
#define long_to_str(i,buf,len) snprintf(buf,len,"%ld",i)
#define long_to_hex(i,buf,len) snprintf(buf,len,"%lx",i)

struct passwd *getUserInfo(int pid);
char *getCmdline(int pid, char *buf, int len);

#endif /* HELPERS_H_ */
