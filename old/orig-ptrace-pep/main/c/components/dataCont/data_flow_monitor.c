#include "data_flow_monitor.h"
#include "log_data_flow_monitor_c.h"

int deny_because_of_IF = 0;
char* gl_data2check = 0;

/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/

/*     THIS CODE IS CRAP */

/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/

//hash table... for the time being implemented as a matrix
//retval=index
int add_char2int(char* cont) {
	int i = 0;
	if (cont == NULL)
		return -1;
	for (i = 0; i < hashcount; i++) {
		if (hashtable[i] != NULL) {
			if (strcmp(hashtable[i], cont) == 0){
				printf ("add_char2int: %s-->%d (exists)\n",cont,i);
				return i;
			}

		}
	}
	//if the element is not already there, we add it
	hashtable[hashcount] = strdup(cont);
	printf ("add_char2int: %s-->%d (NEW!!!)\n",cont,hashcount);
	return hashcount++;
}

//hash table... for the time being implemented as a matrix
//retval=-1 --> error, else --> index
int char2int(char* cont) {
	int i = 0;
	if (cont == NULL)
		return -1;
	for (i = 0; i < hashcount; i++) {
		if (hashtable[i] != NULL) {
			if (strcmp(hashtable[i], cont) == 0){
				printf ("char2int: %s-->%d \n",cont,i);
				return i;
			}
		}
	}

	return -1;

}

char* int2char(int node) {
	if (node < hashcount) {
		return hashtable[node];
	} else {
		return NULL;
	}

}

//hash table... for the time being implemented as a matrix
//retval=index
int add_data2int(char* data) {
	int i = 0;
	if (data == NULL)
		return -1;
	for (i = 0; i < datahashcount; i++) {
		if (datahashtable[i] != NULL) {
			if (strcmp(datahashtable[i], data) == 0){
				printf ("add_data2int: %s-->%d (exists)\n",data,i);
				return i;
			}

		}
	}
	//if the element is not already there, we add it
	datahashtable[datahashcount] = strdup(data);
	printf ("add_data2int: %s-->%d (NEW!!!)\n",data,datahashcount);
	return datahashcount++;
}

//hash table... for the time being implemented as a matrix
//retval=-1 --> error, else --> index
int data2int(char* data) {
	int i = 0;
	if (data == NULL)
		return -1;
	for (i = 0; i < datahashcount; i++) {
		if (datahashtable[i] != NULL) {
			if (strcmp(datahashtable[i], data) == 0){
				printf ("data2int: %s-->%d \n",data,i);
				return i;
			}
		}
	}

	return -1;

}

char* int2data(int node) {
	if (node < datahashcount) {
		return datahashtable[node];
	} else {
		return NULL;
	}

}

/********************************************/

void qdft_print() {
	int x,y;
/*	printf("*********************\n");
	for (x=0; x<hashcount; x++) {
		printf("CONT[%d]=%s : ",x,hashtable[x]);
		for (y=0; y<datahashcount; y++) {
				printf(" (%d)=%d",y,qdftGet_quantity(qdft[y],x));
			}
		printf ("\n");
	}


	printf ("\n");
	for (x=0; x<datahashcount; x++) {
			printf("DATA[%d]=%s\n",x,datahashtable[x]);
	}*/
	/*printf("*********************\n");
    for (x = 0; x < qdft_size; x++) {
		printf("\nDATA %d\n", x);
		qdftShow_graph(qdft[x]);
		printf("\n");
	}
	printf("*********************\n");*/
	printf("*********************\n");
	qdftShow_graphs();
	qdftSave("/home/workspace/graphs.dot");
	printf("*********************\n");
}

/*
 void qdft_emptyNode(char* cont){

 if (cont==NULL){ log_error ("null container string in emptynode function"); return; }

 list_of_data_ptr lod=dataCont_getDataIn(data_flow_monitor->map ,cont);
 int length=list_of_data_length(lod);
 int dd=0;
 char* current_data;

 for (dd=0; dd<length; dd++){
 current_data= lod->data->data_ID;
 if (current_data==NULL) { log_error ("null data in list. emptynode."); return; }
 int datanode=data2int(current_data);
 int contnode=char2int(cont);

 if (datanode==-1){ log_error ("graph for data %s does not exists",current_data); return; }
 if (contnode==-1){ log_error ("cont %s does not exists",cont); return; }


 q=qdftGet_quantity(qdft[datanode],contnode);

 log_info("removing %d units of data(%d) from node %s(%d)...", q, datanode, cont, contnode);
 qdftRemove(qdft[datanode],q, contnode);
 log_info("...done!");

 }
 }
 */

int qdft_checkViolations(char * did){
	//policy: no more than <maxval>% of data <data> can be sent over the network
	//data to track
/*
	char data[MAX_DATA_ID];
	memset(data,'0',MAX_DATA_ID);
	data[0]='D';
	data[MAX_DATA_ID-2]='1';
	data[MAX_DATA_ID-1]='\0';
*/
	char* data = gl_data2check;

	char * Cnet="Cnet";
	//max size in percentage
	long maxval=40;

	/*int datanode = data2int(data);
	if (datanode == -1) {
		log_error ("checkViolations: graph for data %s does not exists",data);
		return;
	}*/


	//long qsrc = maxsize[datanode];
	long qsrc = qdftGet_data_amount (data);
	//int qnet = qdftGet_quantity(qdft[datanode], Cnet);
	int qnet = qdftGet_quantity(data, Cnet);

	log_error ("test: data=%s, qnet=%d, %ld,",data, qnet, (long)(((long)qsrc * (long)maxval) / 100));

	if (qnet > (long)(((long)qsrc * (long)maxval) / 100)){
		log_error ("checkViolations: violation:qnet=%d",qnet);
		did = strdup (data);
		return qnet;
	} else {
		did=strdup("niente");
		return -1;
	}

}




