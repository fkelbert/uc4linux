#include "data_flow_monitor_service.h"


dataCont_ptr dataCont;
SoupSession *session = NULL;


// XML-RPC functions
void data_flow_monitor_handler (__attribute__((unused)) SoupServer *server,
								SoupMessage *msg,
								__attribute__((unused)) const char *path,
								__attribute__((unused)) GHashTable *query,
								__attribute__((unused)) SoupClientContext *context,
								__attribute__((unused)) gpointer data) {
	char *method_name;
	GValueArray *params;


	if (!msg_post(msg)) return;

	soup_message_set_status (msg, SOUP_STATUS_OK);

	if (!parse_method_call(msg, &method_name, &params)) return;

	// =====>>>>> lock semaphore
	sem_wait(&dataCont->lock);

	if (!strcmp (method_name, DFMS_CALL_ACCEPT)) {
		service_handler_accept(msg, params);
	} else if (!strcmp (method_name, DFMS_CALL_DATAFORSINK)) {
		service_handler_dataForSink(msg, params);
	} else if (!strcmp (method_name, DFMS_CALL_SHUTDOWN)) {
		service_handler_shutdown(msg, params);
	} else if (!strcmp (method_name, DFMS_CALL_INSERTMECHANISM)) {
		service_handler_insertMechanism(msg, params);
	} else {
		soup_xmlrpc_set_fault (msg, SOUP_XMLRPC_FAULT_SERVER_ERROR_REQUESTED_METHOD_NOT_FOUND, "Unknown method %s", method_name);
	}
	//dataCont_print(dataCont, 2);

	// <<<<<===== release semaphore
	sem_post(&dataCont->lock);

	g_free (method_name);
	g_value_array_free (params);
}



/**
 * Initializes the service that waits for remote connections
 * to get information from the local data flow monitor
 */
int data_flow_monitor_service(dataCont_ptr dc) {
	threads_init();

	SoupServer *server;

	plog(LOG_TRACE, "Creating XML-RPC HTTP server port %d", DFMS_PORT);
	if (!(server = soup_server_new("port", DFMS_PORT, NULL))) {
		plog(LOG_ERROR, "Unable to bind SoupServer to port %d", DFMS_PORT);
		exit(1);
	}

	plog(LOG_INFO, "Adding handler [%s]", DFMS_NAME);
	soup_server_add_handler (server, DFMS_NAME, data_flow_monitor_handler, NULL, NULL);
	soup_server_run_async (server);

	dataCont = dc;

	session = soup_session_sync_new();

	return 0;
}








void service_handler_accept(SoupMessage *msg, GValueArray *params) {
	char local_socket_name[ADDRxADDR_STR_LEN];
	char remote_socket_name[ADDRxADDR_STR_LEN];

	char *local_cont_id = NULL;
	char *remote_cont_id = NULL;

	char *remote_ip;
	char *remote_port;
	char *local_ip;
	char *local_port;

	if (!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &remote_ip)) {
		type_error (msg, G_TYPE_STRING, params, 0);
		return;
	}

	if (!soup_value_array_get_nth (params, 1, G_TYPE_STRING, &remote_port)) {
		type_error (msg, G_TYPE_STRING, params,1);
		return;
	}

	if (!soup_value_array_get_nth (params, 2, G_TYPE_STRING, &local_ip)) {
		type_error (msg, G_TYPE_STRING, params, 2);
		return;
	}

	if (!soup_value_array_get_nth (params, 3, G_TYPE_STRING, &local_port)) {
		type_error (msg, G_TYPE_STRING, params, 3);
		return;
	}

	if (!soup_value_array_get_nth (params, 4, G_TYPE_STRING, &remote_cont_id)) {
		type_error (msg, G_TYPE_STRING, params, 4);
		return;
	}

	toADDRxADDR(local_socket_name,ADDRxADDR_STR_LEN,local_ip,local_port,remote_ip,remote_port);

	toADDRxADDR(remote_socket_name,ADDRxADDR_STR_LEN,remote_ip,remote_port,local_ip,local_port);

