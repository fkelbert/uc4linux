#include "data_flow_monitor.h"

int deny_because_of_IF = 0;





void addAddrNaming(dataCont_ptr dc, char *HOSTxPID, char *cont_name, char *cont_id) {
	if (is_GLOBAL_IP(cont_name) == 0) {
		dataCont_addNaming(dc, GLOBAL_ADDR, cont_name, cont_id);
	}
	else {
		char HOSTxADDR[strlen(HOSTxPID)+1];
		dataCont_addNaming(dc, to_ADDR(HOSTxADDR,HOSTxPID), cont_name, cont_id);
	}
}



char *getAddrNaming(dataCont_ptr dc, char *HOSTxPID, char *cont_name) {
	if (is_GLOBAL_IP(cont_name) == 0) {
		return dataCont_getNaming(dc, GLOBAL_ADDR, cont_name);
	}
	else {
		char HOSTxADDR[strlen(HOSTxPID)+1];
		return dataCont_getNaming(dc, to_ADDR(HOSTxADDR,HOSTxPID), cont_name);
	}
}




data_flow_monitor_ptr data_flow_monitor_new(){
	data_flow_monitor_ptr dfm =(data_flow_monitor_ptr) malloc (sizeof(data_flow_monitor_t));
	dfm->layer=(char*)calloc(SIZE_LAYER, sizeof(char));
	dfm->map=dataCont_new();

	data_flow_monitor_service(dfm->map);

	return dfm;
}






void data_flow_monitor_init(__attribute__((unused)) data_flow_monitor_ptr data_flow_monitor){
}


int get_deny_because_of_if(){
	return deny_because_of_IF;
}

void reset_deny_because_of_if(){
	deny_because_of_IF=0;;
}

unsigned int data_flow_monitor_reset(data_flow_monitor_ptr data_flow_monitor){
	unsigned int result=0;
	data_flow_monitor_ptr tmp=data_flow_monitor_new();
	free(data_flow_monitor->layer);
	result=dataCont_free(data_flow_monitor->map);
	data_flow_monitor->layer=tmp->layer;
	data_flow_monitor->map=tmp->map;
	return result;
}



