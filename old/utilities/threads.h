#ifndef THREADS_H
#define THREADS_H

#include <glib.h>
#include <pthread.h>

#include "logger.h"
#include "soup_utils.h"

void threads_init();
void threads_run_gloop(void *arg);

#endif