/*
	printf("++ accept():\n");fflush(NULL);
	printf("++ [remote ip:port = %s:%s]\n",remote_ip,remote_port);fflush(NULL);
	printf("++ [remote cont id = %s]\n", remote_cont_id);fflush(NULL);
	printf("++ [local socket name = %s]\n",local_socket_name);fflush(NULL);
*/

	// get local container id
	while (local_cont_id == NULL) {
		local_cont_id = dataCont_getNaming(dataCont, GLOBAL_ADDR, local_socket_name);

		sem_post(&dataCont->lock);
		usleep(33);					// wait for local connect()
		sem_wait(&dataCont->lock);
	}

	// add name for remote container
	dataCont_addNaming(dataCont, GLOBAL_ADDR, remote_socket_name, remote_cont_id);

	// add alias from local container to remote container
	dataCont_addAlias(dataCont, local_cont_id, remote_cont_id);

	// return the local container id
	soup_xmlrpc_set_response (msg, G_TYPE_STRING, local_cont_id);
}



char *service_caller_accept(char *remote_ip, char *remote_port, char *local_ip, char *local_port, char *local_cont_id) {

	GValue g_return_value;
	char *result = EMPTY_STRING;
	char ip[CANONICAL_IP_LEN+1];
	char service_uri[DFMS_SERVICE_URI_LEN];
/*
	if (session == NULL) {
		session = soup_session_sync_new();
	}
*/
	snprintf(service_uri,DFMS_SERVICE_URI_LEN,"http://%s:%d%s",getCanonicalIP(ip,CANONICAL_IP_LEN+1,remote_ip),DFMS_PORT,DFMS_NAME);

	sem_post(&dataCont->lock);

	while (!strcmp(result,EMPTY_STRING)) {
		if (do_xmlrpc (session, service_uri, DFMS_CALL_ACCEPT, &g_return_value,
						G_TYPE_STRING, local_ip,
						G_TYPE_STRING, local_port,
						G_TYPE_STRING, remote_ip,
						G_TYPE_STRING, remote_port,
						G_TYPE_STRING, local_cont_id,
						G_TYPE_INVALID)) {

			plog(LOG_TRACE, "Checking response");
			if (!check_xmlrpc (&g_return_value, G_TYPE_STRING, &result)) {
				plog(LOG_ERROR, "Protocol error in response.");
				exit(1);
			}
		}
/*		else if (tries == DFMS_MAX_CALL_TRIES) {
			plog(LOG_INFO, "Not available: %s",service_uri);
			exit(1);
		}
*/


		//usleep(33);				// sleep microseconds
	}

	//soup_session_abort(session);

	sem_wait(&dataCont->lock);

	return result;
}