void qdft_removeFromNode(int sizea, char* cont, int oldSize) { //oldsize is only for ftruncate function, -1 otherwise

	log_info ("qdft remove invoked, size=%d, cont=%s oldSize=%d",sizea,cont,oldSize );


	if (cont == NULL) {
			log_error ("null container string in removeFromNode function");
			return;
		}

	if (sizea <=0) {
			log_error ("negative or null size. abort.");
			return;
		}

	list_of_data_ptr lod = dataCont_getDataIn(data_flow_monitor->map, cont);
	int length = list_of_data_length(lod);

	int dd;
	int q;
	char* current_data;
	int amount_to_remove=0;


	for (dd = 0; dd < length; dd++) {
		log_info("checking data %d/%d...",dd+1,length);
		current_data = lod->data->data_ID;
		if (current_data == NULL) {
			log_error ("null data in list. removeFromNode.");
			return;
		}
		/*int datanode = data2int(current_data);
		int contnode = char2int(cont);

		if (datanode == -1) {
			log_error ("graph for data %s does not exists",current_data);
			return;
		}
		if (contnode == -1) {
			log_error ("cont %s does not exists",cont);
			return;
		}
*/
		//q = qdftGet_quantity(qdft[datanode], cont);
		q = qdftGet_quantity(current_data, cont);
		log_info ("q=%d",q);


		if (oldSize!=-1) //ftruncate
			amount_to_remove=sizea-(oldSize-q); //UNDER ASSUMPTION THAT q IS ALWAYS < SIZE OF THE FILE

			//if a file of size X contains q units of data and we truncate it to size Y (Y<X), then
			//	sizea= X-Y
		    //  amount to remove = sizea - (X - q)= X-Y-X+q=       q-Y    if q-Y > 0
 	        //	                                                     0    otherwise

		    // new amount of data in file after truncate = q -amount to remove=   min (Y,q)
			//

		else amount_to_remove=sizea; //std behavior


		if (amount_to_remove >0){
			log_info("removing %d units of data(%s) from node %s...", amount_to_remove, current_data, cont);
			qdftRemove(current_data, amount_to_remove, cont);
					log_info("...done!");
		} else {
			log_info("no data removed (oldSize= %d, new size=%d, quantity of data %s in node %s=%d)", oldSize, oldSize-sizea, current_data, cont, q);
		}

		lod=lod->next;


	}

}

void qdft_transfNode(int sizea, char* source, char* dest) {

	log_info ("qdft invoked, size=%d, source=%s, dest=%s",sizea,source, dest);


	if ((sizea <=0)||(sizea==NULL)) {
			log_error ("negative or null size. abort.");
			return;
		}

	if ((source == NULL) || (dest == NULL)) {
		log_error ("null container string in transfnode function");
		return;
	}

	list_of_data_ptr lod = dataCont_getDataIn(data_flow_monitor->map, source);
	int length = list_of_data_length(lod);
	int dd = 0;
	char* current_data;

	for (dd = 0; dd < length; dd++) {
		current_data = lod->data->data_ID;

		if (current_data == NULL) {
			log_error ("null data in list. transfnode.");
			return;
		}

		log_info ("transfering, data=%s, size=%d, source=%s, dest=%s",current_data, sizea,source, dest);
		/*int datanode = data2int(current_data);
		int sourcenode = char2int(source);  ///NOT NEEDED
		int destnode = add_char2int(dest); //NOT NEEDED

		if (datanode == -1) {
			log_error ("graph for data %s does not exists",current_data);
			return;
		}
		if (sourcenode == -1) {
			log_error ("sourcenode %s does not exists",source);
			return;
		}*/

		log_info("transfering %d units of data(%s) from node %s to node %s...", sizea, current_data, source, dest);
		//oldEdge[datanode]=qdftTransfer(qdft[datanode], sizea, source, dest);
		qdftTransfer(current_data, sizea, source, dest);
		log_info("...done!");
		lod=lod->next;

	}

}

/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/
/********************************************/

data_flow_monitor_ptr data_flow_monitor_new() {
	data_flow_monitor_ptr dfm = (data_flow_monitor_ptr) malloc(
			sizeof(data_flow_monitor_t));
	dfm->layer = (char*) malloc(sizeof(char) * SIZE_LAYER);
	dfm->map = dataCont_new();
	return dfm;
}

void exampleForqdft() {
/*
	// Set initial amount of data per data
	qdftmanager_ptr test1 = qdftInit(5, 0);
	qdftmanager_ptr test2 = qdftInit(10, 1);
	qdftmanager_ptr test3 = qdftInit(8, 2);

	// test transfer
	qdftTransfer(test1, 2, 0, 1);
	qdftTransfer(test1, 1, 0, 1);

	log_info("// Data Flow Graph for data 0");
	qdftShow_graph(test1);
	log_info("// Data Flow Graph for data 1");
	qdftShow_graph(test2);
	log_info("// Data Flow Graph for data 2");
	qdftShow_graph(test3);

	// test decrease
	qdftRemove(test1, 5, 0);
	log_info("// Data Flow Graph for data 0");
	qdftShow_graph(test1);
	log_info("// Data Flow Graph for data 1");
	qdftShow_graph(test2);
	log_info("// Data Flow Graph for data 2");
	qdftShow_graph(test3);*/
}



/*******************
 *
 * Auxiliary function to split a list of strings received as event-parameter into an array
 *
 */
char** parseStringPar(char* par, int* num){
	/* TODO: Max length = MAX_LENGTH_PARAM_LIST parameters */

	char * str;
	(*num)=0;
	char ** res= (char **)malloc(sizeof (char*) * MAX_LENGTH_PARAM_LIST);
	str = strtok(par, ",");
	while (str!=NULL){
		res[(*num)]=strdup(str);
		str = strtok(NULL, ",");
//		plog (LOG_DEBUG, "%s, %s, %d",par,str,(*num));
		(*num)++;
	}

	return res;

}

/*******************
 *
 * Auxiliary function to split a list of integers received as event-parameter into an array
 *
 */
int* parseIntPar(char* par, int* num){
	/* TODO: Max length = MAX_LENGTH_PARAM_LIST parameters */
	char * str;
	(*num)=0;
	int* res= (int *)malloc(sizeof (int) * MAX_LENGTH_PARAM_LIST);
	str = strtok(par, ",");
	while (str!=NULL){
		if (strcmp(str,"INF")==0) res[(*num)]=INF;    // "INF" means sizea=infinity
		else res[(*num)]=atoi(str);
		str = strtok(NULL, ",");
//		plog (LOG_DEBUG, "%s, %s, %d",par,str,(*num));
		(*num)++;
	}

	return res;

}