unsigned int data_flow_monitor_update(data_flow_monitor_ptr data_flow_monitor, event_ptr event){
	int syscall=0;
	int loop=0;
	char *tmpname;
	char *tmpname2;
	char** list;
	int size = 0;
	int x;

	int result = 0;

	list_of_data_ptr lod;

	dataCont_ptr dc=data_flow_monitor->map;

	char *HOSTxPID;
	char *HOSTxPID2;

	char* USER="";


	char* FD="";
	char* FN="";
	char* FLAGS="";
	char* RETVAL="";
	char* FDSRC="";
	char* FDDST="";
	char* PROT="";

	char *FNAME;
	char *ADDR;


	if (event != NULL){
		syscall=syscall_to_int(event->event_name);
	}


	deny_because_of_IF = 0;

	if (event->n_params>0){
		HOSTxPID = strdup(event->params[0].param_value);
		ADDR = strdup(event->params[0].param_value);
		FNAME = strdup(event->params[0].param_value);
		strncpy(FNAME+12," FNAME",6);
		strncpy(ADDR+12,"  ADDR",6);
	}

	if (event->n_params>3){
		USER=strdup(event->params[3].param_value);
	}

	if (event->n_params>4){
		FN=strdup(event->params[4].param_value);
		FDSRC=FN;

		char pid2[7];
		memset(pid2,0,7);
		if (sscanf(event->params[4].param_value,"[%[^]]s",pid2) == 1) {
			HOSTxPID2 = strdup(HOSTxPID);
			strncpy(HOSTxPID2+12,pid2,6);
		}
	}


	if (event->n_params>5){
		FD=strdup(event->params[5].param_value);
		FLAGS=FD;
		FDDST=FD;
		PROT=FD;
	}
	if (event->n_params>6) {
		RETVAL=strdup(event->params[6].param_value);
	}


	// =====>>>>> lock semaphore
	sem_wait(&dc->lock);


	switch (syscall) {


	case SYS_exit: //exit
		/*
		 * 0 - pid
		 * 1 - command
		 *
		 * 2 - user
		 * 3 - status
		 */
		dfm_syscall_exit(dc, HOSTxPID);
		break;


	case SYS_fork: //fork
	case SYS_vfork: //vfork
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - retval (pid of the child -> PID2)
		 */
		//get s(PID)
		lod=dataCont_getDataIn(dc,HOSTxPID);

		//add s(PID) to s(PID2)
		dataCont_newCont(dc,HOSTxPID2);
		dataCont_addLodCont_noUpdate(dc, lod, HOSTxPID2);

		//l(PID2) = l(PID)
		//dataCont_addAlias (dc, PID2, PID);

		//get list of containers that are aliases for PID
		list=dataCont_getAliasesTo(dc, HOSTxPID, &size);

		//for each container c | PID in l(c)   => PID2 in l(c)
		if (list!=NULL){
			for (loop=0;loop<size;loop++){
				dataCont_addAlias(dc,list[loop],HOSTxPID2);
			}
		}
		//get list of Aliases (not closure) from PID
		free(list);
		list=dataCont_getAliasesOf_noUpdate(dc,HOSTxPID, &size);

		//add all of them to PID2
		if (list!=NULL){
			for (loop=0;loop<size;loop++){
				dataCont_addAlias(dc,HOSTxPID2,list[loop]);
			}
		}


		list=dataCont_getAllNamingsFrom(dc,HOSTxPID, &size);

		//add all of them to PID2
		if (list!=NULL){
			for (loop=0;loop<size;loop++){
				tmpname=dataCont_getNaming(dc, HOSTxPID, list[loop]);
				dataCont_addNaming(dc,HOSTxPID2,list[loop],tmpname);
			}
		}


/*
		//get s(PID)
		lod=dataCont_getDataIn(dc,PID);


		//add s(PID) to s(PID2)
		dataCont_newCont(dc,PID2);
		dataCont_addLodCont_noUpdate(dc, lod, PID2);

		//l(PID2) = l(PID)
		//dataCont_addAlias (dc, PID2, PID);

		//get list of containers that are aliases for PID
		list=dataCont_getAliasesTo(dc, PID, &size);

		//for each container c | PID in l(c)   => PID2 in l(c)
		if (list!=NULL){
			for (loop=0;loop<size;loop++){
				dataCont_addAlias(dc,list[loop],PID2);
			}
		}
		//get list of Aliases (not closure) from PID
		free(list);
		list=dataCont_getAliasesOf_noUpdate(dc,PID, &size);

		//add all of them to PID2
		if (list!=NULL){
			for (loop=0;loop<size;loop++){
				dataCont_addAlias(dc,PID2,list[loop]);
			}
		}char *type,


		list=dataCont_getAllNamingsFrom(dc,PID, &size);

		//add all of them to PID2
		if (list!=NULL){
			for (loop=0;loop<size;loop++){
				tmpname=dataCont_getNaming(dc, PID, list[loop]);
				dataCont_addNaming(dc,PID2,list[loop],tmpname);
			}
		}

*/
		break;

	case SYS_read: //read
	case 29: //recvfrom
	case 27: //recvmsg
	case 173: //pread
	case 120: //readv
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - filename
		 * 4 - file descriptor
		 */


		//get s(f(PID,FD))
		tmpname=dataCont_getNaming(dc, HOSTxPID, FD);
		lod=dataCont_getDataIn(dc, tmpname);

		//add s(f(pid,fd)) to s(pid)
		dataCont_addLodCont(dc,lod,HOSTxPID);

		//add s(f(pid,fd)) to every c in l(s(pid))
		list=dataCont_getAliasesOf(dc,HOSTxPID,&size);
		if (list!=NULL){
			for (loop=0;loop<size;loop++){
				dataCont_addLodCont(dc,lod,list[loop]);
			}
		}
		break;




	case SYS_write: //write
	case SYS_sendmsg://sendmsg
	case SYS_writev://writev
	case SYS_sendto://sendto
	//case SYS_pwrite://pwrite
	case SYS_ftruncate://ftruncate
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - filename
		 * 4 - file descriptor
		 */
		dfm_syscall_write(dc, HOSTxPID, event->params[5].param_value, event->params[4].param_value);
		break;

	case SYS_open: //open
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - filename
		 * 4 - flags
		 * 5 - ret val
		 *
		 */


		if (event->n_params==7){

			//add naming FNAME x FN -> FN by assumption
			tmpname=dataCont_getNaming(dc, FNAME, FN);

			if (tmpname==NULL){ //if file has no content yet, let's start to monitor it
				tmpname = dataCont_getNewContIDFile(dc);
				dataCont_newCont(dc, tmpname);
				dataCont_addNaming(dc, FNAME, FN, tmpname);
			}


			//if the file is opened with flag O_TRUNC, the old content is lost
			if (FLAGS[5]=='1') {
				dataCont_emptyCont (dc, tmpname);

			}


			//add naming PID x FD -> C(FNAME x FN) by model
			dataCont_addNaming(dc, HOSTxPID, RETVAL, tmpname);

		}
		break;

	case SYS_close: //close
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - filename
		 * 4 - file descriptor
		 *
		 */
		dfm_syscall_close(dc, HOSTxPID, FD, FN);
		break;

	case SYS_unlink: //unlink
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - filename
		 *
		 */

		//get f(PID,FN)
		tmpname=dataCont_getNaming(dc, FNAME, FN);

		if (tmpname!=NULL){
			//s(PID,FN)<-nil
			dataCont_emptyCont(dc, tmpname);

			//l(f(pid,fn)<-nil)
			dataCont_delAllAliasesFrom(dc, tmpname);

			//remove f(pid,fn) from every alias destination
			dataCont_delAllAliasesTo(dc, tmpname);

			dataCont_delNaming(dc, FNAME, FN);
		}

		break;


	case SYS_socket:
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - domain
		 * 4 - type
		 * 5 - protocol
		 * 6 - return value: socket file descriptor ()
		 */
		dfm_syscall_socket(dc, HOSTxPID, event->params[7].param_value);
		break;

	case SYS_bind:
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - socket file descriptor (sfd)
		 * 4 - address family
		 * 5 - list of ip addresses
		 * 6 - port
		 * 7 - return value
		 */
		//dfm_syscall_bind(dc, HOSTxPID, event->params[4].param_value,
		//					event->params[6].param_value,
		//					event->params[7].param_value);
		break;

	case SYS_shutdown:
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - socket file descriptor (sockfd)
		 * 4 - how
		 * 5 - return value
		 */
		dfm_syscall_shutdown(dc, HOSTxPID, event->params[4].param_value,
							event->params[5].param_value);
		break;


	case SYS_listen:
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - socket file descriptor (sockfd)
		 * 4 - backlog
		 * 5 - return value
		 */
		//dfm_syscall_listen(dc, HOSTxPID, event->params[4].param_value);
		break;


	case SYS_connect:
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - socket file descriptor (sfd)
		 * 4 - address family
		 * 5 - remote ip address
		 * 6 - remote port
		 * 7 - local ip address 	(only if response)
		 * 8 - local port 			(only if response)
		 * 9 - return value
		 */
		dfm_syscall_connect(dc, HOSTxPID, event->params[4].param_value,
								event->params[5].param_value,
								event->params[8].param_value,
								event->params[9].param_value,
								event->params[6].param_value,
								event->params[7].param_value);
		break;


	case SYS_accept:
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - socket file descriptor (sfd)
		 * 4 - address family
		 * 5 - local ip address
		 * 6 - local port
		 * 7 - remote ip address
		 * 8 - remote port
		 * 9 - return value
		 */
		dfm_syscall_accept(dc, HOSTxPID,
						event->params[5].param_value,
						event->params[6].param_value,
						event->params[7].param_value,
						event->params[8].param_value,
						event->params[9].param_value,
						event->params[10].param_value);
		break;



	case SYS_kill:
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - target
		 * 4 - signal
		 * 5 - retval
		 *
		 */

		//MANUAL WAY

		//get s(PID)
		lod=dataCont_getDataIn(dc, HOSTxPID);

		//add s(PID) to l*(PID2)
		list=dataCont_getAliasesOf(dc,HOSTxPID2,&size);

		dataCont_addLodCont_noUpdate(dc,lod,HOSTxPID2);
		if (list!=NULL){
			for (loop=0;loop<size;loop++){
				dataCont_addLodCont_noUpdate(dc,lod,list[loop]);
			}
		}
		//AUTOMATIC WAY
		//dataCont_addLodCont(dc,lod,PID2);


		break;

	case SYS_dup2: //dup2
	case SYS_dup: //dup
	case SYS_fcntl: //fcntl

		// TODO! Implementation of dup2 is not correct -> read the manpage!

		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - file descriptor -> FDSRC (old descriptor)
		 * 4 - ret val -> FD (new descriptor)
		 *
		 */

		/*
		 * N.B : fcntl is detected only if the parameter CMD is F_DUPFD. the check is performed in syscall_handler
		 */

		//get f(PID, FDSRC)
		tmpname=dataCont_getNaming(dc, HOSTxPID, FDSRC);

		// f(PID, FD)= f(PID, FDSRC)
		dataCont_addNaming(dc, HOSTxPID, FD, tmpname);

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


		break;




	case SYS_execve: //execve
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - filename
		 *
		 */

		//get f(PID,FN)
		tmpname=dataCont_getNaming(dc, FNAME, FN);

		//add s(f(PID,FN)) to s(PID)
		lod=dataCont_getDataIn(dc, tmpname);
		dataCont_addLodCont_noUpdate(dc,lod,HOSTxPID);

		/*
		 * TO CHECK! I don't buy the semantic of the action at page 21!
		 *
		 */


		break;

	case 73://munmap
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - start
		 * 4 - length
		 * 5 - retval
		 *
		 */



		break;




	case SYS_rename: //rename
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - old file name FDSRC
		 * 4 - new file name FDDST
		 */

		//create new container for the pipe (if it has been correctly created)

		tmpname=dataCont_getNaming(dc, FNAME, FDSRC);
		tmpname2=dataCont_getNaming(dc, FNAME, FDDST);

		//empty destination container (for future usage of the old file descriptor)
		dataCont_emptyCont(dc, tmpname2);


		//swap the naming
		dataCont_addNaming(dc, FNAME, FDSRC, tmpname2);
		dataCont_addNaming(dc, FNAME, FDDST, tmpname);


		//!!!! ADDED. 2011/08/17 Florian Kelbert. Was Wrong without.
		dataCont_delNaming(dc, FNAME, FDSRC);

		break;

	case SYS_mmap: //mmap
		/** Parameters: command user start length prot flags fd offset (+retval)
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - start
		 * 4 - length
		 * 5 - prot       PROT
		 * 6 - flags
		 * 7 - fd         FDSRC
		 * 8 - offset
		 * 9 - retval
		 *
		 */

			PROT=strdup(event->params[6].param_value);
			FDSRC=strdup(event->params[8].param_value);
			if (strcmp(FDSRC,"-1")!=0){

			///get f(p,e)
			tmpname=(dataCont_getNaming(dc, HOSTxPID, FDSRC));

			/*
      if (tmpname==NULL){
        printf ("\n********************************************** f(%s,%s) NOT FOUND ***********************\n\n",PID,FDSRC);
        break;
      }
      //content of f(p,e)
			 */
			lod=dataCont_getDataIn(dc, tmpname);
			//    MANUAL
			//
			//add lod to s(P)
			dataCont_addLodCont_noUpdate(dc, lod, HOSTxPID);

			//add lod to l*(P)
			list=dataCont_getAliasesOf(dc, HOSTxPID, &size);

			for (x=0; x<size; x++){
				dataCont_addLodCont_noUpdate(dc, lod, list[x]);
			}

			//      AUTOMATIC
			//      add lod to P and to l*(P)
			//      dataCont_addLodCont(dc, lod, tmpname);

			if (PROT[14]=='1'){  //READ AND WRITE
				//content of f(p,e)
				lod=dataCont_getDataIn(dc, HOSTxPID);

				//     MANUAL
				//add lod to f(p,e)
				dataCont_addLodCont_noUpdate(dc, lod, tmpname);

				//add lod to l*(f(p,e))
				list=dataCont_getAliasesOf(dc, tmpname, &size);

				for (x=0; x<size; x++){
					dataCont_addLodCont_noUpdate(dc, lod, list[x]);
				}

			}

			dataCont_addAlias(dc, tmpname, HOSTxPID);

			if (PROT[14]=='1'){  //READ AND WRITE
				dataCont_addAlias(dc, HOSTxPID, tmpname);
			}


		}
		break;

	case SYS_truncate://truncate

		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - filename
		 * 4 - file descriptor
		 */


		//get f(PID,FN)
		tmpname=dataCont_getNaming(dc, FNAME, FN);

		//MANUAL WAY
		//get l*(f(PID,FN))
		list=dataCont_getAliasesOf(dc,tmpname,&size);

		//add s(PID) to f(PID,FN)
		lod=dataCont_getDataIn(dc, HOSTxPID);
		dataCont_addLodCont(dc,lod,tmpname);

		//add s(PID) to each element in l*(f(PID,FD))
		if (list!=NULL){
			for (loop=0;loop<size;loop++){
				dataCont_addLodCont_noUpdate(dc,lod,list[loop]);
			}
		}
		//AUTOMATIC WAY
		//dataCont_addLodCont(dc,lod,tmpname);


		break;


	case SYS_pipe: //pipe
		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - file descriptor source
		 * 4 - file descriptor dest
		 * 5 - ret value
		 */


		if (event->n_params==7){
			if (strcmp(RETVAL,"0")==0){
				//create new container for the pipe (if it has been correctly created)
				tmpname = dataCont_getNewContIDPipe(dc);

				dataCont_newCont(dc, tmpname);

				//add naming PID x FDSRC -> Cp
				dataCont_addNaming(dc, HOSTxPID, FDSRC, tmpname);

				//add naming PID x FDDST -> Cp
				dataCont_addNaming(dc, HOSTxPID, FDDST, tmpname);
			}
		}
		break;



	default:
		break;
	}

	if (data_flow_monitor==NULL) {
		result = 1;
	}

	// 1:STDOUT   2:STDOUT (no empty containers) -1:FILE (empty containers visualized) -2:FILE (empty containers not visualized)
	if ((event->n_params>0)&&((!(strcmp(event->event_name,"mmap")==0)||(strcmp(FDSRC,"-1")!=0)))) {
//		dataCont_print(dc, 2);
	}

	// <<<<<===== release semaphore
	sem_post(&dc->lock);

	return result;
}