void service_handler_dataForSink(SoupMessage *msg, GValueArray *params) {
	GValueArray *g_lod;
	GValueArray *g_mechanisms_per_data;
	GValueArray *g_mechanisms;

	int *mechanisms_per_data;
	char *cont;
	char *val;
	char **mechanisms;
	unsigned int i, j;



	if (!soup_value_array_get_nth (params, 0, G_TYPE_VALUE_ARRAY, &g_lod)) {
		type_error (msg, G_TYPE_VALUE_ARRAY, params, 0);
		exit(1);
	}

	if (!soup_value_array_get_nth (params, 1, G_TYPE_VALUE_ARRAY, &g_mechanisms_per_data)) {
		type_error (msg, G_TYPE_VALUE_ARRAY, params, 1);
		exit(1);
	}

	if (!soup_value_array_get_nth (params, 2, G_TYPE_VALUE_ARRAY, &g_mechanisms)) {
		type_error (msg, G_TYPE_VALUE_ARRAY, params, 2);
		exit(1);
	}

	if (!soup_value_array_get_nth (params, 3, G_TYPE_STRING, &cont)) {
		type_error (msg, G_TYPE_STRING, params, 3);
		exit(1);
	}


	for (i = 0; i < g_lod->n_values; i++) {
		if (!soup_value_array_get_nth (g_lod, i, G_TYPE_STRING, &val)) {
			type_error (msg, G_TYPE_STRING, params, i);
			exit(1);
		}

		// add all the data to the corresponding container
		dataCont_addDataCont_noUpdate(dataCont, val, cont);
	}


	// get the array with the number of mechanisms per data
	mechanisms_per_data = malloc(g_mechanisms_per_data->n_values * sizeof(int));
	for (i = 0; i < g_mechanisms_per_data->n_values; i++) {
		if (!soup_value_array_get_nth (g_mechanisms_per_data, i, G_TYPE_INT, &mechanisms_per_data[i])) {
			type_error (msg, G_TYPE_INT, params, i);
			exit(1);
		}
	}


	mechanisms = malloc(g_mechanisms->n_values * sizeof(char*));
	for (i = 0; i < g_mechanisms->n_values; i++) {
		if (!soup_value_array_get_nth (g_mechanisms, i, G_TYPE_STRING, &mechanisms[i])) {
			type_error (msg, G_TYPE_STRING, params, i);
			exit(1);
		}
	}

	for (int i = 0; i < g_mechanisms->n_values; i++) {
		xmlDocPtr d = xmlRecoverMemory(mechanisms[i], strlen(mechanisms[i]));
		xmlNodePtr rootNode = xmlDocGetRootElement(d);
		parse_and_add_mechanism(rootNode);
	}

	soup_xmlrpc_set_response (msg, G_TYPE_INT, 0);
}



void service_caller_dataForSink(char *remote_ip, list_of_data_ptr lod, unsigned int lod_len, char *cont, int *mechanisms_per_data, unsigned char **mechanisms) {
	GValue g_return_value;
	GValueArray *g_lod;
	GValueArray *g_mechanisms;
	GValueArray *g_mechanisms_per_data;



	int result;
	char ip[CANONICAL_IP_LEN+1];
	char service_uri[DFMS_SERVICE_URI_LEN];
	unsigned int i;
	unsigned int nr_of_mechanisms;
	list_of_data_ptr tmplod;


	// assemble list of data as array for SOUP
	g_lod = g_value_array_new(lod_len);
	for (tmplod = lod; tmplod; tmplod = tmplod->next) {
		soup_value_array_append(g_lod, G_TYPE_STRING, tmplod->data->data_ID);
	}


	// assemble mechanisms per data as array
	g_mechanisms_per_data = g_value_array_new(lod_len);
	nr_of_mechanisms = 0;
	for (i = 0; i < lod_len; i++) {
		nr_of_mechanisms += mechanisms_per_data[i];
		soup_value_array_append(g_mechanisms_per_data, G_TYPE_INT, mechanisms_per_data[i]);
	}

	// assemble mechanisms as array
	g_mechanisms = g_value_array_new(nr_of_mechanisms);
	for (i = 0; i < nr_of_mechanisms; i++) {
		soup_value_array_append(g_mechanisms, G_TYPE_STRING, mechanisms[i]);
	}

	snprintf(service_uri,DFMS_SERVICE_URI_LEN,"http://%s:%d%s",getCanonicalIP(ip,CANONICAL_IP_LEN+1,remote_ip),DFMS_PORT,DFMS_NAME);

	int success = 0;

	sem_post(&dataCont->lock);

	while (!success) {
		if (do_xmlrpc (session, service_uri, DFMS_CALL_DATAFORSINK, &g_return_value,
						G_TYPE_VALUE_ARRAY, g_lod,
						G_TYPE_VALUE_ARRAY, g_mechanisms_per_data,
						G_TYPE_VALUE_ARRAY, g_mechanisms,
						G_TYPE_STRING, cont,
						G_TYPE_INVALID)) {

			success = 1;

			plog(LOG_TRACE, "Checking response");
			if (!check_xmlrpc (&g_return_value, G_TYPE_INT, &result)) {
				plog(LOG_ERROR, "Protocol error in response.");
				g_value_array_free(g_lod);
				exit(1);
			}
		}
	}

	g_value_array_free(g_lod);

	sem_wait(&dataCont->lock);


}