void data_flow_monitor_init(data_flow_monitor_ptr data_flow_monitor) {
	char *tmpcont;
	char *tmpdata;
	int n = 0;

	for (n = 0; n < MAXNODES; n++)
		qdft[n] = NULL;
		maxsize[n] = 0;
		oldEdge[n] = 0;
	qdft_size = 0;

	hashcount = 0;
	datahashcount = 0;

	tmpdata = qdftGet_new_data(1, INIT_DATA_MODE);
	dataCont_newCont(data_flow_monitor->map, "Cnet");
	dataCont_newData(data_flow_monitor->map, tmpdata);
	dataCont_addDataCont(data_flow_monitor->map, tmpdata, "Cnet");
	//qdft_size++;
	//qdft[add_data2int("Dnet")] = qdftInit(1, "Cnet");
	qdftInit(tmpdata, 1, "Cnet");

	/*
	 dataCont_newData(data_flow_monitor->map, "DATA1");
	 dataCont_newData(data_flow_monitor->map, "DATA2");
	 dataCont_newData(data_flow_monitor->map, "DATA3");
	 dataCont_newData(data_flow_monitor->map, "DATA4");

	 tmpcont=dataCont_getNewContID(data_flow_monitor->map);
	 tmpcont[1]='-';
	 tmpcont[2]='F';
	 tmpcont[3]='I';
	 tmpcont[4]='L';
	 tmpcont[5]='E';
	 dataCont_newCont(data_flow_monitor->map, tmpcont);
	 dataCont_addNaming(data_flow_monitor->map, "FNAME", "test0", tmpcont);
	 dataCont_addDataCont (data_flow_monitor->map, "DATA0",tmpcont);


	 tmpcont=dataCont_getNewContID(data_flow_monitor->map);
	 tmpcont[1]='-';
	 tmpcont[2]='F';
	 tmpcont[3]='I';
	 tmpcont[4]='L';
	 tmpcont[5]='E';
	 dataCont_newCont(data_flow_monitor->map, tmpcont);
	 dataCont_addNaming(data_flow_monitor->map, "FNAME", "test1", tmpcont);
	 dataCont_addDataCont (data_flow_monitor->map, "DATA1",tmpcont);

	 tmpcont=dataCont_getNewContID(data_flow_monitor->map);
	 tmpcont[1]='-';
	 tmpcont[2]='F';
	 tmpcont[3]='I';
	 tmpcont[4]='L';
	 tmpcont[5]='E';
	 dataCont_newCont(data_flow_monitor->map, tmpcont);
	 dataCont_addNaming(data_flow_monitor->map, "FNAME", "test2", tmpcont);
	 dataCont_addDataCont (data_flow_monitor->map, "DATA2",tmpcont);

	 tmpcont=dataCont_getNewContID(data_flow_monitor->map);
	 tmpcont[1]='-';
	 tmpcont[2]='F';
	 tmpcont[3]='I';
	 tmpcont[4]='L';
	 tmpcont[5]='E';
	 dataCont_newCont(data_flow_monitor->map, tmpcont);
	 dataCont_addNaming(data_flow_monitor->map, "FNAME", "test3", tmpcont);
	 dataCont_addDataCont (data_flow_monitor->map, "DATA3",tmpcont);
	 dataCont_addDataCont (data_flow_monitor->map, "DATA4",tmpcont);
	 dataCont_addDataCont (data_flow_monitor->map, "DATA5",tmpcont);

	 tmpcont=dataCont_getNewContID(data_flow_monitor->map);
	 tmpcont[1]='-';
	 tmpcont[2]='F';
	 tmpcont[3]='I';
	 tmpcont[4]='L';
	 tmpcont[5]='E';
	 dataCont_newCont(data_flow_monitor->map, tmpcont);
	 dataCont_addNaming(data_flow_monitor->map, "FNAME", "test4", tmpcont);
	 dataCont_addDataCont (data_flow_monitor->map, "DATA1",tmpcont);
	 dataCont_addDataCont (data_flow_monitor->map, "DATA4",tmpcont);
	 dataCont_addDataCont (data_flow_monitor->map, "DATA6",tmpcont);
	 */
}

int get_deny_because_of_if() {
	return deny_because_of_IF;
}

void reset_deny_because_of_if() {
	deny_because_of_IF = 0;
}

unsigned int data_flow_monitor_reset(data_flow_monitor_ptr data_flow_monitor) {
	unsigned int result = 0;
	data_flow_monitor_ptr tmp = data_flow_monitor_new();
	free(data_flow_monitor->layer);
	result = dataCont_free(data_flow_monitor->map);
	data_flow_monitor->layer = tmp->layer;
	data_flow_monitor->map = tmp->map;
	return result;
}

// source code should be updated to use INT-params appropriately!
char *tmpConvertIntParamToString(event_ptr event, char *paramName)
{
	int tmpInt = event_get_param_value_by_name_int(event, paramName);
	//log_warn("x1 - converting int-param([%s]) back to string for comparison", paramName);
	char *tmp = (char*) malloc(16 * sizeof(char));
	snprintf(tmp, 16, "%d", tmpInt);
	log_warn("x1 - converting int-param([%s]) back to string for comparison", paramName);
	return tmp;
}

void setInitData(char* d, unsigned int * q){ log_trace("invoking setInitData %s ---- (%x)%d",d,q,*q); qdftSet_data_amount(d,*q); }


