/*
 * filesystem_utils.h
 *
 *  Created on: 2011/08/08
 *      Author: Florian Kelbert
 */

#ifndef _filesystem_utils_h
#define _filesystem_utils_h

#include <string.h>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <stdio.h>


#include <sys/wait.h>
#include <sys/types.h>
#include <sys/syslimits.h>

#define BUFSIZE 512

char *get_absolute_filename(char *abs_filename, int size, char *rel_filename, int pid, char *command);
int get_cwd(int pid, char *cwd, int size);

#endif
