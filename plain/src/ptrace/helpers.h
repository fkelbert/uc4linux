

#ifndef HELPERS_H_
#define HELPERS_H_

#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <pwd.h>


struct passwd *getUserInfo(int pid);

#endif /* HELPERS_H_ */