unsigned int data_flow_monitor_update(data_flow_monitor_ptr data_flow_monitor,
		event_ptr event) {
	int syscall = 0;
	int loop = 0;
	char *tmpname;
	char *tmpname2;
	char** list;
	int size, x, sizea;
	size = 0; // initialize, otherwise problems with empty naming relation and if size is occasionally >0

	list_of_data_ptr lod;
	list_of_data_ptr tmplod;
	list_of_cont_ptr loc;

	dataCont_ptr dc = data_flow_monitor->map;

	//char PID[7];
	char *PID;
	char* tmpPID;
	//char *PID2;
	char PID2[7];
	char* tmpPID2;

	char* USER = "";
	char* FD = "";
	char* FN = "";
	char* FLAGS = "";
	char* RETVAL = "";
	char* FDSRC = "";
	char* FDDST = "";
	char* PROT = "";


	char* ACTION = event->action_desc->action_name;

	int SIZEA = event_get_param_value_by_name_int(event, "sizea");          //IN FFOX EVENTS SIZEA IS A STRING
	int OLDSIZE = event_get_param_value_by_name_int(event, "oldSize");


/* FIREFOX EVENTS parameters
 ***************************/

	char* TARGET = event_get_param_value_by_name_string(event, "obj");
	char* EVERYTHING_SELECTED_FLAG = event_get_param_value_by_name_string(event, "all");
	char* LIST_OF_INIT_DATA = event_get_param_value_by_name_string(event, "init_data");


	char* SIZEA_STR = event_get_param_value_by_name_string(event, "sizea"); //IN FFOX EVENTS SIZEA IS A STRING (possibly containing a list of values)
	int* SIZEA_ARR = NULL; //Array of values of sizea (for copy and cut events, see below)
	int all_selected = 0;
	char** par_arr;
	char** data_arr;
	int len=0;
	int len_arr=0;   //NB: len and len_arr should always be the same. Nevertheless, they are both retrieved and checked for equality as "checksum"
	int i,l;
/***************************/


	if (event != NULL) {
		syscall = syscallToInt(ACTION);
	}

	deny_because_of_IF = 0;
	unsigned int nParams = g_hash_table_size(event->params);

	//tmpPID=event_get_param_value_by_name_string(event, "pid");
	//tmpPID=tmpConvertIntParamToString(event, "pid");
	PID = tmpConvertIntParamToString(event, "pid");
	//if(tmpPID!=NULL)
	//{
	//  for(x=0; x<6; x++) PID[x]=tmpPID[1+x];
	//  PID[6]='\0';
	//}

	//log_error("x1 - got pid=[%d] -> converted to PID=[%s]", event_get_param_value_by_name_int(event, "pid"), PID);
	USER = event_get_param_value_by_name_string(event, "user");
	RETVAL = event_get_param_value_by_name_string(event, "retval");

	switch (syscall) {
	case 1: /* -- exit --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - status */

		qdft_removeFromNode(INF, PID,-1);
		dataCont_emptyCont(dc, PID);


		dataCont_delAllAliasesFrom(dc, PID);
		dataCont_delAllAliasesTo(dc, PID);
		dataCont_delAllNamingFor(dc, PID);
		dataCont_removeCont(dc, PID);
		//dataCont_print(dc,1);


		break;

	case 2: // -- fork  --
	case 66: /* -- vfork --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - retval (pid of the child -> PID2) */

		if (RETVAL != NULL) {
			for (x = 0; x < 6; x++)
				//PID2[x] = RETVAL[1 + x];
				PID2[x] = RETVAL[x];
			PID2[6] = '\0';
		}

		//get s(PID)
		lod = dataCont_getDataIn(dc, PID);

		//add s(PID) to s(PID2)
		dataCont_newCont(dc, PID2);

		dataCont_addLodCont_noUpdate(dc, lod, PID2);
		printf("FORK:");
		qdft_transfNode(INF, PID, PID2);

		//l(PID2) = l(PID)
		//dataCont_addAlias(dc, PID2, PID);  ---> WRONG IN OLD IMPLEMENTATIONS

		//get list of containers that are aliases for PID
		list = dataCont_getAliasesTo(dc, PID, &size);

		//for each container c | PID in l(c)   => PID2 in l(c)
		if (list != NULL) {
			for (loop = 0; loop < size; loop++)
				dataCont_addAlias(dc, list[loop], PID2);
		}
		//get list of Aliases (not closure) from PID
		free(list);
		list = dataCont_getAliasesOf_noUpdate(dc, PID, &size);

		//add all of them to PID2
		if (list != NULL) {
			for (loop = 0; loop < size; loop++)
				dataCont_addAlias(dc, PID2, list[loop]);
		}
		list = dataCont_getAllNamingsFrom(dc, PID, &size);

		//add all of them to PID2
		if (list != NULL) {
			for (loop = 0; loop < size; loop++) {
				tmpname = dataCont_getNaming(dc, PID, list[loop]);
				dataCont_addNaming(dc, PID2, list[loop], tmpname);
			}
		}
		break;

	case 3: // -- read     --
	case 29: // -- recvfrom --
	case 27: // -- recvmsg  --
	case 173: // -- pread    --
	case 120: /* -- readv    --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - filename
	 * 4 - file descriptor  */

		//FD=event_get_param_value_by_name_string(event, "fd");
		FD = tmpConvertIntParamToString(event, "fd");
		//get s(f(PID,FD))
		tmpname = dataCont_getNaming(dc, PID, FD);
		lod = dataCont_getDataIn(dc, tmpname);

		//add s(f(pid,fd)) to s(pid)
		dataCont_addLodCont(dc, lod, PID);

		printf("READ:");
		qdft_transfNode(SIZEA, tmpname, PID);

		//add s(f(pid,fd)) to every c in l(s(pid))
		list = dataCont_getAliasesOf(dc, PID, &size);
		if (list != NULL) {
			for (loop = 0; loop < size; loop++){
				dataCont_addLodCont(dc, lod, list[loop]);
				printf("READ(loop):");
				qdft_transfNode(SIZEA, tmpname, list[loop]);
			}
		}
		break;

	case 4: // -- write     --
	case 28: // -- sendmsg   --
	case 121: // -- writev    --
	case 133: // -- sendto    --
	case 174: // -- pwrite    --
	case 201: /* -- ftruncate --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - filename
	 * 4 - file descriptor  */

		//FD=event_get_param_value_by_name_string(event, "fd");
		FD = tmpConvertIntParamToString(event, "fd");
		//MANUAL WAY
		//get f(PID,FD)
		tmpname = dataCont_getNaming(dc, PID, FD);

		//get l*(f(PID,FD))
		list = dataCont_getAliasesOf(dc, tmpname, &size);

		//add s(PID) to f(PID,FD)
		lod = dataCont_getDataIn(dc, PID);

		if (tmpname != NULL) {
			dataCont_addLodCont(dc, lod, tmpname);

			if (syscall == 201) { //ftruncate
				log_info("ftruncate invoked. sizea=%d",SIZEA);

				/*					log_info("adding %d units of data from pid to file",sizeof(size_t));
				 qdft_transfNode(sizeof(size_t), PID, tmpname);
				 */
				log_info("updating units of data in file according to new size");
				qdft_removeFromNode(-SIZEA, tmpname, OLDSIZE);
			} else {
				printf("WRITE:");
				qdft_transfNode(SIZEA, PID, tmpname);
			}

			//if ((strcmp(tmpname, "Cnet") == 0) && (list_of_data_find(lod,"D0000000000000") == 0)) {
			long dunit = -1;
			char* did;
			dunit = qdft_checkViolations(did);
			log_info("dfmonitor. dunit=%d, did=%s",dunit,did);
			if (dunit != -1) {
				printf("Sending %d units of data %s to Cnet --> forbidden! \n",
						SIZEA, did);
				deny_because_of_IF = 1;
				IF_revert(event);
				//qdft_print();
				break;

			}

			tmplod = dataCont_getDataIn(dc, tmpname);

			//add s(PID) to each element in l*(f(PID,FD))
			if (list != NULL) {
				for (loop = 0; loop < size; loop++) {
					dataCont_addLodCont_noUpdate(dc, lod, list[loop]);
					qdft_transfNode(SIZEA, PID, list[loop]);

					//ftruncate
					//
					// IF WE CONSIDER FTRUNCATE AS A FUNCTION THAT SENDS sizeof(int) BITS
					// OF INFORMATION ( THE NEW SIZE OF THE FILE IS AN INT, SO WE MAY BE CONVEYING THIS AMOUNT
					// OF DATA) JUST UNCOMMENT NEXT LINE.
					//
					// THE REASON WHY WE DON'T CONSIDER IT IS THAT IT DOESN'T FIT OUR MODEL OF DATA
					// (METADATA, LIKE SIZE, CORRESPONDS TO A SIDE-CHANNEL IN OUR MODEL, AND THEREFORE IS NOT CONSIDERED)
					//
					//if (syscall != 201)	{printf("WRITE(loop):"); qdft_transfNode(SIZEA, PID, list[loop]);}
				}
			}

			//AUTOMATIC WAY
			//dataCont_addLodCont(dc,lod,tmpname);
			//dataCont_print(dc,1);
		}

		break;

	case 5: /* -- open --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - filename
	 * 4 - flags
	 * 5 - retval  */

		FN = event_get_param_value_by_name_string(event, "filename");
		FLAGS = event_get_param_value_by_name_string(event, "flags");

		if (nParams == 6) // should be true in any way as we are in open?!
		{ // ok, not the case, only true for SYS_MSG_RES !!!! so why not use this as condition???
			//add naming "FNAME" x FN -> FN by assumption
			tmpname = dataCont_getNaming(dc, "FNAME", FN);

			if (tmpname == NULL) { //if file has no content yet, let's start to monitor it
				tmpname = dataCont_getNewContID(dc);

				tmpname[1] = '-';
				tmpname[2] = 'F';
				tmpname[3] = 'I';
				tmpname[4] = 'L';
				tmpname[5] = 'E';

				dataCont_newCont(dc, tmpname);
				dataCont_addNaming(dc, "FNAME", FN, tmpname);
			}

			//if the file is opened with flag O_TRUNC, the old content is lost
			if (FLAGS[5] == '1'){
				qdft_removeFromNode(INF, tmpname,-1);
				dataCont_emptyCont(dc, tmpname);

			}

			//add naming PID x FD -> C("FNAME" x FN) by model
			dataCont_addNaming(dc, PID, RETVAL, tmpname);

			//dataCont_print(dc,1);
		}
		break;

	case 6: /* -- close --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - filename
	 * 4 - file descriptor  */

		/*if ((FD!=NULL)&&(strcmp(FD,"1")==0)){
		 tmpname=dataCont_getNaming(dc, PID, FD);
		 dataCont_delAlias(dc,PID,FD);
		 }*/

		//FD=event_get_param_value_by_name_string(event, "fd");
		FD = tmpConvertIntParamToString(event, "fd");
		//del naming PID x FD -> C(PID x FN) by model
		dataCont_delNaming(dc, PID, FD);
		break;

	case 10: /* -- unlink --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - filename  */

		FN = event_get_param_value_by_name_string(event, "filename");

		//get f(PID,FN)
		tmpname = dataCont_getNaming(dc, "FNAME", FN);
		if (tmpname != NULL) {
			//s(PID,FN)<-nil
			qdft_removeFromNode(INF, tmpname,-1);
			dataCont_emptyCont(dc, tmpname);


			//l(f(pid,fn)<-nil)
			dataCont_delAllAliasesFrom(dc, tmpname);

			//remove f(pid,fn) from every alias destination
			dataCont_delAllAliasesTo(dc, tmpname);

			dataCont_delNaming(dc, "FNAME", FN);
		}
		//dataCont_print(dc,1);
		break;

	case 30: // -- accept  --
	case 97: /* -- socket  --
	 //case 98: /* -- connect --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - "domain/type" | sockfd
	 * 4 - retval (FD) */

		//add naming PID x FD -> Cnet by model
		FD = RETVAL;
		dataCont_addNaming(dc, PID, FD, "Cnet");
		//dataCont_print(dc,1);
		break;

	case 37: /* -- kill  --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - target
	 * 4 - signal
	 * 5 - retval  */

		tmpPID2 = event_get_param_value_by_name_string(event, "target");
		if (tmpPID2 != NULL) {
			for (x = 0; x < 6; x++)
				PID2[x] = tmpPID2[1 + x];
			PID2[6] = '\0';
		}

		//MANUAL WAY
		//get s(PID)
		lod = dataCont_getDataIn(dc, PID);

		//add s(PID) to l*(PID2)
		list = dataCont_getAliasesOf(dc, PID2, &size);

		dataCont_addLodCont_noUpdate(dc, lod, PID2);
		printf("KILL:");
		qdft_transfNode(SIZEA, PID, PID2);

		if (list != NULL) {
			for (loop = 0; loop < size; loop++){
				dataCont_addLodCont_noUpdate(dc, lod, list[loop]);
				printf("KILL(loop):");
				qdft_transfNode(SIZEA, PID, list[loop]);}
		}
		//AUTOMATIC WAY
		//dataCont_addLodCont(dc,lod,PID2);
		//dataCont_print(dc,1);
		break;

	case 41: // -- dup   --
	case 90: // -- dup2  --
	case 92: /* -- fcntl --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - file descriptor -> FDSRC (old descriptor)
	 * 4 - retval          -> FD    (new descriptor)  */

		/*
		 * N.B : fcntl is detected only if the parameter CMD is F_DUPFD. the check is performed in syscall_handler
		 */

		//FDSRC=event_get_param_value_by_name_string(event, "fd");
		FDSRC = tmpConvertIntParamToString(event, "fd");
		FD = RETVAL;

		//get f(PID, FDSRC)
		tmpname = dataCont_getNaming(dc, PID, FDSRC);

		// f(PID, FD)= f(PID, FDSRC)
		dataCont_addNaming(dc, PID, FD, tmpname);

		/*
		 if ((FD!=NULL)&&(strcmp(FD,"1")==0)){ //if the new descriptor is STDOUT, then "write" actions to that descriptor can't be detected, so for a conservative approach, we assume that one write happened
		 //get f(PID,FD)
		 tmpname=dataCont_getNaming(dc, PID, FD);

		 //add s(PID) to f(PID,FD)
		 lod=dataCont_getDataIn(dc, PID);
		 dataCont_addLodCont(dc,lod,tmpname);

		 //add s(PID) to l*(f(PID,FD))
		 dataCont_addLodCont(dc,lod,tmpname);

		 dataCont_addAlias(dc,PID,tmpname);
		 }
		 */

		//dataCont_print(dc,1);
		break;

	case 59: /* -- execve --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - filename  */

		FN = event_get_param_value_by_name_string(event, "filename");

		//get f(PID,FN)
		tmpname = dataCont_getNaming(dc, "FNAME", FN);

		//add s(f(PID,FN)) to s(PID)
		lod = dataCont_getDataIn(dc, tmpname);
		dataCont_addLodCont_noUpdate(dc, lod, PID);
		printf("EXECVE:");
		qdft_transfNode(SIZEA, tmpname, PID);

		/*
		 * TO CHECK! I don't buy the semantic of the action at page 21!
		 */
		//dataCont_print(dc,1);
		break;

	case 73: /* -- munmap --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - start
	 * 4 - length
	 * 5 - retval   */
		break;

	case 128: /* -- rename --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - old file name FDSRC
	 * 4 - new file name FDDST  */

		FDSRC = event_get_param_value_by_name_string(event, "oldfilename");
		FDDST = event_get_param_value_by_name_string(event, "newfilename");

		//create new container for the pipe (if it has been correctly created)
		tmpname = dataCont_getNaming(dc, "FNAME", FDSRC);
		tmpname2 = dataCont_getNaming(dc, "FNAME", FDDST);

		//empty destination container (for future usage of the old file descriptor)
		qdft_removeFromNode(INF, tmpname2,-1);
		dataCont_emptyCont(dc, tmpname2);


		//add memory of the process to new name
		lod = dataCont_getDataIn(dc, PID);
		dataCont_addLodCont_noUpdate(dc, lod, tmpname);
		printf("RENAME:");
		qdft_transfNode(INF, PID, tmpname);

		//swap the naming
		dataCont_addNaming(dc, "FNAME", FDSRC, tmpname2);
		dataCont_addNaming(dc, "FNAME", FDDST, tmpname);

		//dataCont_print(dc,1);
		break;

	case 197: /* -- mmap --
	 * Parameters: command user start length prot flags fd offset (+retval)
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - start
	 * 4 - length
	 * 5 - prot       PROT
	 * 6 - flags
	 * 7 - fd         FDSRC
	 * 8 - offset
	 * 9 - retval            */

		eventLog("processing mmap-syscall with event: ", event);

		PROT = event_get_param_value_by_name_string(event, "prot");
		//FDSRC=event_get_param_value_by_name_string(event, "fd");
		FDSRC = tmpConvertIntParamToString(event, "fd");
		if (strcmp(FDSRC, "-1") != 0) {
			///get f(p,e)
			tmpname = (dataCont_getNaming(dc, PID, FDSRC));

			/*
			 if (tmpname==NULL){
			 printf ("\n********************************************** f(%s,%s) NOT FOUND ***********************\n\n",PID,FDSRC);
			 break;
			 }
			 //content of f(p,e)
			 */
			lod = dataCont_getDataIn(dc, tmpname);

			//    MANUAL
			//
			//add lod to s(P)
			dataCont_addLodCont_noUpdate(dc, lod, PID);
			printf("MMAP:");
			qdft_transfNode(SIZEA, tmpname, PID);


			//add lod to l*(P)
			list = dataCont_getAliasesOf(dc, PID, &size);
			for (x = 0; x < size; x++){
				dataCont_addLodCont_noUpdate(dc, lod, list[x]);
				printf("MMAP(loop):");
				qdft_transfNode(SIZEA, tmpname, list[x]);
			}

			//      AUTOMATIC
			//      add lod to P and to l*(P)
			//      dataCont_addLodCont(dc, lod, tmpname);
			if (PROT[14] == '1') { //READ AND WRITE
				//content of f(p,e)
				lod = dataCont_getDataIn(dc, PID);
				//     MANUAL
				//add lod to f(p,e)
				dataCont_addLodCont_noUpdate(dc, lod, tmpname);
				qdft_transfNode(SIZEA, PID, tmpname);
				printf("MMAPrw:");

				//add lod to l*(f(p,e))
				list = dataCont_getAliasesOf(dc, tmpname, &size);
				for (x = 0; x < size; x++){
					dataCont_addLodCont_noUpdate(dc, lod, list[x]);
					printf("MMAPrw(loop):");
					qdft_transfNode(SIZEA, PID, list[x]);
				}
			}

			dataCont_addAlias(dc, tmpname, PID);

			if (PROT[14] == '1') //READ AND WRITE
				dataCont_addAlias(dc, PID, tmpname);

			//dataCont_print(dc,1);
		}
		break;

	case 200: /* -- truncate --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - filename
	 * 4 - file descriptor  */

		FN = event_get_param_value_by_name_string(event, "filename");

		//get f(PID,FN)
		tmpname = dataCont_getNaming(dc, "FNAME", FN);

		//MANUAL WAY
		//get l*(f(PID,FN))
		list = dataCont_getAliasesOf(dc, tmpname, &size);

		//add s(PID) to f(PID,FN)
		lod = dataCont_getDataIn(dc, PID);

		log_info("truncate invoked. sizea=%d",SIZEA);

		log_info("adding %d units of data from pid to file",sizeof(size_t));
		dataCont_addLodCont(dc, lod, tmpname);
		qdft_transfNode(sizeof(size_t), PID, tmpname);

		log_info("updating units of data in file according to new size");
		qdft_removeFromNode(-SIZEA, tmpname, OLDSIZE);

		//add s(PID) to each element in l*(f(PID,FD))
		if (list != NULL) {
			for (loop = 0; loop < size; loop++)
				dataCont_addLodCont_noUpdate(dc, lod, list[loop]);
		}
		//AUTOMATIC WAY
		//dataCont_addLodCont(dc,lod,tmpname);

		//dataCont_print(dc,1);
		break;

	case 263: /* -- pipe --
	 * 0 - pid
	 * 1 - command
	 * 2 - user
	 * 3 - file descriptor source
	 * 4 - file descriptor dest
	 * 5 - retvalue               */

		//FDSRC=event_get_param_value_by_name_string(event, "source");
		FDSRC = tmpConvertIntParamToString(event, "source");
		//FDDST=event_get_param_value_by_name_string(event, "dest");
		FDDST = tmpConvertIntParamToString(event, "dest");

		if (nParams == 6) {
			if (strcmp(RETVAL, "0") == 0) {
				//create new container for the pipe (if it has been correctly created)
				tmpname = dataCont_getNewContID(dc);

				tmpname[1] = '-';
				tmpname[2] = 'P';
				tmpname[3] = 'I';
				tmpname[4] = 'P';
				tmpname[5] = 'E';

				/*        printf("\n\PIPE PID=%s FDSRC=%s FDDST=%s \n\n", PID, FDSRC, FDDST);*/
				dataCont_newCont(dc, tmpname);

				//add naming PID x FDSRC -> Cp
				dataCont_addNaming(dc, PID, FDSRC, tmpname);

				//add naming PID x FDDST -> Cp
				dataCont_addNaming(dc, PID, FDDST, tmpname);

				//dataCont_print(dc,1);
			}
		}
		break;


/****************************************************************************************
 ****************************************************************************************
 ********************************                 ***************************************
 ********************************  F i r e f o x  ***************************************
 ***************************                            *********************************
 *************************** Information Flow Semantics *********************************
 ***************************                            *********************************
 ****************************************************************************************
 ****************************************************************************************
 ****************************************************************************************
 ****************************************************************************************
 ****************************************************************************************/

	case -1: // -- System call not recognized == FFOX action --

		//*   	*_int stands for events concerning the internal clipboard, *_ext for the system clipboard
		//* 	each copy and cut event generates two events, one for the internal clipboard and one for the system

		if (EVERYTHING_SELECTED_FLAG!=NULL) all_selected=(strcmp(EVERYTHING_SELECTED_FLAG,"TRUE")==0);

		if (strcmp(ACTION, "copy_int")==0){
			log_info("\n\n\ncopy_int event\n\n\n\n\n");

			SIZEA_ARR=parseIntPar (SIZEA_STR, &len_arr);
			par_arr=parseStringPar (TARGET, &len); //in a copy command, TARGET may be a list of containers

			if (len!=len_arr){
				log_error ("len=%d != len_arr=%d. Break! (Clipboard not modified)",len,len_arr);
				break;
			}

			qdft_removeFromNode(INF, FFOX_CLIPBOARD,-1);
			dataCont_emptyCont(dc, FFOX_CLIPBOARD);



			for (i=0; i<len; i++){

				//lod= dataCont_getDataIn(dc, dataCont_getNaming(dc,"FNAME",par_arr[i]));
				lod= dataCont_getDataIn(dc, par_arr[i]);
				//qdft_transfNode(SIZEA_ARR[i], dataCont_getNaming(dc,"FNAME",par_arr[i]), FFOX_CLIPBOARD);
				qdft_transfNode(SIZEA_ARR[i], par_arr[i], FFOX_CLIPBOARD);
				dataCont_addLodCont(dc,lod, FFOX_CLIPBOARD);
			}

		} else if (strcmp(ACTION, "copy_ext")==0){
			log_info("\n\n\ncopy_ext event\n\n\n\n\n");

			SIZEA_ARR=parseIntPar (SIZEA_STR, &len_arr);
			par_arr=parseStringPar (TARGET, &len); //in a copy command, TARGET may be a list of containers

			if (len!=len_arr){
				log_error ("len=%d != len_arr=%d. Break! (Clipboard not modified)",len,len_arr);
				break;
			}

			qdft_removeFromNode(INF, FFOX_CLIPBOARD,-1);
			dataCont_emptyCont(dc, FFOX_CLIPBOARD);

			for (i=0; i<len; i++){
				//lod= dataCont_getDataIn(dc, dataCont_getNaming(dc,"FNAME",par_arr[i]));
				lod= dataCont_getDataIn(dc, par_arr[i]);
				//qdft_transfNode(SIZEA_ARR[i], dataCont_getNaming(dc,"FNAME",par_arr[i]), FFOX_CLIPBOARD);
				qdft_transfNode(SIZEA_ARR[i], par_arr[i], FFOX_CLIPBOARD);
				dataCont_addLodCont(dc,lod, FFOX_CLIPBOARD);
			}

		} else if (strcmp(ACTION, "paste")==0){
			log_info("\n\n\npaste event (flag=%d)\n\n\n\n\n",all_selected);
			lod= dataCont_getDataIn(dc, FFOX_CLIPBOARD);
			SIZEA_ARR=parseIntPar (SIZEA_STR, &len_arr);
			if (len_arr!=1){
				log_error ("len!=1. Break! (Paste not executed)");
				break;
			}
			if (all_selected){
				qdft_removeFromNode(INF, TARGET,-1);
				dataCont_emptyCont(dc,TARGET);
			}
			qdft_transfNode(SIZEA_ARR[0], FFOX_CLIPBOARD, TARGET); //when pasting, we transfer all the content of the clipboard -> sizea=INF
			dataCont_addLodCont(dc,lod,TARGET);

		} else if (strcmp(ACTION, "cut_int")==0){
			log_info("\n\n\ncut_int event (flag=%d)\n\n\n\n\n",all_selected);
			SIZEA_ARR=parseIntPar (SIZEA_STR, &len_arr);

			qdft_removeFromNode(INF, FFOX_CLIPBOARD,-1);
			dataCont_emptyCont(dc, FFOX_CLIPBOARD);

			lod= dataCont_getDataIn(dc, TARGET); //in a cut command, the TARGET is always one container, because you can cut only within a single textbox

			qdft_transfNode(SIZEA_ARR[0], TARGET, FFOX_CLIPBOARD); //sizea is unique, because is the size of the selected text, i.e. SIZE_ARR[0]
			dataCont_addLodCont(dc,lod,FFOX_CLIPBOARD);

			if (all_selected){
				qdft_removeFromNode(INF, TARGET,-1);
				dataCont_emptyCont(dc, TARGET);
			}

		} else if (strcmp(ACTION, "cut_ext")==0){
			log_info("\n\n\ncut_ext event (flag=%d)\n\n\n\n\n",all_selected);
			SIZEA_ARR=parseIntPar (SIZEA_STR, &len_arr);

			qdft_removeFromNode(INF, FFOX_CLIPBOARD,-1);
			dataCont_emptyCont(dc, FFOX_CLIPBOARD);

			lod= dataCont_getDataIn(dc, TARGET); //in a cut command, the TARGET is always one container, because you can cut only within a single textbox

			qdft_transfNode(SIZEA_ARR[0], TARGET, FFOX_CLIPBOARD); //sizea is unique, because is the size of the selected text, i.e. SIZE_ARR[0]
			dataCont_addLodCont(dc,lod,FFOX_CLIPBOARD);

			if (all_selected){
				qdft_removeFromNode(INF, TARGET,-1);
				dataCont_emptyCont(dc, TARGET);
			}

		} else if (strcmp(ACTION, "reset")==0){
			log_info("\n\n\nreset event \n\n\n\n\n");
			par_arr=parseStringPar (TARGET, &len);

			for (i=0; i<len; i++){
				qdft_removeFromNode(INF, par_arr[i],-1);
				dataCont_emptyCont(dc,par_arr[i]);
			}

		} else if (strcmp(ACTION, "submit")==0){
			log_info("\n\n\nsubmit event \n\n\n\n\n");

		} else if (strcmp(ACTION, "erase")==0){
			log_info("\n\n\nerase event \n\n\n\n\n");
			qdft_removeFromNode(INF, TARGET,-1);
			dataCont_emptyCont(dc,TARGET);

		} else if (strcmp(ACTION, "load_page")==0){
			log_info("\n\n\nload page event\n\n\n\n\n");

			SIZEA_ARR=parseIntPar (SIZEA_STR, &len_arr);
			par_arr=parseStringPar (TARGET, &len);
			if (len!=len_arr){
				log_error ("[LOAD]len_size=%d != len_arr=%d. Break! (Containers not added)",len,len_arr);
				break;
			}

			data_arr=parseStringPar (LIST_OF_INIT_DATA, &len_arr); //TODO:extend cont-data to 1-n (now 1-1)
			if (len!=len_arr){
				log_error ("[LOAD]len_size=%d != len_data_arr=%d. Break! (Containers not added)",len,len_arr);
				break;
			}

			GHashTable *maxSizeInitData=g_hash_table_new(g_str_hash, g_str_equal);

			for (i=0; i<len; i++){
				int *ind=(int*) g_hash_table_lookup(maxSizeInitData, data_arr[i]);

				if (ind==NULL){
					unsigned int *b= (unsigned int*) malloc (sizeof(unsigned int));
					*b=SIZEA_ARR[i];
					g_hash_table_insert(maxSizeInitData, data_arr[i], b);
				} else {
					*ind += SIZEA_ARR[i];
				}

			}

			g_hash_table_foreach(maxSizeInitData, &setInitData, NULL);




			for (i=0; i<len; i++){

				dataCont_addDataCont(dc, data_arr[i], par_arr[i]);
				log_info(" * %s [%d] -----> %s", par_arr[i], SIZEA_ARR[i], data_arr[i]);
				qdftInit(data_arr[i], SIZEA_ARR[i], par_arr[i]);
			}

		} else if (strcmp(ACTION, "unload_page")==0){
			log_info("\n\n\nunload page event \n\n\n\n\n");
			par_arr=parseStringPar (TARGET, &len);
			for (i=0; i<len; i++){
				qdft_removeFromNode(INF, par_arr[i],-1);
				dataCont_emptyCont(dc,par_arr[i]);
			}

		} else {

		}








/****************************************************************************************
 ****************************************************************************************
 ****************************************************************************************
 ****************************************************************************************
 ****************************************************************************************
 ****************************************************************************************
 ****************************************************************************************
 ****************************************************************************************/














	default:
		break;
	} //switch(syscall)

	if (data_flow_monitor == NULL)
		return 1;

	// 1:STDOUT   2:STDOUT (no empty containers) -1:FILE (empty containers visualized) -2:FILE (empty containers not visualized)
	if ((nParams > 0) && ((!(strcmp(event->action_desc->action_name, "mmap")
			== 0) || (strcmp(FDSRC, "-1") != 0))))
		dataCont_print(dc, 2);

	qdft_print();


	return 0;
}

