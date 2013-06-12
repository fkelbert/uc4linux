#include "threads.h"

int initialized = 0;


void thread_run_gloop(GMainLoop *gloop) {
	plog(LOG_TRACE, "Starting gloop thread");
	g_main_loop_run(gloop);
}


void threads_init() {
	GMainLoop *gloop;
	pthread_t gloop_thread;

	if (!initialized) {
		initialized = 1;
		init_glib();
		gloop = g_main_loop_new (NULL, TRUE);
		pthread_create(&gloop_thread, NULL, (void*) thread_run_gloop, gloop);
	}
}