void dfm_syscall_close(dataCont_ptr dc, char *HOSTxPID, char *fd, char *filename) {
	if (!strcmp(filename,SOCKET_FILENAME)) {
		// if the file descriptor corresponds to a socket,
		// there are possibly other things to do...
		dfm_syscall_close_socket(dc,HOSTxPID,fd);
	}


	//del naming PID x FD -> C(PID x FN) by model
	dataCont_delNaming(dc, HOSTxPID, fd);
}



void dfm_syscall_close_socket(dataCont_ptr dc, char *HOSTxPID, char *fd) {
	/* if this is the last file descriptor for that socket,
	 we need to shutdown the connection */

	// get container
	char *cont_ID = dataCont_getNaming(dc, HOSTxPID, fd);

	int count = 0;
	int names_size = 0;
	int i;


	// count the file descriptor references for that socket
	char ***names = dataCont_getAllNamingsOfContainer(dc, cont_ID, &names_size);

	if (names != NULL) {
		for (i = 0; i < names_size; i++) {
			if (is_ADDR(names[i][0]) != 0) {
				// is _NOT_ an identifier of type ADDR
				count++;
			}
			free(names[i][0]);
			free(names[i][1]);
			free(names[i]);
		}
		free(names);
	}

	// last file descriptor for socket -> shutdown
	if (count == 1) {
		dfm_syscall_shutdown(dc, HOSTxPID, fd, SHUT_RDWR_STR);
	}
}