void IF_reset() {
	log_debug("Initializing IF tracking");
	data_flow_monitor = data_flow_monitor_new();
	data_flow_monitor_init(data_flow_monitor);
	dataCont_print(data_flow_monitor->map, -1);
	log_debug("IF tracking initialized");
}

int IF_update(event_ptr event) {
	data_flow_monitor_update(data_flow_monitor, event);
	return 0;
}


int IF_revert(event_ptr event){
	/* now this implementation is hardcoded for the current policy,
	   but this function should contain a switch case and a different
	   implementation of the revert for each system call (source and dest are different)
	   Therefore the event parameter, here, is used only to retrieve the PID parameter, which
	   was the source container of the action to revert
	   In addition, as of now, it works only to revert a transfer.
	   In future it should also be possible to revert a deletion */

	char* PID=tmpConvertIntParamToString(event, "pid");

	list_of_data_ptr lod = dataCont_getDataIn(data_flow_monitor->map, PID);
	int length = list_of_data_length(lod);
	int dd;
	int q;
	char* current_data;

	for (dd = 0; dd < length; dd++) {
			current_data = lod->data->data_ID;
			if (current_data == NULL) {
				log_error ("revert: null data in list");
				return -1;
			}

			//int datanode = data2int(current_data);

			qdftRevert_last_transfer(current_data);// , PID, "Cnet");
	}
}

