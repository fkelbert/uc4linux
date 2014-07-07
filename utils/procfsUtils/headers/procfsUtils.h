

#ifndef PROCFSUTILS_H_
#define PROCFSUTILS_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>


struct passwd *getUserInfo(int pid);
char *getCmdline(int pid, char *buf, int len);

#endif /* PROCFSUTILS_H_ */