void dfm_syscall_connect(dataCont_ptr dc, char *HOSTxPID, char *sfd, char *family, char *local_ip, char *local_port, char *remote_ip, char *remote_port) {
	char *local_cont_id;
	char local_socket_name[ADDRxADDR_STR_LEN];

	// handly AF_INET only
	if (strcmp(family,AF_INET_STR)) {
		return;
	}
/*
	if (ignore_ip(local_ip) || ignore_ip(remote_ip)) {
		return;
	}
	*/

	// No local ip address was assigned, therefore something went wrong. The systemcall fails.
	if (strcmp(local_ip,"unspec") == 0) {
		return;
	}

	// local_socket_name := (sn(e),(a,x))
	toADDRxADDR(local_socket_name,ADDRxADDR_STR_LEN,local_ip,local_port,remote_ip,remote_port);

	// c := f((pid,sfd))
	local_cont_id = dataCont_getNaming(dc, HOSTxPID, sfd);

	// f[(p,(sn(e),(a,x))) <- c]
	addAddrNaming(dc, HOSTxPID, local_socket_name, local_cont_id);
}






/* shutdown system call disables reception and/or transmission on a socket */
void dfm_syscall_shutdown(dataCont_ptr dc, char *HOSTxPID, char *sfd, char *how) {
	char *cont_id;
	char remote_ip[IP_LEN_HEX+1];
	int i;
	int size;
	char ***names;

	// get the socket container: f((pid,sfd))
	cont_id = dataCont_getNaming(dc, HOSTxPID, sfd);

	// all names of container
	names = dataCont_getAllNamingsOfContainer(dc, cont_id, &size);

	if (!strcmp(how,SHUT_RD_STR) || !strcmp(how,SHUT_RDWR_STR)) {
		// disallow reception

		// empty storage function
		dataCont_emptyCont(dc,cont_id);

		// delete all aliases to container
		dataCont_delAllAliasesTo(dc, cont_id);
	}

	if (!strcmp(how,SHUT_WR_STR) || !strcmp(how,SHUT_RDWR_STR)) {
		// disallow transmission

		// delete all aliases from this container
		dataCont_delAllAliasesFrom(dc, cont_id);
	}

	if (!strcmp(how,SHUT_RDWR_STR)) {
		// disallow transmission and reception;
		// therefore delete all identifiers of type ADDR

		// delete all identifiers of type ADDR
		if (names != NULL) {
			for (i = 0; i < size; i++) {
				if (is_ADDR(names[i][0]) == 0) {
					dataCont_delNaming(dc, names[i][0], names[i][1]);
				}
			}
		}
	}

	if (names != NULL) {
		for (i = 0; i < size; i++) {
			if (is_ADDR(names[i][0]) == 0) {
				getRemoteIP(remote_ip, names[i][1]);

				if (strcmp(remote_ip,dc->ip)) {
					service_caller_shutdown(remote_ip, names[i][1], how);
				}
			}
			free(names[i][0]);
			free(names[i][1]);
			free(names[i]);
		}
		free(names);
	}
}