/**********
 *
 * given a container and a quantity of data in it, generates a new mapping
 *
 */


char* IF_initCont(char* cont, char* qod) {

	/*
	 * If qod is null
	 */


	char* tmpdata;
	char* tmpcont;

	int quantity = 1;

	if (cont == NULL)
		return NULL;


	if (qod!=NULL){
		//tmpdata = dataCont_getNewDataID(data_flow_monitor->map);

		tmpcont=cont; //TODO:sanitize cont
		quantity = atoi(qod);
		if (quantity==0) quantity=1; //in case qod is not valid, quantity is 1  ---> qdft = tainting tracking

	} else {
		//cont is the filename
		tmpcont = dataCont_getNaming(data_flow_monitor->map, "FNAME", cont);
		if (tmpcont == NULL) tmpcont = dataCont_getNewContID(data_flow_monitor->map);
		dataCont_addNaming(data_flow_monitor->map, "FNAME", cont, tmpcont);


		//qod==NULL  -->  cont is a file name
		//define initial quantity as the file size
		struct stat point;
		stat(cont, &point);
		quantity = (int)point.st_size;
	}

	tmpdata = qdftGet_new_data(quantity, INIT_DATA_MODE);
	if (gl_data2check == 0)
		gl_data2check = tmpdata;


	dataCont_addDataCont(data_flow_monitor->map, tmpdata, tmpcont);

	log_info(" * %s [%d] -----> %s", cont, quantity, tmpdata);

	qdftInit(tmpdata, quantity, tmpcont);

	//maxsize[datanode]=(int) point.st_size;
	log_info(" * qdft entry created ....  %s stores %d units", tmpcont, quantity);

	return tmpdata;
}


bool stateFormula(){
	return TRUE;
}