void service_handler_shutdown(SoupMessage *msg, GValueArray *params) {
	char *cont_id;
	char *name;
	char *how;
	int i;
	int size;
	char ***names;

	if (!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &name)) {
		type_error (msg, G_TYPE_STRING, params, 0);
		exit(1);
	}

	if (!soup_value_array_get_nth (params, 1, G_TYPE_STRING, &how)) {
		type_error (msg, G_TYPE_STRING, params, 1);
		exit(1);
	}

/*
	printf("++ shutdown():\n");
	printf("++ [cont name = %s]\n", name);
	printf("++ [how = %s]\n", how);
*/

	// get the socket container: f((pid,sfd))
	cont_id = dataCont_getNaming(dataCont, GLOBAL_ADDR, name);

	// all names of container
	names = dataCont_getAllNamingsOfContainer(dataCont, cont_id, &size);


	if (!strcmp(how,SHUT_RD_STR) || !strcmp(how,SHUT_RDWR_STR)) {
		// disallow reception

		// empty storage function
		dataCont_emptyCont(dataCont,cont_id);

		// delete all aliases to container
		dataCont_delAllAliasesTo(dataCont, cont_id);
	}

	if (!strcmp(how,SHUT_WR_STR) || !strcmp(how,SHUT_RDWR_STR)) {
		// disallow transmission

		// delete all aliases from this container
		dataCont_delAllAliasesFrom(dataCont, cont_id);
	}

	if (!strcmp(how,SHUT_RDWR_STR)) {
		// disallow transmission and reception;
		// therefore delete all identifiers of type ADDR

		// delete all identifiers of type ADDR
		if (names != NULL) {
			for (i = 0; i < size; i++) {
				if (is_ADDR(names[i][0]) == 0) {
					dataCont_delNaming(dataCont, names[i][0], names[i][1]);
				}
				free(names[i][0]);
				free(names[i][1]);
				free(names[i]);
			}
			free(names);
		}
	}

	free(cont_id);

	soup_xmlrpc_set_response (msg, G_TYPE_INT, 0);
}


void service_caller_shutdown(char *remote_ip, char *name, char *how) {
	GValue g_return_value;
	char ip[CANONICAL_IP_LEN+1];
	char service_uri[DFMS_SERVICE_URI_LEN];
	int result;

/*
	if (session == NULL) {
		session = soup_session_sync_new();
	}
*/
	snprintf(service_uri,DFMS_SERVICE_URI_LEN,"http://%s:%d%s",getCanonicalIP(ip,CANONICAL_IP_LEN+1,remote_ip),DFMS_PORT,DFMS_NAME);

	int success = 0;

	sem_post(&dataCont->lock);

	while (!success) {
		if (do_xmlrpc (session, service_uri, DFMS_CALL_SHUTDOWN, &g_return_value,
						G_TYPE_STRING, name,
						G_TYPE_STRING, how,
						G_TYPE_INVALID)) {
			/*plog(LOG_INFO, "Not available: %s",service_uri);
			exit(1);*/
			success = 1;

			plog(LOG_TRACE, "Checking response");
			if (!check_xmlrpc (&g_return_value, G_TYPE_INT, &result)) {
				plog(LOG_ERROR, "Protocol error in response.");
				exit(1);
			}
		}
	}


	//soup_session_abort(session);

	sem_wait(&dataCont->lock);
}






void service_handler_insertMechanism(SoupMessage *msg, GValueArray *params) {
	char *mechanism;


	if (!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &mechanism)) {
		type_error (msg, G_TYPE_STRING, params, 0);
		exit(1);
	}

	xmlDocPtr d = xmlRecoverMemory(mechanism, strlen(mechanism));
	xmlNodePtr rootNode = xmlDocGetRootElement(d);
	parse_and_add_mechanism(rootNode);


	soup_xmlrpc_set_response (msg, G_TYPE_INT, 0);
}
