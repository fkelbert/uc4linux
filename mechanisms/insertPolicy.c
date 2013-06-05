#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <stdarg.h>
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>
#include <string.h>

#include "soup_utils.h"


int iinitialized = 0;


void ithread_run_gloop(GMainLoop *gloop) {
	g_main_loop_run(gloop);
}


void ithreads_init() {
	GMainLoop *gloop;
	pthread_t gloop_thread;

	if (!iinitialized) {
		iinitialized = 1;
		init_glib();
		gloop = g_main_loop_new (NULL, TRUE);
		pthread_create(&gloop_thread, NULL, (void*) ithread_run_gloop, gloop);
	}
}



void service_caller_insertPolicy(char *mechanism) {
	ithreads_init();


	SoupSession *session;
	GValue g_return_value;

	char *service_uri;
	int result = 0;


	session = soup_session_sync_new();

	service_uri = "http://localhost:8082/data_flow_monitor_service";

	if (!do_xmlrpc (session, service_uri, "insertMechanism", &g_return_value,
					G_TYPE_STRING, mechanism,
					G_TYPE_INVALID)) {
		plog(LOG_INFO, "Not available: %s",service_uri);

		exit(1);
	}


	plog(LOG_TRACE, "Checking response");
	if (!check_xmlrpc (&g_return_value, G_TYPE_INT, &result)) {
		plog(LOG_ERROR, "Protocol error in response.");
		exit(1);
	}

	soup_session_abort(session);
}





int main(int argc, char** argv) {
	if (argc < 2) {
		return 1;
	}

	FILE *file = fopen(argv[1], "r");

	if (file == NULL) {
		return 1;
	}

	char buf[8192];
	memset(buf,0,8192);
	fread(buf, sizeof(char), 8191, file);

	service_caller_insertPolicy(buf);
}


