#ifndef SYSCALLNAMES_H_
#define SYSCALLNAMES_H_

#include <syscall.h>
#include "syscallNumbers.h"

#define SYSCALLNAME(syscallcode) syscallNames[syscallcode]

extern char *syscallNames[];

#endif /* SYSCALLNAMES_H_ */