void dfm_syscall_exit(dataCont_ptr dc, char *HOSTxPID) {
	int size = 0;
	int i = 0;

	// get all names relative to that process
	char **names = dataCont_getAllNamingsFrom(dc, HOSTxPID, &size);

	if (names != NULL) {
		for (i = 0; i < size; i++) {
			// explicitly call close() for every identifier of that process,
			// in order to ensure that everything gets done (closing sockets etc.)
			dfm_syscall_close(dc, HOSTxPID, names[i], SOCKET_FILENAME);
		}
		free(names);
	}

	dataCont_emptyCont(dc, HOSTxPID);

	dataCont_delAllAliasesFrom(dc, HOSTxPID);

	dataCont_delAllAliasesTo(dc, HOSTxPID);

	dataCont_removeCont(dc, HOSTxPID);
}




void dfm_syscall_socket(dataCont_ptr dc, char *HOSTxPID, char *sfd) {
	// create a socket container
	char *new_cont_id = dataCont_getNewContIDSocket(dc);
	dataCont_newCont(dc, new_cont_id);

	// give it a name using PID x SFD
	dataCont_addNaming(dc, HOSTxPID, sfd, new_cont_id);
}






void dfm_syscall_accept(dataCont_ptr dc, char *HOSTxPID, char *family, char *local_ip, char *local_port, char *remote_ip, char *remote_port, char *new_fd) {
	char *local_cont_id = NULL;
	char *remote_cont_id = NULL;
	char local_socket_name[ADDRxADDR_STR_LEN];
	char remote_socket_name[ADDRxADDR_STR_LEN];
	char HOSTxADDR[strlen(HOSTxPID)+1];

	// handle AF_INET only
	if (strcmp(family,AF_INET_STR)) {
		return;
	}
/*
	if (ignore_ip(local_ip) || ignore_ip(remote_ip)) {
		return;
	}
*/
	to_ADDR(HOSTxADDR,HOSTxPID);

	// local_socket_name := (sn(e),(a,x))
	toADDRxADDR(local_socket_name,ADDRxADDR_STR_LEN,local_ip,local_port,remote_ip,remote_port);

	// remote_socket_name := ((a,x),sn(e))
	toADDRxADDR(remote_socket_name,ADDRxADDR_STR_LEN,remote_ip,remote_port,local_ip,local_port);

	// create the new container c == local_cont_id
	local_cont_id = dataCont_getNewContIDSocket(dc);
	dataCont_newCont(dc, local_cont_id);

	// assign the names

	// f[(p,(sn(e),(a,x))) <- c]
	addAddrNaming(dc, HOSTxPID, local_socket_name, local_cont_id);

	// f[((p,e)) <- c]
	dataCont_addNaming(dc, HOSTxPID, new_fd, local_cont_id);


	if (strcmp(local_ip,remote_ip)) {
		// client is remote

		// get the remote container id from remote host in order to create the alias to it.
		//while (remote_cont_id == NULL) {
		remote_cont_id = service_caller_accept(remote_ip, remote_port, local_ip, local_port, local_cont_id);
		//}

		// add alias from local container to remote container
		dataCont_addAlias(dc, local_cont_id, remote_cont_id);
	}
	else {
		// client is local

		// get the socket container of the client process
		while (remote_cont_id == NULL) {
			remote_cont_id = getAddrNaming(dc, HOSTxADDR, remote_socket_name);

			sem_post(&dc->lock);
			usleep(33);				// wait for corresponding connect()
			sem_wait(&dc->lock);
		}

		// remote socket has been created and correctly named: add aliases
		dataCont_addAlias(dc, local_cont_id, remote_cont_id);
		dataCont_addAlias(dc, remote_cont_id, local_cont_id);
	}

	// add name for remote container
	addAddrNaming(dc, HOSTxPID, remote_socket_name, remote_cont_id);
}




