
#include "data_flow_monitor.h"
#include <sys/syscall.h>
#include <string.h>

int deny_because_of_IF = 0;



data_flow_monitor_ptr data_flow_monitor_new(){
	data_flow_monitor_ptr dfm =(data_flow_monitor_ptr) malloc (sizeof(data_flow_monitor_t));
	dfm->layer=(char*)calloc(SIZE_LAYER, sizeof(char));
	dfm->map=dataCont_new();
	return dfm;
}



void data_flow_monitor_init(data_flow_monitor_ptr data_flow_monitor){
/*
	char *tmpcont;
	dataCont_newCont(data_flow_monitor->map, "Cnet");
	dataCont_newData(data_flow_monitor->map, "Dnet");
	dataCont_addDataCont(data_flow_monitor->map, "Dnet", "Cnet");
*/
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
	int size,x;

	list_of_data_ptr lod;
	list_of_data_ptr tmplod;
	list_of_cont_ptr loc;

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


	//char *contID;
	param_t tmp;



	if (event != NULL){
		syscall=syscall_to_int(event->event_name);
	}


	deny_because_of_IF = 0;

	if (event->n_params>0){
		HOSTxPID = strdup(event->params[0].param_value);
		FNAME = strdup(event->params[0].param_value);
		ADDR = strdup(event->params[0].param_value);
		strncpy(FNAME+12," FNAME",6);
		strncpy(ADDR+12 ,"  ADDR",6);
	}

	if (event->n_params>2){
		USER=strdup(event->params[2].param_value);
	}

	if (event->n_params>3){
		FN=strdup(event->params[3].param_value);
		FDSRC=FN;

		char pid2[7];
		memset(pid2,0,7);
		if (sscanf(event->params[3].param_value,"[%[^]]s",pid2) == 1) {
			HOSTxPID2 = strdup(HOSTxPID);
			strncpy(HOSTxPID2+12,pid2,6);
		}
	}

	if (event->n_params>4){
		FD=strdup(event->params[4].param_value);
		FLAGS=FD;
		FDDST=FD;
		PROT=FD;
	}
	if (event->n_params>5) {
		RETVAL=strdup(event->params[5].param_value);
	}


	switch (syscall) {


	case SYS_exit: //exit
		/*
		 * 0 - pid
		 * 1 - command
		 *
		 * 2 - user
		 * 3 - status
		 */

		dataCont_emptyCont(dc, HOSTxPID);

		dataCont_delAllAliasesFrom(dc, HOSTxPID);

		dataCont_delAllAliasesTo(dc, HOSTxPID);

		dataCont_delAllNamingFor(dc, HOSTxPID);

		dataCont_removeCont(dc, HOSTxPID);


		//dataCont_print(dc,1);

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
	case 28://sendmsg
	case 121://writev
	case 133://sendto
	case 174://pwrite
	case 201://ftruncate

		/*
		 * 0 - pid
		 * 1 - command
		 * 2 - user
		 * 3 - filename
		 * 4 - file descriptor
		 */

		//MANUAL WAY


		//get f(PID,FD)
		tmpname=dataCont_getNaming(dc, HOSTxPID, FD);


		//get l*(f(PID,FD))
		list=dataCont_getAliasesOf(dc,tmpname,&size);

		//add s(PID) to f(PID,FD)
		lod=dataCont_getDataIn(dc, HOSTxPID);

		if (tmpname!=NULL) {
			dataCont_addLodCont(dc,lod,tmpname);

			tmplod=dataCont_getDataIn(dc, tmpname);


			//add s(PID) to each element in l*(f(PID,FD))
			if (list!=NULL){
				for (loop=0;loop<size;loop++){
					dataCont_addLodCont_noUpdate(dc,lod,list[loop]);
				}
			}

			//AUTOMATIC WAY
			//dataCont_addLodCont(dc,lod,tmpname);
		}

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


		if (event->n_params==6){

			//add naming FNAME x FN -> FN by assumption
			tmpname=dataCont_getNaming(dc, FNAME, FN);

			if (tmpname==NULL){ //if file has no content yet, let's start to monitor it
/*
				tmpname=dataCont_getNewContID(dc);

				tmpname[1]='-';
				tmpname[2]='F';
				tmpname[3]='I';
				tmpname[4]='L';
				tmpname[5]='E';
*/
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

			//dataCont_print(dc,1);

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
		syscall_close(dc, HOSTxPID, FD);
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


		//dataCont_print(dc,1);


		break;


	case SYS_socket: {
			/*
			 * 0 - pid
			 * 1 - command
			 * 2 - user
			 * 3 - domain
			 * 4 - type
			 * 5 - protocol
			 * 6 - return value: socket file descriptor ()
			 */

			char *new_cont_id;

			/* so, let's implement this for sockets of all type */

			/* create a socket container id */
			//new_cont_id=dataCont_getNewContID(dc);
			//strncpy(new_cont_id+1,SOCK_CONTAINER,strlen(SOCK_CONTAINER));
			new_cont_id = dataCont_getNewContIDSocket(dc);

			/* create the new container with ID tmpname
			 * and give it a name using PID x FD
			 */
			dataCont_newCont(dc, new_cont_id);
			dataCont_addNaming(dc, HOSTxPID, event->params[6].param_value, new_cont_id);
		}
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
		syscall_bind(dc, HOSTxPID, event->params[3].param_value,
							event->params[5].param_value,
							event->params[6].param_value,
							ADDR);
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

		// get the socket container: tmpname = f((pid,sfd))
		tmpname = dataCont_getNaming(dc, HOSTxPID, event->params[3].param_value);

		// get all names for that container: names = f^-(tmpname)
		char ***names = dataCont_getAllNamingsOfContainer(dc, tmpname, &size);

		// delete all addresses for that socket, namely all addresses with ADDR
		if (names != NULL) {
			for (x = 0; x < size; x++) {
				if (strcmp(names[x][0],ADDR) == 0) {
					dataCont_delNaming(dc, ADDR, names[x][1]);
				}
			}
		}
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
			syscall_connect(dc, HOSTxPID, event->params[3].param_value,
									event->params[7].param_value,
									event->params[8].param_value,
									ADDR);
			break;


	case SYS_accept: {
			/*
			 * 0 - pid
			 * 1 - command
			 * 2 - user
			 * 3 - socket file descriptor (sfd)
			 * 4 - address family
			 * 5 - ip address
			 * 6 - port
			 * 7 - return value
			 */

			char *ip_addr = event->params[5].param_value;
			char *port = event->params[6].param_value;
			char *new_fd = event->params[7].param_value;

			char *local_cont_id;
			char *remote_cont_id;
			char remote_socket_name[22];

			/* ==== 1st: Get/Create everything necessary ==== */

			// create a new local container id
			//local_cont_id=dataCont_getNewContID(dc);
			//strncpy(local_cont_id+1,SOCK_CONTAINER,strlen(SOCK_CONTAINER));
			local_cont_id=dataCont_getNewContIDSocket(dc);

			// assemble the known name of remote socket: ipaddress:port
			memset(remote_socket_name,0,22);
			snprintf(remote_socket_name,22,"%s:%s",ip_addr,port);

			// get the remote container id
			remote_cont_id = dataCont_getNaming(dc, ADDR, remote_socket_name);




			/* ==== 2nd: Update naming function ==== */

			/* create the new container and assign it a name using PID x FD
			 * f(PID,FD) <- c */
			dataCont_newCont(dc, local_cont_id);
			dataCont_addNaming(dc, HOSTxPID, new_fd, local_cont_id);

			// get all names of the remote socket: names = f^-(remote_cont_id)
			char ***names = dataCont_getAllNamingsOfContainer(dc, remote_cont_id, &size);

			// delete all addresses for that socket, namely all addresses with ADDR
			// x <- O
			if (names != NULL) {
				for (x = 0; x < size; x++) {
					if (strcmp(names[x][0],ADDR) == 0) {
						dataCont_delNaming(dc, ADDR, names[x][1]);
						printf("---delete name %s %s\n",ADDR, names[x][1]);
					}
				}
			}


			/* ==== 3rd: Update alias function ==== */

			dataCont_addAlias(dc, local_cont_id, remote_cont_id);
			dataCont_addAlias(dc, remote_cont_id, local_cont_id);


			printf("---new local cont id: %s\n",local_cont_id);
			printf("---new name: %s %s\n",HOSTxPID, new_fd);
			printf("---remote cont id: %s\n",remote_cont_id);
		}
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

		//dataCont_print(dc,1);


		break;


	case SYS_dup: //dup
	case SYS_dup2: //dup2
	case SYS_fcntl: //fcntl
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

		//dataCont_print(dc,1);

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
		//dataCont_print(dc,1);

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

		//dataCont_print(dc,1);

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


			PROT=strdup(event->params[5].param_value);
			FDSRC=strdup(event->params[7].param_value);
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

			//dataCont_print(dc,1);

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

		//dataCont_print(dc,1);

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


		if (event->n_params==6){
			if (strcmp(RETVAL,"0")==0){
				//create new container for the pipe (if it has been correctly created)
/*
				tmpname=dataCont_getNewContID(dc);

				tmpname[1]='-';
				tmpname[2]='P';
				tmpname[3]='I';
				tmpname[4]='P';
				tmpname[5]='E';
*/
				tmpname = dataCont_getNewContIDPipe(dc);


				/*        printf("\n\PIPE PID=%s FDSRC=%s FDDST=%s \n\n", PID, FDSRC, FDDST);*/
				dataCont_newCont(dc, tmpname);

				//add naming PID x FDSRC -> Cp
				dataCont_addNaming(dc, HOSTxPID, FDSRC, tmpname);

				//add naming PID x FDDST -> Cp
				dataCont_addNaming(dc, HOSTxPID, FDDST, tmpname);

				//dataCont_print(dc,1);
			}
		}


		break;



	default:

		break;
	}




	if (data_flow_monitor==NULL) return 1;

	// 1:STDOUT   2:STDOUT (no empty containers) -1:FILE (empty containers visualized) -2:FILE (empty containers not visualized)
	if ((event->n_params>0)&&((!(strcmp(event->event_name,"mmap")==0)||(strcmp(FDSRC,"-1")!=0)))) dataCont_print(dc, 2);

	return 0;
}



void syscall_close(dataCont_ptr dc, char *PID, char *fd) {
	//del naming PID x FD -> C(PID x FN) by model
	dataCont_delNaming(dc, PID, fd);
}




void syscall_connect(dataCont_ptr dc, char *PID, char *sfd, char *local_ip_addr, char *local_port, char *ADDR) {
	char *cont_name;
	char ***names;
	int size = 0;
	int is_bound = 0;
	int i = 0;

	if (strcmp(local_ip_addr,"unspec") == 0) {
		// No local ip address was assigned, therefore something went wrong. The systemcall fails.
		return;
	}

	// get the socket container: cont_name = f((pid,sfd))
	cont_name = dataCont_getNaming(dc, PID, sfd);

	// get all names for that container: names = f^-(cont_name)
	names = dataCont_getAllNamingsOfContainer(dc, cont_name, &size);

	// find out, whether the socket is already bound to an address
	if (names != NULL) {
		while (i < size && !is_bound) {
			if (strcmp(names[i][0],ADDR) == 0) {
				is_bound = 1;
			}
			i++;
		}
	}

	// if the socket is not bound to an address, we explicitly call bind()
	if (!is_bound) {
		syscall_bind(dc, PID, sfd, local_ip_addr, local_port, ADDR);
	}
}



/**
 * ipaddr is a listed of IP addresses
 */
void syscall_bind(dataCont_ptr dc, char *PID, char *sfd, char *ipaddr, char *port, char *ADDR) {
	// get the socket container: f((pid,sfd))
	char *cont_id = dataCont_getNaming(dc, PID, sfd);

	// read the ip addresses in ipaddr and add a namign for each ipaddr:port

	char new_name[15];
	char *pos = ipaddr;

	while ((pos = strchr(ipaddr,IP_ADDR_SEP)) != NULL) {
		memset(new_name,0,15);
		snprintf(new_name,15,"%.*s:%s\n",8,ipaddr,port);
		dataCont_addNaming(dc, ADDR, new_name, cont_id);
		ipaddr = pos+1;
	}

	// do not forget about the last address
	memset(new_name,0,15);
	snprintf(new_name,15,"%.*s:%s",8,ipaddr,port);
	dataCont_addNaming(dc, ADDR, new_name, cont_id);
}                                                                                                                                           