void dfm_syscall_write(dataCont_ptr dc, char *HOSTxPID, char *fd, char *filename) {
	char *cont_id = NULL;
	char **aliases = NULL;
	int nr_of_aliases;
	int i;
	int lod_len;
	list_of_data_ptr lod;
	int *mechanisms_per_data = NULL;
	unsigned char **all_mechanisms = NULL;


	//get f(PID,FD)
	cont_id = dataCont_getNaming(dc, HOSTxPID, fd);
	if (cont_id == NULL) {
		return;
	}

	//add s(PID) to f(PID,FD)
	lod = dataCont_getDataIn(dc, HOSTxPID);
	dataCont_addLodCont(dc, lod, cont_id);

	if (strcmp(filename,SOCKET_FILENAME)) {
		// === not a socket ...

		// get l*(f(PID,FD))
		aliases = dataCont_getAliasesOf(dc, cont_id, &nr_of_aliases);
		if (aliases == NULL) {
			return;
		}


		//add s(PID) to each element in l*(f(PID,FD))
		for (i = 0; i < nr_of_aliases; i++){
			dataCont_addLodCont_noUpdate(dc, lod, aliases[i]);
		}
	}
	else {
		// === is a socket

		// get all aliases l*(f(PID,FD));
		// while there are no aliases, wait for connection to be established
		while (aliases == NULL) {
			aliases = dataCont_getAliasesOf(dc, cont_id, &nr_of_aliases);

			sem_post(&dc->lock);
			usleep(33);				// wait for connection to be established
			sem_wait(&dc->lock);
		}


		lod_len = list_of_data_length(lod);

		if (lod_len > 0) {
			// loop over all aliased containers
			for (i = 0; i < nr_of_aliases; i++){
				// get responsible PIP for aliased container
				char remote_ip[IP_LEN_HEX+1];
				dataCont_getIP(remote_ip,aliases[i]);

				// check, whether the responsible PIP is a remote one
				if (strcmp(remote_ip,dc->ip)) {
					if (!all_mechanisms) {
						// get all mechanisms for all data: do that only once
						mechanisms_per_data = calloc(lod_len, sizeof(int));
						all_mechanisms = get_all_mechanisms(lod, lod_len, mechanisms_per_data);
					}

					// different PIP --> data for sink
					service_caller_dataForSink(remote_ip, lod, lod_len, aliases[i], mechanisms_per_data, all_mechanisms);
				}
				else {
					// local PIP and remote PIP are the same

					//add s(PID) to alias
					dataCont_addLodCont_noUpdate(dc, lod, aliases[i]);
				}
			}

			free(mechanisms_per_data);
			free(all_mechanisms);
		}
	}
}



/**
 * Get all policies for all the data in the specified list of data.
 * The caller must provide the memory for mechanisms_per_data (data_count * sizeof(int)).
 * This method will allocate memory for the return value of type unsigned char**. This memory
 * needs to be freed by the caller.
 *
 * @PARAM		lod						the list of data
 * @PARAM		data_count				the number of elements in lod
 * @IN/OUT		mechanisms_per_data		an array of ints. [0]-> number of mechanisms for data0, [1]-> number of mechanisms for data1, ...
 * @RETURN								a list of mechanisms as char* (XML Format). The returned array mechanisms_per_data tells the caller
 * 										which mechanisms are for which data:
 * 										lod[0]: RETURN[0 .. mechanisms_per_data[0]-1]
 * 										lod[1]: RETURN[mechanisms_per_data[0] .. mechanisms_per_data[1]-1]
 * 										lod[2]: RETURN[mechanisms_per_data[1] .. mechanisms_per_data[2]-1]
 * 										etc.
 */
unsigned char **get_all_mechanisms(list_of_data_ptr lod, int data_count, int *mechanisms_per_data) {
	int i;
	int j;
	int all_mechanisms_count = 0;
	int all_mechanisms_count_new = 0;
	unsigned char **all_mechanisms = NULL;
	unsigned char **data_mechanisms = NULL;
	unsigned char **tmp_all_mechanisms = NULL;
	size_t data_mechanisms_count;
	list_of_data_ptr tmplod;


	// loop over all data elements in the list
	tmplod = lod;
	for (i = 0; i < data_count; i++) {
		// get mechanisms for data
		data_mechanisms = getMechanismsForDataAsXML(tmplod->data->data_ID, &data_mechanisms_count);

		if (data_mechanisms == NULL) {
			mechanisms_per_data[i] = 0;
		}
		else {
			// store the number of mechanisms for that data in the out array
			mechanisms_per_data[i] = data_mechanisms_count;

			// add the mechanisms to the returned array
			all_mechanisms_count_new = all_mechanisms_count + data_mechanisms_count;
			if ((tmp_all_mechanisms = realloc(all_mechanisms, all_mechanisms_count_new * sizeof(char*))) != NULL) {
				all_mechanisms_count = all_mechanisms_count_new;
				all_mechanisms = tmp_all_mechanisms;

				for (j = data_mechanisms_count; j > 0; j--) {
					all_mechanisms[all_mechanisms_count - j] = data_mechanisms[data_mechanisms_count - j];
				}
			}

			free(data_mechanisms);
		}

		tmplod = tmplod->next;
	}

	return all_mechanisms;
}
