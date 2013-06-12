/*
 * syscall_handler.c
 *
 *      Authors: Ricardo Neisse
 *               Enrico Lovat
 *               Florian Kelbert
 */



#include "syscall_handler.h"

#define CFLAGS_LEN 256
#define BUFSIZE 512

static int PRINT_TRACE=1;
static int IGNORE_SYS_FILES=1;

int cfd;


/**
 * Returns the ip address that identifies this host in dest,
 * which is must provide memory for at least 9 chars.
 * ip_addr_list is a list of IP addresses separated by IP_ADDR_SEP.
 * This method chooses the lowest ip addresses in the list, excluding localhost 127.0.0.1.
 * This method returns -1 on failure, which means that no valid address != 127.0.0.1 has been found,
 * 0 on success.
 */
uint32_t get_host_ip(char *dest, char *ip_addr_list) {
	uint32_t res_ip = 0xFFFFFFFF;
	uint32_t tmp_ip = 0;

	char *pos;

	while ((pos = strchr(ip_addr_list,IP_ADDR_SEP)) != NULL) {
		sscanf(ip_addr_list,"%08X,",&tmp_ip);

		if ((tmp_ip < res_ip) && (tmp_ip >> 24 != 0x7F)) {
			res_ip = tmp_ip;
		}

		ip_addr_list = pos+1;
	}

	sscanf(ip_addr_list,"%08X,",&tmp_ip);

	if ((tmp_ip < res_ip) && (tmp_ip >> 24 != 0x7F)) {
		res_ip = tmp_ip;
	}


	if (res_ip == 0xFFFFFFFF || res_ip == 0x7F000001) {
		// failure
		return -1;
	}

	memset(dest,0,9);
	sprintf(dest,"%08X",res_ip);
	return 0;
}



/**
 * Creates the process identifier: IPAddr x PID
 * Returns the result in the provided buffer buf of the specified size.
 * buf should provide memory for at least 20 chars.
 * Returns NULL on failure, the address of buf on success.
 */
char *host_x_pid(char *buf, int size, int pid) {
	char ip_addr_list[2048];
	char host_ip[9];

	if (get_host_ip(host_ip,get_all_ip_addresses(ip_addr_list,2048,0)) == -1) {
		return NULL;
	}

	memset(buf, 0, size);
	snprintf(buf,size,"[%s x %06d]",host_ip,pid);

	return buf;
}



/**
 * Returns a string of all ip addresses in buf of size size. The
 * addresses are separated by IP_ADDR_SEP. The return value is the address of buf.
 * If parameter lo == 1, then the localhost ip address (127.0.0.1 = 7F000001) is included,
 * otherwise it is not.
 */
char *get_all_ip_addresses(char *buf, int size, int lo) {
	struct ifaddrs *ifa_struct=NULL;
	struct ifaddrs *ifa=NULL;

	// get all ip addresses
	getifaddrs(&ifa_struct);

	memset(buf,0,size);
	int i = 0;
	// loop over all ip addresses and assign a name to the container for each address
	for (ifa = ifa_struct; ifa != NULL; ifa = ifa->ifa_next) {

		// only for IPv4 addresses: add address to buffer
		if (ifa->ifa_addr->sa_family == AF_INET) {
			uint32_t addr = ntohl(((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr.s_addr);
			// only include addresses that are note local loopback 127.0.0.1
			if (addr >> 24 != 0x7f) {
				snprintf(buf+i*9,size-i*9,"%08X%c",ntohl(((struct sockaddr_in *)(ifa->ifa_addr))->sin_addr.s_addr),IP_ADDR_SEP);
				i++;
			}
		}
	}

	// include local loopback 127.0.0.1, if requested
	if (lo == 1) {
		snprintf(buf+i*9,size-i*9,"%08X%c",0x7F000001,IP_ADDR_SEP);
		i++;
	}

	buf[i*9-1] = '\0';
	return buf;
}






/**
 * This method returns the ipaddr and the port of the socket identified by
 * the specified process id (pid) and the specified file descriptor (fd).
 *
 * This method forks a child process and makes use of the lsof command to
 * retrieve the ip address and the port of the socket. Only call this method
 * if there is no other possibility to retrieve ip address and port.
 *
 * in: pid
 * in: fd
 * out: ipaddr
 * out: port
 *
 * returns: 0 on success, -1 on failure
 */
int get_local_socket_address(int pid, int fd, char *ipaddr, int size, int *port) {
	int pfd[2];

	// pipe to communicate result of lsof to parent
	if (pipe(pfd) == -1) {
		perror("pipe");
		return -1;
	}

	int childpid = fork();

	if (childpid == -1) {
		perror("fork");
		close(pfd[0]);
		close(pfd[1]);
		return -1;
	}

	// the child will do lsof, while the parent will be waiting for the result

	if (childpid == 0) {
		/* child doing lsof */

		close(pfd[0]);		// close reading end of pipe
		close(1);			// close stdout
		dup(pfd[1]);		// duplicate writing end of pipe to stdout

		// assemble parameters for lsof: -p<PID>, -d<FD>
		char buf_pid[BUFSIZE];
		char buf_fd[BUFSIZE];
		memset(buf_pid, 0, BUFSIZE);
		memset(buf_fd, 0, BUFSIZE);
		snprintf(buf_pid,BUFSIZE,"-p%d",pid);
		snprintf(buf_fd,BUFSIZE,"-d%d",fd);

		// execute lsof -a -p<PID> -d<FD> -Fn
		execl("/usr/local/sbin/lsof","lsof","-a",buf_pid,buf_fd,"-Fn",NULL);

		exit(0);
	}
	else {
		// parent reading from lsof

		close(pfd[1]);		// close writing end of pipe


		// read from pipe to get result of lsof
		int recvd = 0;
		char buf[BUFSIZE];
		memset(buf, 0, BUFSIZE);
		int c = 0;
		while((recvd = read(pfd[0], buf+c, BUFSIZE-c)) > 0) {
			c += recvd;
		}

		if (recvd == -1) {
			close(pfd[0]);
			return -1;
		}

		// now let's find the local address

		char trash[BUFSIZE];
		char s_port[BUFSIZE];
		char s_addr[BUFSIZE];
		memset(trash, 0, BUFSIZE);
		memset(s_port, 0, BUFSIZE);
		memset(s_addr, 0, BUFSIZE);

		// scan the lsof output; the corresponding line starts with 'n' (see manpage of lsof)
		if (sscanf(buf,"%s\nn%[^:]:%[^-\n]",trash,s_addr,s_port) != 3
			|| sscanf(s_port,"%d",port) != 1) {
			close(pfd[0]);
			return -1;
		}

		if (strcmp(s_addr,"localhost") == 0) {
			// either we get the string 'localhost' ...
			strncpy(ipaddr, "7F000001", size);
		}
		else if (strcmp(s_addr,"*") == 0) {
			// .. or we get INADDR_ANY ...
			strncpy(ipaddr, "0", size);
		}
		else {
			unsigned int a1,a2,a3,a4;
			if (sscanf(s_addr,"%d.%d.%d.%d",&a1,&a2,&a3,&a4) == 4) {
				// ... or we get an ip address ...
				snprintf(ipaddr, size, "%X", (a1<<24)+(a2<<16)+(a3<<8)+a4);
			}
			else {
				// ... or we get crap
				close(pfd[0]);
				return -1;
			}
		}

		close(pfd[0]);
	}

	return 0;
}



/**
 * This method returns the current working directory (cwd) of the process with
 * process id pid.
 *
 * This method forks a child process and makes use of the lsof command to
 * retrieve the cwd. cwd is returned in the provided buffer cwdfo size size.
 *
 * in: pid
 * out: cwd
 * in: size
 * out: port
 *
 * returns: 0 on success, -1 on failure
 */
int get_cwd(int pid, char *cwd, int size) {
	int pfd[2];

	// pipe to communicate result of lsof to parent
	if (pipe(pfd) == -1) {
		perror("pipe");
		return -1;
	}

	int childpid = fork();

	if (childpid == -1) {
		perror("fork");
		return -1;
	}

	// the child will do lsof, while the parent will be waiting for the result

	if (childpid == 0) {
		/* child doing lsof */

		close(pfd[0]);		// close reading end of pipe
		close(1);			// close stdout
		dup(pfd[1]);		// duplicate writing end of pipe to stdout

		// assemble parameters for lsof: -p<PID>, -d<FD>
		char buf_pid[BUFSIZE];
		memset(buf_pid, 0, BUFSIZE);
		snprintf(buf_pid,BUFSIZE,"-p%d",pid);

		// execute lsof -a -p<PID> -d'cwd' -Fn
		if (execl("/usr/local/sbin/lsof","lsof","-a",buf_pid,"-d","cwd","-Fn",NULL) == -1) {
			perror("execl");
		}

		close(pfd[1]);
		exit(0);
	}
	else {
		// parent reading from lsof

		close(pfd[1]);		// close writing end of pipe


		// read from pipe to get result of lsof
		int recvd = 0;
		char buf[BUFSIZE];
		memset(buf, 0, BUFSIZE);
		int c = 0;
		while((recvd = read(pfd[0], buf+c, BUFSIZE-c)) > 0) {
			c += recvd;
		}

		if (recvd == -1) {
			close(pfd[0]);
			return -1;
		}

		// now let's find the cwd

		char trash[BUFSIZE];
		memset(trash, 0, BUFSIZE);
		memset(cwd, 0, size);

		int read_trash = 0;
		int read_cwd = 0;

		// scan the lsof output; the corresponding line starts with 'n' (see manpage of lsof)
		if (sscanf(buf,"%s\n%nn%[^\n]\n",trash,&read_trash,cwd,&read_cwd) != 2) {
			close(pfd[0]);
			return -1;
		}

		if (read_cwd-read_trash >= size) {
			cwd[size-1] = '\0';
		}

		close(pfd[0]);
	}

	return 0;
}




/**
 * Returns the absolute path of the file with relative filename rel_filename in
 * the memory provided by abs_filename of size size.
 * If rel_filename starts with a '/', it is copied to abs_filename as-is.
 *
 * rel_filename is converted to an absolute filename relative to the current
 * working directory of the process with process id pid.
 *
 * out: abs_filename
 * in: size
 * in: rel_filename
 * in: pid
 *
 * returns: 0 on success, -1 on failure
 */
int get_absolute_filename(char *abs_filename, int size, char *rel_filename, int pid) {
	char path[2 * PATH_MAX];

	memset(abs_filename,0,size);
	memset(path,0,2 * PATH_MAX);

	// check, whether rel_filename is absolute
	if (rel_filename[0] == '/') {
		strncpy(abs_filename,rel_filename,size);
		return 0;
	}


	// get current working directory, add / and copy relative filename
	get_cwd(pid,path,PATH_MAX);
	strncpy(path+strlen(path),"/",1);
	strncpy(path+strlen(path),rel_filename, 2 * PATH_MAX - (strlen(path)+1));

	// finally, resolve to realpath (see man realpath)
	if (realpath(path, abs_filename) == NULL) {
		return -1;
	}

	return 0;
}





/*
list_seqnr_ptr list;

//data_flow_monitor_ptr data_flow_monitor;

int list_seqnr_push(u_int16_t seqnr){
	list_seqnr_ptr tmpptr;
	if (list==NULL){
		list=(list_seqnr_ptr) malloc (sizeof(list_seqnr_t));
		list->next=NULL;
		list->seqnr=seqnr;
		return 0;
	}

	tmpptr=list;
	while (tmpptr->next!=NULL){
		if (tmpptr->seqnr==seqnr){
			return 1; //error, seqnr already present
		}
		tmpptr=tmpptr->next;
	}

	//last element in list
	if (tmpptr->seqnr==seqnr){
		return 1; //error, seqnr already present
	}

	tmpptr->next=(list_seqnr_ptr) malloc (sizeof(list_seqnr_t));
	tmpptr->next->next=NULL;
	tmpptr->next->seqnr=seqnr;
	return 0;

}

int list_seqnr_pop(u_int16_t seqnr){
	list_seqnr_ptr tmpptr, tmpprev;

	if (list==NULL){
		return 1;
	}
	tmpptr=list;

	if (tmpptr->seqnr==seqnr){
		list=list->next;
		return 0; //first element
	}

	while (tmpptr!=NULL){
		if (tmpptr->seqnr==seqnr){
			tmmprev->next=tmpptr->next; //skip node
			free (tmpptr);
			return 0; //error, seqnr already present
		}
		tmpprev=tmmptr;
		tmpptr=tmpptr->next;
	}

	return 1; //error, element not found already present

}

int list_seqnr_length(){

}
*/



const char *byte_to_binary (int x){
	static char b[17];
	int i=0;
	b[0] = '\0';

	int z;
	for (z = 32768; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
}



/**
 * Converts a domain (AF_INET, AF_INET6, AF_UNIX,...) to a string and returns this string
 * in buf. buf must provide memory for at least 10 chars.
 *
 * Returns the address of buf.
 */
char* domain_to_str(char *buf, int buf_size, int domain) {
	memset(buf,0,buf_size);

	switch(domain) {
		case AF_INET:
			snprintf(buf, buf_size, "%s", "AF_INET");
			break;
		case AF_INET6:
			snprintf(buf, buf_size, "%s", "AF_INET6");
			break;
		case AF_UNIX:
			snprintf(buf, buf_size, "%s", "AF_UNIX");
			break;
		default:
			snprintf(buf, buf_size, "%s", "AF_UNSPEC");
			break;
	}

	return buf;
}


char* type_to_str(char *buf, int buf_size, int type) {
	memset(buf,0,buf_size);

	switch(type) {
		case SOCK_STREAM:
			snprintf(buf, buf_size, "%s", "SOCK_STREAM");
			break;
		case SOCK_DGRAM:
			snprintf(buf, buf_size, "%s", "SOCK_DGRAM");
			break;
		case SOCK_SEQPACKET:
			snprintf(buf, buf_size, "%s", "SOCK_SEQPACKET");
			break;
		case SOCK_RAW:
			snprintf(buf, buf_size, "%s", "SOCK_RAW");
			break;
		case SOCK_RDM:
			snprintf(buf, buf_size, "%s", "SOCK_RDM");
			break;
		default:
			snprintf(buf, buf_size, "%s", "UNKNOWN");
			break;
	}

	return buf;
}











// XML-RPC functions
void event_handler (SoupServer *server, SoupMessage *msg, const char *path, GHashTable *query, SoupClientContext *context, gpointer data) {
	char *method_name;
	GValueArray *params;

	plog (LOG_INFO, "Syscall handler service");

	if (!msg_post(msg)) return;

	soup_message_set_status (msg, SOUP_STATUS_OK);

	if (!parse_method_call(msg, &method_name, &params)) return;

	if (!strcmp (method_name, "attach_process")) {
		plog (LOG_INFO, "Invoking method attach_process()");
		do_attach_process (msg, params);
	} else {
		plog (LOG_TRACE, "Method event_handler.%s() not implemented", method_name);
		soup_xmlrpc_set_fault (msg, SOUP_XMLRPC_FAULT_SERVER_ERROR_REQUESTED_METHOD_NOT_FOUND, "Unknown method %s", method_name);
	}
	g_free (method_name);
	g_value_array_free (params);
}

/**
 * Receives an array char* as  parameter
 * Each char* in the array is the name of one property to be attested
 */
int do_attach_process (SoupMessage *msg, GValueArray *params) {
	int i;
	int pid;
	const char *val;
	char *in, *out;
	if (!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &in)) {
		type_error (msg, G_TYPE_STRING, params, 0);
		return;
	}
	pid = atoi(in);
	plog(LOG_INFO, "Attaching process [%d]",pid);
	if(attach_process(cfd,&pid)) {
		out = "fail";
	} else {
		if(install_policy(cfd,pid)) {
			out = "fail";
		} else {
			out = "success";
		}
	}
	soup_xmlrpc_set_response (msg, G_TYPE_STRING, out);
}

GMainLoop *gloop;

void *run_gloop(void *arg) {
	plog(LOG_TRACE, "Starting gloop thread");
	g_main_loop_run (gloop);
}

// Systrace functions
void init_systrace (int *cfd) {
	int fd;

	if ((fd = open("/dev/systrace", O_RDONLY)) == -1)
		err(1, "/dev/systrace");

	/* Get a systrace descriptor. */
	if (ioctl(fd, STRIOCCLONE, cfd) == -1)
		err(1, "STRIOCCLONE");
	close(fd);
}

int attach_process(int cfd, int *pid)
{
	if (ioctl(cfd, STRIOCATTACH, pid) == -1)
		return -1;
	else
		return 0;
}

int ask_syscalls(int cfd, int syscall_code, struct systrace_policy strpol) {
	strpol.strp_op = SYSTR_POLICY_MODIFY;
	strpol.strp_code = syscall_code;
	strpol.strp_policy = SYSTR_POLICY_ASK;
	if (ioctl(cfd, STRIOCPOLICY, &strpol) == -1)
		return -1;
	else
		return 0;
}

int install_policy(int cfd, int pid) {
	struct systrace_policy strpol;
	int i;

	// Install policy that allows all system calls
	strpol.strp_op = SYSTR_POLICY_NEW;
	strpol.strp_maxents = SYS_MAXSYSCALL;
	if (ioctl(cfd, STRIOCPOLICY, &strpol) == -1)
		return -1;

	strpol.strp_op = SYSTR_POLICY_ASSIGN;
	strpol.strp_pid = pid;
	if (ioctl(cfd, STRIOCPOLICY, &strpol) == -1)
		return -1;

	// Permit all system calls
	for (i = 0; i < SYS_MAXSYSCALL; i++) {
		strpol.strp_op = SYSTR_POLICY_MODIFY;
		strpol.strp_code = i;
		strpol.strp_policy = SYSTR_POLICY_PERMIT;
		if (ioctl(cfd, STRIOCPOLICY, &strpol) == -1)
			return -1;
	}

	// Register to all system calls
	for (i = 0; i < SYS_MAXSYSCALL; i++) {
		ask_syscalls(cfd,i,strpol);
	}
	/*
if (ask_syscalls(cfd,SYS_open,strpol))
	return -1;
if (ask_syscalls(cfd,SYS_read,strpol))
	return -1;
if (ask_syscalls(cfd,SYS_write,strpol))
	return -1;
if (ask_syscalls(cfd,SYS_close,strpol))
	return -1;
if (ask_syscalls(cfd,SYS_unlink,strpol))
	return -1;
	 */
	plog(LOG_DEBUG,"Policy installed");
	return 0;
}

int may_ignore(char *filename, int fd) {
	//	if ((fd>=0 && fd <3) || filename==NULL) {
/*
	if (filename==NULL) {
		return TRUE;
	}
*/

	if 	((filename!=NULL) && (
			strcmp(filename,".")==0 ||
			strcmp(filename,"..")==0 ||
			strcmp(filename,"/dev/tty")==0 ||
			strcmp(filename,"/etc/pwd.db")==0 ||
			strcmp(filename,"/etc/group")==0 ||
			strcmp(filename,"/etc/spwd.db")==0 ||
			strcmp(filename,"/etc/pwd.db")==0 ||
			strcmp(filename,"/etc/localtime")==0 ||
			substr(filename,"/home/neisse/.bash_rc") ||
			substr(filename,".bash_profile") ||
			substr(filename,".bash_login") ||
			substr(filename,".bash_logout") ||
			substr(filename,".bash_history") ||
			substr(filename,"/.Xauthority") ||
			substr(filename,".profile") ||
			substr(filename,"/etc/profile") ||
			substr(filename,"terminfo") ||
			substr(filename,"/usr/local/lib/") ||
			substr(filename,"/usr/share/") ||
			substr(filename,"/var/tmp/") ||
//			substr(filename,"/tmp") ||
			substr(filename,"/var/run/") ||
			substr(filename,"/usr/lib/")
//			substr(filename,"/.") //no hidden files/folders monitored, but relatives paths like ../../ are not allowed...
	)){
		plog(LOG_DEBUG,"Ignoring file access [%s]", filename);
		return TRUE;
	}

	return FALSE;
}

event_t *handle_system_call(int cfd, struct str_message* sys_msg) {
	void *p;
	void *p2;

	char filename[PATH_MAX];
	char *p_filename;
	char filename2[PATH_MAX];
	char *p_filename2;
	int fd2;
	char cflags[CFLAGS_LEN];
	mechanism_actions_t *response;
	int i, fd, x;
	int arrfd[2];
	char **list;
	int flags;
	int c_param;
	int* pointertoint;
	struct passwd *user_info;
	event_t *event=NULL;
	char *command;
	int handle=0;


	switch (sys_msg->msg_data.msg_ask.code) {

		case SYS_exit:
			user_info = getUserInfo(sys_msg->msg_pid);

			event = (event_t *)malloc(sizeof(event_t));
			event->event_name = strdup("exit");
			event->mode = M_EALL;

			// Event parameters: command, user, status   (+retval)

			if (sys_msg->msg_type==SYSTR_MSG_RES){
				event->n_params = 5;
			}else {
				event->n_params = 4;
			}


			c_param = 0;
			event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

			// 0: PID
			event->params[c_param].param_name = strdup("host x pid");
			event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

			// Command
			c_param++;
			sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
			command = strdup(cflags);
			event->params[c_param].param_name = strdup("command");
			if (command) {
				event->params[c_param].param_value = command;
			} else {
				event->params[c_param].param_value = strdup("< >");
			}



			// User
			c_param++;
			event->params[c_param].param_name = strdup("user");
			event->params[c_param].param_value = strdup(user_info->pw_name);
			// status
			c_param++;
			switch (sys_msg->msg_data.msg_ask.code) {
				case SYS_exit:
					event->params[c_param].param_name = strdup("status");
					sprintf(cflags, "%d", sys_msg->msg_data.msg_ask.args[0]);
					break;
			}
			event->params[c_param].param_value = strdup(cflags);

			//retvalue
			if (sys_msg->msg_type==SYSTR_MSG_RES){
				c_param++;
				sprintf(cflags, "%d", sys_msg->msg_data.msg_ask.args[0]);
				event->params[c_param].param_name = strdup("retval");
				event->params[c_param].param_value = strdup(cflags);
			}
			break;


		case SYS_pipe:

			memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );
			copy_io( cfd,sys_msg->msg_pid, SYSTR_READ, p, &arrfd[0], sizeof(int) );
			copy_io( cfd,sys_msg->msg_pid, SYSTR_READ, p+sizeof(int), &arrfd[1], sizeof(int) );

			pointertoint= (int*) sys_msg->msg_data.msg_ask.args[0];

			fd  =  arrfd[0];
			fd2 =  arrfd[1];

			// If the filename was open before while the monitor
			// is running we know how to translate the file descriptor
			// to the filename
			p_filename = get_file_name(sys_msg->msg_pid, fd);
			p_filename2 = get_file_name(sys_msg->msg_pid, fd2);



			user_info = getUserInfo(sys_msg->msg_pid);

			event = (event_t *)malloc(sizeof(event_t));
			event->event_name = strdup("pipe");
			event->mode = M_EALL;

			// Event parameters: command, user, fdsource, fddests  (+return value)

			if (sys_msg->msg_type==SYSTR_MSG_RES){
				event->n_params = 6;
			} else {
				event->n_params = 5;
			}

			c_param = 0;
			event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

			// 0: PID
			event->params[c_param].param_name = strdup("host x pid");
			event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

			c_param++;
			sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
			command = strdup(cflags);
			event->params[c_param].param_name = strdup("command");
			if (command) {
				event->params[c_param].param_value = command;
			} else {
				event->params[c_param].param_value = strdup("< >");
			}



			// User
			c_param++;
			event->params[c_param].param_name = strdup("user");
			event->params[c_param].param_value = strdup(user_info->pw_name);

			// File descriptor source
			c_param++;
			event->params[c_param].param_name = strdup("source");
/*
			if (p_filename) {
				event->params[c_param].param_value = strdup(p_filename);
			} else {
*/
				sprintf(cflags,"%d",fd);
				//				event->params[c_param].param_value = strdup((fd==2)?"stderr":((fd==1)?"stdout":(fd==0)?"stdin":cflags));
				event->params[c_param].param_value = strdup(cflags);
/*			}*/

			// File descriptor dest
			c_param++;
			event->params[c_param].param_name = strdup("dest");
/*
			if (p_filename2) {
				event->params[c_param].param_value = strdup(p_filename2);
			} else {
*/
				sprintf(cflags,"%d",fd2);
				//				event->params[c_param].param_value = strdup((fd2==2)?"stderr":((fd2==1)?"stdout":(fd2==0)?"stdin":cflags));
				event->params[c_param].param_value = strdup(cflags);

/*			}*/


			if (sys_msg->msg_type==SYSTR_MSG_RES){
				c_param++;
				event->params[c_param].param_name = strdup("retval");
				sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
				event->params[c_param].param_value = strdup(cflags);
			}

			// plog(LOG_INFO,"%s [%d] from %s to %s \n", syscallnames[sys_msg->msg_data.msg_ask.code], sys_msg->msg_data.msg_ask.code, event->params[2].param_value,event->params[3].param_value);

			break;



		case SYS_open:
			//     case SYS_creat:
			// p is void pointer
			memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );

			char rel_filename[PATH_MAX];

			// get filename according to system call
			get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &rel_filename[0] );

			// make absolute filename out of it
			get_absolute_filename(filename, PATH_MAX, rel_filename, sys_msg->msg_pid);

			if ((IGNORE_SYS_FILES)&&(may_ignore(&filename[0],fd))) {
				event = (event_t *)malloc(sizeof(event_t));
				event->event_name = strdup(syscallnames[sys_msg->msg_data.msg_ask.code]);
				event->mode = M_EALL;
				// Parameters: filename
				event->n_params = 0;
				event->params = NULL;


			} else {
				user_info = getUserInfo(sys_msg->msg_pid);

				event = (event_t *)malloc(sizeof(event_t));
				event->event_name = strdup("open");
				event->mode = M_EALL;

				// Event parameters: command, user, filename, and flags  (+return value)

				if (sys_msg->msg_type==SYSTR_MSG_RES){
					event->n_params = 6;
				} else {
					event->n_params = 5;
				}

				c_param = 0;
				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

				// 0: PID
				event->params[c_param].param_name = strdup("host x pid");
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));


				c_param++;
				sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
				command = strdup(cflags);
				event->params[c_param].param_name = strdup("command");
				if (command) {
					event->params[c_param].param_value = command;
				} else {
					event->params[c_param].param_value = strdup("< >");
				}
				// User
				c_param++;
				event->params[c_param].param_name = strdup("user");
				event->params[c_param].param_value = strdup(user_info->pw_name);


				// Filename
				c_param++;
				event->params[c_param].param_name = strdup("filename");
				event->params[c_param].param_value = strdup(filename);


				// Flags
				c_param++;
				//      event->params[c_param].param_name = strdup((sys_msg->msg_data.msg_ask.code==SYS_open)?"flags":"mode");
				event->params[c_param].param_name = strdup("flags");
				event->params[c_param].param_value = strdup(byte_to_binary(sys_msg->msg_data.msg_ask.args[1]));

				if (sys_msg->msg_type==SYSTR_MSG_RES){
					c_param++;
					event->params[c_param].param_name = strdup("retval");
					sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
					event->params[c_param].param_value = strdup(cflags);
				}
				//plog(LOG_INFO," %s [%s] -----------------------------------------------------------O_TRUNC=%d cflags=(%s)%d\n", syscallnames[sys_msg->msg_data.msg_ask.code],	filename,O_TRUNC, byte_to_binary(sys_msg->msg_data.msg_ask.args[1]),sys_msg->msg_data.msg_ask.args[1]);
			}
			break;

		case SYS_recvfrom:
		case SYS_recvmsg:
		case SYS_readv:
		case SYS_pread:
		case SYS_read:
			fd = sys_msg->msg_data.msg_ask.args[0];
			// is running we know how to translate the file descriptor
			// If the filename was open before while the monitor
			// to the filename
			p_filename = get_file_name(sys_msg->msg_pid, fd);

			if ((IGNORE_SYS_FILES)&&(may_ignore(p_filename,fd))) {
				event = (event_t *)malloc(sizeof(event_t));
				event->event_name = strdup(syscallnames[sys_msg->msg_data.msg_ask.code]);
				event->mode = M_EALL;
				// Parameters: filename
				event->n_params = 0;
				event->params = NULL;
			} else {
				user_info = getUserInfo(sys_msg->msg_pid);

				// Generate event
				event = (event_t *)malloc(sizeof(event_t));
				switch (sys_msg->msg_data.msg_ask.code){
				case SYS_read:event->event_name = strdup("read");break;
				case SYS_readv:event->event_name = strdup("readv");break;
				case SYS_pread:event->event_name = strdup("pread");break;
				case SYS_recvfrom:event->event_name = strdup("recvfrom");break;
				case SYS_recvmsg:event->event_name = strdup("recvmsg");break;

				}
				event->mode = M_EALL;
				// Parameters: command user filename fd (+retval) [+bu dest addr]
				//retvalue
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					event->n_params = 7;
				}else {
					event->n_params = 6;
				}

				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup("host x pid");
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				c_param++;
				sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
				command = strdup(cflags);
				event->params[c_param].param_name = strdup("command");
				if (command) {
					event->params[c_param].param_value = command;
				} else {
					event->params[c_param].param_value = strdup("< >");
				}



				// User
				c_param++;
				event->params[c_param].param_name = strdup("user");
				event->params[c_param].param_value = strdup(user_info->pw_name);

				// Filename
				c_param++;
				event->params[c_param].param_name = strdup("filename");
				if (p_filename) {
					event->params[c_param].param_value = strdup(p_filename);
				} else {
					event->params[c_param].param_value = strdup((fd==2)?"stderr":((fd==1)?"stdout":(fd==0)?"stdin":"unknown"));
				}

				// FileDescriptor
				c_param++;
				event->params[c_param].param_name = strdup("filedescriptor");
				sprintf(cflags,"%d",fd);
				event->params[c_param].param_value = strdup(cflags);

				//retvalue
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					c_param++;
					event->params[c_param].param_name = strdup("retval");
					sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
					event->params[c_param].param_value = strdup(cflags);
				}

				//buf dest addr
				c_param++;
				event->params[c_param].param_name = strdup("buf");
				//          sprintf(cflags,"0x%x(%s)",sys_msg->msg_data.msg_ask.args[1],byte_to_binary(sys_msg->msg_data.msg_ask.args[1]));
				sprintf(cflags,"%x",sys_msg->msg_data.msg_ask.args[1]);
				event->params[c_param].param_value = strdup(cflags);


			}
			//plog(LOG_DEBUG," %s [%s] (%d) \n", syscallnames[sys_msg->msg_data.msg_ask.code], event->params[c_param].param_value, fd);
			break;

		case SYS_write:
		case SYS_pwrite:
		case SYS_writev:
		case SYS_truncate:
		case SYS_ftruncate:
		case SYS_sendto:
		case SYS_sendmsg:

			if (sys_msg->msg_data.msg_ask.code==SYS_truncate){
				memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );

				char rel_filename[PATH_MAX];

				// get filename according to system call
				get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &rel_filename[0] );

				// make absolute filename out of it
				get_absolute_filename(filename, PATH_MAX, rel_filename, sys_msg->msg_pid);

				fd=get_fd(sys_msg->msg_pid, filename);
				p_filename=strdup(filename);
			}else {
				fd = sys_msg->msg_data.msg_ask.args[0];
				p_filename = get_file_name(sys_msg->msg_pid, fd);
			}


			if ((IGNORE_SYS_FILES)&&(may_ignore(p_filename,fd))) {
				event = (event_t *)malloc(sizeof(event_t));
				sprintf (cflags, "%s(fd=%d)",syscallnames[sys_msg->msg_data.msg_ask.code],fd);
				event->event_name = strdup(cflags);
				event->mode = M_EALL;
				// Parameters: filename
				event->n_params = 0;
				event->params = NULL;


			} else {


				user_info = getUserInfo(sys_msg->msg_pid);
				// Generate event
				event = (event_t *)malloc(sizeof(event_t));


				switch (sys_msg->msg_data.msg_ask.code){
				case SYS_write:event->event_name = strdup("write");break;
				case SYS_writev:event->event_name = strdup("writev");break;
				case SYS_pwrite:event->event_name = strdup("pwrite");break;
				case SYS_truncate:event->event_name = strdup("truncate");break;
				case SYS_ftruncate:event->event_name = strdup("ftruncate");break;
				case SYS_sendto:event->event_name = strdup("sendto");break;
				case SYS_sendmsg:event->event_name = strdup("sendmsg");break;
				}

				event->mode = M_EALL;
				// Parameters: command user filename filedescriptor  (+retval)

				//retvalue
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					event->n_params = 6;
				} else {
					event->n_params = 5;
				}
				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup("host x pid");
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				c_param++;		sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
				command = strdup(cflags);
				event->params[c_param].param_name = strdup("command");
				if (command) {
					event->params[c_param].param_value = command;
				} else {
					event->params[c_param].param_value = strdup("< >");
				}



				// User
				c_param++;
				event->params[c_param].param_name = strdup("user");
				event->params[c_param].param_value = strdup(user_info->pw_name);
				// Filename
				c_param++;
				event->params[c_param].param_name = strdup("filename");
				if (p_filename) {
					event->params[c_param].param_value = strdup(p_filename);
				} else {
					event->params[c_param].param_value = strdup((fd==2)?"stderr":((fd==1)?"stdout":(fd==0)?"stdin":"unknown"));
				}

				// FileDescriptor
				c_param++;
				event->params[c_param].param_name = strdup("filedescriptor");
				sprintf(cflags,"%d",fd);

				if(p_filename)
				{
					event->params[c_param].param_value = strdup(cflags);
				} else {
					//					event->params[c_param].param_value = strdup((fd==2)?"stderr":((fd==1)?"stdout":(fd==0)?"stdin":"unknown"));
					event->params[c_param].param_value = strdup(cflags);
				}


				//retvalue
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					c_param++;
					event->params[c_param].param_name = strdup("retvalue");
					sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
					event->params[c_param].param_value = strdup(cflags);
				}

			}
			//plog(LOG_DEBUG," %s [%s] (%d) \n", syscallnames[sys_msg->msg_data.msg_ask.code], event->params[c_param].param_value, fd);
			break;

		case SYS_close:
			fd = sys_msg->msg_data.msg_ask.args[0];
			p_filename = get_file_name(sys_msg->msg_pid,fd);


			if ((IGNORE_SYS_FILES)&&(may_ignore(p_filename,fd))) {
				event = (event_t *)malloc(sizeof(event_t));
				event->event_name = strdup(syscallnames[sys_msg->msg_data.msg_ask.code]);
				event->mode = M_EALL;
				// Parameters: filename
				event->n_params = 0;
				event->params = NULL;


			} else {

				user_info = getUserInfo(sys_msg->msg_pid);

				event = (event_t *)malloc(sizeof(event_t));
				event->event_name = strdup("close");
				event->mode = M_EALL;
				// Parameter: command user filename filedescriptor
				//retvalue
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					event->n_params = 6;
				} else {
					event->n_params = 5;
				}


				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup("host x pid");
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				c_param++;
				sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
				command = strdup(cflags);
				event->params[c_param].param_name = strdup("command");
				if (command) {
					event->params[c_param].param_value = command;
				} else {
					event->params[c_param].param_value = strdup("< >");
				}



				// User
				c_param++;
				event->params[c_param].param_name = strdup("user");
				event->params[c_param].param_value = strdup(user_info->pw_name);
				// Filename
				c_param++;
				event->params[c_param].param_name = strdup("filename");
				if(p_filename){
					event->params[c_param].param_value = strdup(p_filename);
				} else {
					event->params[c_param].param_value = strdup((fd==2)?"stderr":((fd==1)?"stdout":(fd==0)?"stdin":"unknown"));
				}

				// FileDescriptor
				c_param++;
				event->params[c_param].param_name = strdup("filedescriptor");
				//					if(p_filename)
				//					{
				sprintf(cflags,"%d",fd);
				event->params[c_param].param_value = strdup(cflags);
				//					} else {
				//						event->params[c_param].param_value = strdup((fd==2)?"stderr":((fd==1)?"stdout":(fd==0)?"stdin":"unknown"));

				//					}



				//retvalue
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					c_param++;
					event->params[c_param].param_name = strdup("retval");
					sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
					event->params[c_param].param_value = strdup(cflags);
				}

				//					if (may_ignore(filename, sys_msg->msg_data.msg_ask.rval[0]) || filename==NULL) break;
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					delete_fd_filename_mapping(sys_msg->msg_pid, sys_msg->msg_data.msg_ask.args[0]);
				}
				//plog(LOG_INFO," %s [%s] (%d) \n", syscallnames[sys_msg->msg_data.msg_ask.code], event->params[c_param].param_value, fd);
			}
			break;


		case SYS_wait4:

			user_info = getUserInfo(sys_msg->msg_pid);

			event = (event_t *)malloc(sizeof(event_t));
			event->event_name = strdup("wait4");
			event->mode = M_EALL;
			// Parameter: command user child_pid status option
			//retvalue
			if (sys_msg->msg_type==SYSTR_MSG_RES){
				event->n_params = 7;
			} else {
				event->n_params = 6;
			}


			event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
			c_param = 0;

			// 0: PID
			event->params[c_param].param_name = strdup("host x pid");
			event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

			c_param++;
			sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
			command = strdup(cflags);
			event->params[c_param].param_name = strdup("command");
			if (command) {
				event->params[c_param].param_value = command;
			} else {
				event->params[c_param].param_value = strdup("< >");
			}



			// User
			c_param++;
			event->params[c_param].param_name = strdup("user");
			event->params[c_param].param_value = strdup(user_info->pw_name);

			// childpid
			c_param++;
			event->params[c_param].param_name = strdup("child_pid");
			sprintf(cflags,"[%06d]%s", sys_msg->msg_data.msg_ask.args[0], getProcCommand(sys_msg->msg_pid));
			event->params[c_param].param_value = strdup(cflags);

			//status
			c_param++;
			event->params[c_param].param_name = strdup("status");
			memcpy( &p, &sys_msg->msg_data.msg_ask.args[1], sizeof(p) );
			copy_io( cfd,sys_msg->msg_pid, SYSTR_READ, p, &arrfd[0], sizeof(int) );
			//sprintf(cflags,"%d", sys_msg->msg_data.msg_ask.args[1]);
			sprintf(cflags,"%s (exit_status=%d)", byte_to_binary(arrfd[0]), WEXITSTATUS(arrfd[0]));

			event->params[c_param].param_value = strdup(cflags);

			// option
			c_param++;
			event->params[c_param].param_name = strdup("option");
			sprintf(cflags,"%d", sys_msg->msg_data.msg_ask.args[2]);
			event->params[c_param].param_value = strdup(cflags);


			//retvalue
			if (sys_msg->msg_type==SYSTR_MSG_RES){
				c_param++;
				event->params[c_param].param_name = strdup("retval");
				sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
				event->params[c_param].param_value = strdup(cflags);
			}

			//plog(LOG_INFO," %s [%s] (%d) \n", syscallnames[sys_msg->msg_data.msg_ask.code], event->params[c_param].param_value, fd);
			break;

		case SYS_unlink:
		case SYS_execve:


			if (((sys_msg->msg_data.msg_ask.code)==SYS_execve)&&(sys_msg->msg_type==SYSTR_MSG_RES)){
				p_filename = strdup("<empty>");
			} else {
				char rel_filename[PATH_MAX];

				memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof( p ) );

				// get filename according to system call
				get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &rel_filename[0] );

				// make absolute filename out of it
				get_absolute_filename(filename, PATH_MAX, rel_filename, sys_msg->msg_pid);

				p_filename=strdup(&filename[0]);
			}

			if ((IGNORE_SYS_FILES)&&(may_ignore(p_filename,fd))) {
				event = (event_t *)malloc(sizeof(event_t));
				event->event_name = strdup(syscallnames[sys_msg->msg_data.msg_ask.code]);
				event->mode = M_EALL;
				// Parameters: filename
				event->n_params = 0;
				event->params = NULL;


			} else {


				user_info = getUserInfo(sys_msg->msg_pid);
				event = (event_t *)malloc(sizeof(event_t));

				switch (sys_msg->msg_data.msg_ask.code){
				case SYS_unlink:
					event->event_name = strdup("unlink");break;
				case SYS_execve:
					event->event_name = strdup("execve");break;
				}

				event->mode = M_EALL;
				// Parameters: command user filename (+retval)
				//retvalue
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					event->n_params = 5;
				}else {
					event->n_params = 4;
				}

				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup("host x pid");
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				c_param++;		sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
				command = strdup(cflags);
				event->params[c_param].param_name = strdup("command");
				if (command) {
					event->params[c_param].param_value = command;
				} else {
					event->params[c_param].param_value = strdup("< >");
				}




				// User
				c_param++;
				event->params[c_param].param_name = strdup("user");
				event->params[c_param].param_value = strdup(user_info->pw_name);
				// Filename
				c_param++;
				event->params[c_param].param_name = strdup("filename");
				event->params[c_param].param_value = strdup(p_filename);




				//      event->params[c_param].param_value = strdup(" ");



				//plog(LOG_DEBUG," %s [%s] \n", syscallnames[sys_msg->msg_data.msg_ask.code], event->params[c_param].param_value);


				//retvalue
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					c_param++;
					event->params[c_param].param_name = strdup("retval");
					sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
					event->params[c_param].param_value = strdup(cflags);
				}
			}
			break;


			case SYS_kill:

				event = (event_t *)malloc(sizeof(event_t));
				user_info = getUserInfo(sys_msg->msg_pid);
				switch (sys_msg->msg_data.msg_ask.code){
				case SYS_kill:
					event->event_name = strdup("kill");
					break;
					//case SYS_execve:event->event_name = strdup("execve");break;
				}
				event->mode = M_EALL;


				// Parameters: command user target signal (+retval)

				if (sys_msg->msg_type==SYSTR_MSG_RES){
					event->n_params = 6;
				}else {
					event->n_params = 5;
				}


				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup("host x pid");
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				c_param++;		sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
				command = strdup(cflags);
				event->params[c_param].param_name = strdup("command");
				if (command) {
					event->params[c_param].param_value = command;
				} else {
					event->params[c_param].param_value = strdup("< >");
				}



				// User
				c_param++;
				event->params[c_param].param_name = strdup("user");
				event->params[c_param].param_value = strdup(user_info->pw_name);
				// Pid2
				c_param++;
				event->params[c_param].param_name = strdup("target");
				sprintf(cflags,"[%06d]%s", sys_msg->msg_data.msg_ask.args[0], getProcCommand(sys_msg->msg_pid));
				event->params[c_param].param_value = strdup(cflags);
				//plog(LOG_DEBUG," %s [%s] \n", syscallnames[sys_msg->msg_data.msg_ask.code], event->params[c_param].param_value);

				//signal
				c_param++;
				event->params[c_param].param_name = strdup("signal");
				sprintf(cflags,"%d", sys_msg->msg_data.msg_ask.args[1]);
				event->params[c_param].param_value = strdup(cflags);



				//retvalue
				if (sys_msg->msg_type==SYSTR_MSG_RES){
					c_param++;
					event->params[c_param].param_name = strdup("retval");
					sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
					event->params[c_param].param_value = strdup(cflags);
				}

				break;




				case SYS_rename:
					// p is void pointer
					memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof(p) );
					get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &filename[0] );

					memcpy( &p2, &sys_msg->msg_data.msg_ask.args[1], sizeof(p2) );
					get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p2, &filename2[0] );

					user_info = getUserInfo(sys_msg->msg_pid);

					event = (event_t *)malloc(sizeof(event_t));
					event->event_name = strdup("rename");
					event->mode = M_EALL;

					// Event parameters: command, user, old filename, new filename (+retval)

					//retvalue
					if (sys_msg->msg_type==SYSTR_MSG_RES){
						event->n_params = 6;
					} else {
						event->n_params = 5;
					}


					c_param = 0;
					event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

					// 0: PID
					event->params[c_param].param_name = strdup("host x pid");
					event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

					c_param++;		sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
					command = strdup(cflags);
					event->params[c_param].param_name = strdup("command");
					if (command) {
						event->params[c_param].param_value = command;
					} else {
						event->params[c_param].param_value = strdup("< >");
					}



					// User
					c_param++;
					event->params[c_param].param_name = strdup("user");
					event->params[c_param].param_value = strdup(user_info->pw_name);
					// Filename
					c_param++;
					event->params[c_param].param_name = strdup("oldfilename");
					event->params[c_param].param_value = strdup(filename);
					// Filename
					c_param++;
					event->params[c_param].param_name = strdup("newfilename");
					event->params[c_param].param_value = strdup(filename2);

					//     plog(LOG_INFO," %s [%s]->[%s]\n", syscallnames[sys_msg->msg_data.msg_ask.code], filename,filename2);

					//retvalue
					if (sys_msg->msg_type==SYSTR_MSG_RES){
						c_param++;
						event->params[c_param].param_name = strdup("retval");
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
						event->params[c_param].param_value = strdup(cflags);
					}
					break;


				case SYS_fork:
				case SYS_vfork:
					user_info = getUserInfo(sys_msg->msg_pid);

					event = (event_t *)malloc(sizeof(event_t));
					switch (sys_msg->msg_data.msg_ask.code){
					case SYS_fork: event->event_name = strdup("fork");   break;
					case SYS_vfork: event->event_name = strdup("vfork");   break;
					}event->mode = M_EALL;

					// Event parameters: command, user, (retval)

					//retvalue
					if (sys_msg->msg_type==SYSTR_MSG_RES){
						event->n_params = 4;
					} else {
						event->n_params = 3;
					}


					c_param = 0;
					event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

					// 0: PID
					event->params[c_param].param_name = strdup("host x pid");
					event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

					c_param++;		sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
					command = strdup(cflags);
					event->params[c_param].param_name = strdup("command");
					if (command) {
						event->params[c_param].param_value = command;
					} else {
						event->params[c_param].param_value = strdup("< >");
					}



					// User
					c_param++;
					event->params[c_param].param_name = strdup("user");
					event->params[c_param].param_value = strdup(user_info->pw_name);

					//retvalue
					if (sys_msg->msg_type==SYSTR_MSG_RES){
						c_param++;
						event->params[c_param].param_name = strdup("retval");
						sprintf(cflags,"[%06d]%s", sys_msg->msg_data.msg_ask.rval[0], getProcCommand(sys_msg->msg_pid));
						event->params[c_param].param_value = strdup(cflags);
					}
					break;




					case SYS_dup:
					case SYS_dup2:
					case SYS_fcntl:


						handle=1;
						if (sys_msg->msg_data.msg_ask.code==SYS_fcntl){
							if ((sys_msg->msg_data.msg_ask.args[1])!=F_DUPFD){
								handle=0;
							}
						}

						if (handle==1){



							user_info = getUserInfo(sys_msg->msg_pid);

							event = (event_t *)malloc(sizeof(event_t));

							switch (sys_msg->msg_data.msg_ask.code) {
							case SYS_dup:
								event->event_name = strdup("dup");
								break;
							case SYS_dup2:
								event->event_name = strdup("dup2");
								break;
							case SYS_fcntl:
								event->event_name = strdup("fcntl");
								break;
							}
							event->mode = M_EALL;

							// Event parameters: command, user, fd, (retval)

							//retvalue
							if (sys_msg->msg_type==SYSTR_MSG_RES){
								event->n_params = 5;
							} else {
								event->n_params = 4;
							}

							c_param = 0;
							event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

							// 0: PID
							event->params[c_param].param_name = strdup("host x pid");
							event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

							c_param++;		sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
							command = strdup(cflags);
							event->params[c_param].param_name = strdup("command");
							if (command) {
								event->params[c_param].param_value = command;
							} else {
								event->params[c_param].param_value = strdup("< >");
							}



							// User
							c_param++;
							event->params[c_param].param_name = strdup("user");
							event->params[c_param].param_value = strdup(user_info->pw_name);

							// File Descriptor
							c_param++;
							event->params[c_param].param_name = strdup("fd");
							//if (sys_msg->msg_data.msg_ask.code==SYS_dup){
							sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[0]);
							//} else {
							//sprintf(cflags,"%d,%d",sys_msg->msg_data.msg_ask.args[0],sys_msg->msg_data.msg_ask.args[1]);
							//}
							event->params[c_param].param_value = strdup(cflags);

							//retvalue
							if (sys_msg->msg_type==SYSTR_MSG_RES){
								c_param++;
								event->params[c_param].param_name = strdup("retval");
								sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
								event->params[c_param].param_value = strdup(cflags);
							}

						} else {
							event = (event_t *)malloc(sizeof(event_t));
							event->event_name = strdup(syscallnames[sys_msg->msg_data.msg_ask.code]);
							event->mode = M_EALL;
							// Parameters: filename
							event->n_params = 0;
							event->params = NULL;
						}

						break;




					case SYS_socket:
						event = (event_t *)malloc(sizeof(event_t));
						event->event_name = strdup("socket");
						event->mode = M_EALL;

						// Event parameters: command, user, domain, type, protocol (+ return value)

						if (sys_msg->msg_type==SYSTR_MSG_RES){
							event->n_params = 7;
						}else {
							event->n_params = 6;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;


						// 0: PID
						event->params[c_param].param_name = strdup("host x pid");
						event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

						// 1: Command
						c_param++;
						sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
						command = strdup(cflags);
						event->params[c_param].param_name = strdup("command");
						if (command) {
							event->params[c_param].param_value = command;
						} else {
							event->params[c_param].param_value = strdup("< >");
						}

						// 2: User
						c_param++;
						user_info = getUserInfo(sys_msg->msg_pid);
						event->params[c_param].param_name = strdup("user");
						event->params[c_param].param_value = strdup(user_info->pw_name);


						// 3: Domain
						c_param++;
						domain_to_str(cflags,CFLAGS_LEN,sys_msg->msg_data.msg_ask.args[0]);
						event->params[c_param].param_name = strdup("domain");
						event->params[c_param].param_value = strdup(cflags);

						// 4: Type
						c_param++;
						type_to_str(cflags,CFLAGS_LEN,sys_msg->msg_data.msg_ask.args[1]);
						event->params[c_param].param_name = strdup("type");
						event->params[c_param].param_value = strdup(cflags);

						// 5: Protocol
						c_param++;
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[2]);
						event->params[c_param].param_name = strdup("protocol");
						event->params[c_param].param_value = strdup(cflags);

						// 6: retvalue
						if (sys_msg->msg_type==SYSTR_MSG_RES){
							c_param++;
							sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
							event->params[c_param].param_name = strdup("retval");
							event->params[c_param].param_value = strdup(cflags);

							// Tell the file descriptor table that the file descriptor is
							// associated to a network socket
							store_fd_filename_mapping(sys_msg->msg_pid, sys_msg->msg_data.msg_ask.rval[0],FDT_NETWORK_SOCKET);
						}
						break;


					case SYS_accept:
						event = (event_t *)malloc(sizeof(event_t));
						event->event_name = strdup("accept");
						event->mode = M_EALL;

						// Event parameters: command, user, domain, sockfd, struct sockaddr, addrlen (+return value)

						if (sys_msg->msg_type==SYSTR_MSG_RES){
							event->n_params = 8;
						}else {
							event->n_params = 4;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params) ;


						// 0: PID
						event->params[c_param].param_name = strdup("host x pid");
						event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));
						// 1: Command
						c_param++;
						sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
						command = strdup(cflags);
						event->params[c_param].param_name = strdup("command");
						if (command) {
							event->params[c_param].param_value = command;
						} else {
							event->params[c_param].param_value = strdup("< >");
						}

						// 2: User
						c_param++;
						user_info = getUserInfo(sys_msg->msg_pid);
						event->params[c_param].param_name = strdup("user");
						event->params[c_param].param_value = strdup(user_info->pw_name);


						// 3: Socket file descriptor
						c_param++;
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[0]);
						event->params[c_param].param_name = strdup("sockfd");
						event->params[c_param].param_value = strdup(cflags);


						// ON RETURN there is: address family, address, port, return value
						if (sys_msg->msg_type==SYSTR_MSG_RES){
							// get struct sockaddr address
							struct sockaddr_in addr;
							memcpy(&p, &sys_msg->msg_data.msg_ask.args[1], sizeof(p));
							copy_io(cfd, sys_msg->msg_pid, SYSTR_READ, p, &addr, sizeof(struct sockaddr_in));

							// 4: address family
							c_param++;
							sprintf(cflags,"%d",addr.sin_family);
							domain_to_str(cflags,CFLAGS_LEN,addr.sin_family);
							event->params[c_param].param_name = strdup("family");
							event->params[c_param].param_value = strdup(cflags);

							// 5: address
							c_param++;
							snprintf(cflags,CFLAGS_LEN,"%X",ntohl(addr.sin_addr.s_addr));
							event->params[c_param].param_name = strdup("address");
							event->params[c_param].param_value = strdup(cflags);

							// 6: port
							c_param++;
							snprintf(cflags,CFLAGS_LEN,"%d",ntohs(addr.sin_port));
							event->params[c_param].param_name = strdup("port");
							event->params[c_param].param_value = strdup(cflags);

							// 7: return value
							c_param++;
							sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
							event->params[c_param].param_name = strdup("retval");
							event->params[c_param].param_value = strdup(cflags);

							// Tell the file descriptor table that the file descriptor is
							// associated to a network socket
							store_fd_filename_mapping(sys_msg->msg_pid, sys_msg->msg_data.msg_ask.rval[0],FDT_NETWORK_SOCKET);
						}

						break;



					case SYS_connect: {
							event = (event_t *)malloc(sizeof(event_t));
							event->event_name = strdup("connect");
							event->mode = M_EALL;

							int sockfd = sys_msg->msg_data.msg_ask.args[0];

							// Event parameters: command, user, socket file descriptor, struct sockaddr  (+return value)

							if (sys_msg->msg_type==SYSTR_MSG_RES){
								event->n_params = 10;
							} else {
								event->n_params = 7;
							}

							c_param = 0;
							event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;


							// 0: PID
							event->params[c_param].param_name = strdup("host x pid");
							event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

							// 1: Command
							c_param++;
							sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
							command = strdup(cflags);
							event->params[c_param].param_name = strdup("command");
							if (command) {
								event->params[c_param].param_value = command;
							} else {
								event->params[c_param].param_value = strdup("< >");
							}

							// 2: User
							c_param++;
							user_info = getUserInfo(sys_msg->msg_pid);
							event->params[c_param].param_name = strdup("user");
							event->params[c_param].param_value = strdup(user_info->pw_name);


							// 3: Socket file descriptor
							c_param++;
							sprintf(cflags,"%d",sockfd);
							event->params[c_param].param_name = strdup("sockfd");
							event->params[c_param].param_value = strdup(cflags);

							// get struct sockaddr address
							struct sockaddr_in addr;
							memcpy(&p, &sys_msg->msg_data.msg_ask.args[1], sizeof(p));
							copy_io(cfd, sys_msg->msg_pid, SYSTR_READ, p, &addr, sizeof(struct sockaddr_in));

							// 4: address family
							c_param++;
							int af_index = c_param;
							sprintf(cflags,"%d",addr.sin_family);
							domain_to_str(cflags,CFLAGS_LEN,addr.sin_family);
							event->params[c_param].param_name = strdup("family");
							event->params[c_param].param_value = strdup(cflags);

							// 5: address
							c_param++;
							int remote_addr_index =  c_param;
							snprintf(cflags,CFLAGS_LEN,"%X",ntohl(addr.sin_addr.s_addr));
							event->params[c_param].param_name = strdup("remote address");
							event->params[c_param].param_value = strdup(cflags);

							// 6: port
							c_param++;
							int remote_port_index = c_param;
							snprintf(cflags,CFLAGS_LEN,"%d",ntohs(addr.sin_port));
							event->params[c_param].param_name = strdup("remote port");
							event->params[c_param].param_value = strdup(cflags);


							if (sys_msg->msg_type==SYSTR_MSG_RES){
								// find the local ip address and port, in case
								// the kernel did an explicit bind() because
								// the socket was not already bound.
								char addr[BUFSIZE];
								int port;
								if (get_local_socket_address(sys_msg->msg_pid, sockfd, addr, BUFSIZE, &port) == -1) {
									// No local ip address was assigned, therefore something went wrong. The systemcall fails.

									// 7: local address
									c_param++;
									event->params[c_param].param_name = strdup("local address");
									event->params[c_param].param_value = strdup("unspec");

									// 8: local port
									c_param++;
									sprintf(cflags,"%d",port);
									event->params[c_param].param_name = strdup("local port");
									event->params[c_param].param_value = strdup("0");
								}
								else {
									// 7: local address
									c_param++;
									event->params[c_param].param_name = strdup("local address");
									event->params[c_param].param_value = strdup(addr);

									// 8: local port
									c_param++;
									sprintf(cflags,"%d",port);
									event->params[c_param].param_name = strdup("local port");
									event->params[c_param].param_value = strdup(cflags);
								}

								// 9: retvalue
								c_param++;
								sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
								event->params[c_param].param_name = strdup("retval");
								event->params[c_param].param_value = strdup(cflags);
							}

					}
					break;



					case SYS_bind:
						event = (event_t *)malloc(sizeof(event_t));
						event->event_name = strdup("bind");
						event->mode = M_EALL;



						// Event parameters: command, user, socket file descriptor, struct sockaddr  (+return value)

						if (sys_msg->msg_type==SYSTR_MSG_RES){
							event->n_params = 8;
						} else {
							event->n_params = 7;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;


						// 0: PID
						event->params[c_param].param_name = strdup("host x pid");
						event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

						// 1: Command
						c_param++;
						sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
						command = strdup(cflags);
						event->params[c_param].param_name = strdup("command");
						if (command) {
							event->params[c_param].param_value = command;
						} else {
							event->params[c_param].param_value = strdup("< >");
						}

						// 2: User
						c_param++;
						user_info = getUserInfo(sys_msg->msg_pid);
						event->params[c_param].param_name = strdup("user");
						event->params[c_param].param_value = strdup(user_info->pw_name);


						// 3: Socket file descriptor
						c_param++;
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[0]);
						event->params[c_param].param_name = strdup("sockfd");
						event->params[c_param].param_value = strdup(cflags);

						// get struct sockaddr address
						struct sockaddr_in addr;
						memcpy(&p, &sys_msg->msg_data.msg_ask.args[1], sizeof(p));
						copy_io(cfd, sys_msg->msg_pid, SYSTR_READ, p, &addr, sizeof(struct sockaddr_in));

						// 4: address family
						c_param++;
						sprintf(cflags,"%d",addr.sin_family);
						domain_to_str(cflags,CFLAGS_LEN,addr.sin_family);
						event->params[c_param].param_name = strdup("family");
						event->params[c_param].param_value = strdup(cflags);

						// 5: list of ip address
						c_param++;
						if (addr.sin_addr.s_addr == 0) {
							// INADDR_ANY
							get_all_ip_addresses(cflags,CFLAGS_LEN, 1);
						}
						else {
							snprintf(cflags,CFLAGS_LEN,"%X",ntohl(addr.sin_addr.s_addr));
						}
						event->params[c_param].param_name = strdup("address(es)");
						event->params[c_param].param_value = strdup(cflags);

						// 6: port
						c_param++;
						snprintf(cflags,CFLAGS_LEN,"%d",ntohs(addr.sin_port));
						event->params[c_param].param_name = strdup("port");
						event->params[c_param].param_value = strdup(cflags);


						// 7: retvalue
						if (sys_msg->msg_type==SYSTR_MSG_RES){
							c_param++;
							sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
							event->params[c_param].param_name = strdup("retval");
							event->params[c_param].param_value = strdup(cflags);
						}

						break;


					case SYS_listen:
						event = (event_t *)malloc(sizeof(event_t));
						event->event_name = strdup("listen");
						event->mode = M_EALL;

						// Event parameters: command, user, socket file descriptor, backlog (+return value)

						if (sys_msg->msg_type==SYSTR_MSG_RES){
							event->n_params = 6;
						} else {
							event->n_params = 5;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;


						// 0: PID
						event->params[c_param].param_name = strdup("host x pid");
						event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

						// 1: Command
						c_param++;
						sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
						command = strdup(cflags);
						event->params[c_param].param_name = strdup("command");
						if (command) {
							event->params[c_param].param_value = command;
						} else {
							event->params[c_param].param_value = strdup("< >");
						}

						// 2: User
						c_param++;
						user_info = getUserInfo(sys_msg->msg_pid);
						event->params[c_param].param_name = strdup("user");
						event->params[c_param].param_value = strdup(user_info->pw_name);


						// 3: Socket file descriptor
						c_param++;
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[0]);
						event->params[c_param].param_name = strdup("sockfd");
						event->params[c_param].param_value = strdup(cflags);

						// 4: backlog
						c_param++;
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[1]);
						event->params[c_param].param_name = strdup("backlog");
						event->params[c_param].param_value = strdup(cflags);

						// 5: retvalue
						if (sys_msg->msg_type==SYSTR_MSG_RES){
							c_param++;
							sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
							event->params[c_param].param_name = strdup("retval");
							event->params[c_param].param_value = strdup(cflags);
						}


						break;



					case SYS_mmap:
						fd = sys_msg->msg_data.msg_ask.args[4];

						user_info = getUserInfo(sys_msg->msg_pid);

						event = (event_t *)malloc(sizeof(event_t));
						event->event_name = strdup("mmap");
						event->mode = M_EALL;

						// Parameters: command user start length prot flags fd offset (+retval)
						if (sys_msg->msg_type==SYSTR_MSG_RES){
							event->n_params = 10;
						} else{
							event->n_params = 9;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

						// 0: PID
						event->params[c_param].param_name = strdup("host x pid");
						event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

						c_param++;		sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
						command = strdup(cflags);
						event->params[c_param].param_name = strdup("command");
						if (command) {
							event->params[c_param].param_value = command;
						} else {
							event->params[c_param].param_value = strdup("< >");
						}



						// User
						c_param++;
						event->params[c_param].param_name = strdup("user");
						event->params[c_param].param_value = strdup(user_info->pw_name);

						// start
						c_param++;
						event->params[c_param].param_name = strdup("start");
						sprintf(cflags,"%x",sys_msg->msg_data.msg_ask.args[0]);
						//                              sprintf(cflags,"0x%x(%s)",sys_msg->msg_data.msg_ask.args[0],byte_to_binary(sys_msg->msg_data.msg_ask.args[0]));
						event->params[c_param].param_value = strdup(cflags);
						// length
						c_param++;
						event->params[c_param].param_name = strdup("length");
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[1]);
						event->params[c_param].param_value = strdup(cflags);

						// prot
						c_param++;
						event->params[c_param].param_name = strdup("prot");
						event->params[c_param].param_value = strdup(byte_to_binary(sys_msg->msg_data.msg_ask.args[2]));

						// flags
						c_param++;
						event->params[c_param].param_name = strdup("flags");
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[3]);
						event->params[c_param].param_value = strdup(cflags);

						// fd
						c_param++;
						event->params[c_param].param_name = strdup("fd");
						sprintf(cflags,"%d",fd);
						event->params[c_param].param_value = strdup(cflags);

						// offset
						c_param++;
						event->params[c_param].param_name = strdup("offset");
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[5]);
						event->params[c_param].param_value = strdup(cflags);


						if (sys_msg->msg_type==SYSTR_MSG_RES){
							// retval
							c_param++;
							event->params[c_param].param_name = strdup("retval");
/*
				            printf("-------------------PROT_READ  %x(%s)\n",PROT_READ,byte_to_binary(PROT_READ));
				            printf("-------------------PROT_WRITE %x(%s)\n",PROT_WRITE,byte_to_binary(PROT_WRITE));
				            printf("-------------------PROT_EXEC  %x(%s)\n",PROT_EXEC,byte_to_binary(PROT_EXEC));
				            printf("-------------------PROT_NONE  %x(%s)\n",PROT_NONE,byte_to_binary(PROT_NONE));
*/

				            sprintf(cflags,"%x",sys_msg->msg_data.msg_ask.rval[0]);
							event->params[c_param].param_value = strdup(cflags);

						} else{

						}
						break;


						/*
							case SYS_munmap:

							user_info = getUserInfo(sys_msg->msg_pid);

							event = (event_t *)malloc(sizeof(event_t));
							event->event_name = strdup(syscallnames[sys_msg->msg_data.msg_ask.code]);
							event->mode = M_EALL;

							// Parameters: command user
							if (sys_msg->msg_type==SYSTR_MSG_RES){
								event->n_params = 6;
							}else {
								event->n_params = 5;
							}
							c_param = 0;
							event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;
	// Command
	sprintf(cflags,"[%06d]", sys_msg->msg_pid);
	event->params[c_param].param_name = strdup("pid");
	if (cflags) {
		event->params[c_param].param_value = strdup(cflags);
	} else {
		event->params[c_param].param_value = strdup("< >");
	}

	sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
	command = strdup(cflags);
	event->params[c_param].param_name = strdup("command");
	if (command) {
		event->params[c_param].param_value = command;
	} else {
		event->params[c_param].param_value = strdup("< >");
	}



							// User
							c_param++;
							event->params[c_param].param_name = strdup("user");
							event->params[c_param].param_value = strdup(user_info->pw_name);


							// start
							c_param++;
							event->params[c_param].param_name = strdup("start");
//                              sprintf(cflags,"0x%x(%s)",sys_msg->msg_data.msg_ask.args[0],byte_to_binary(sys_msg->msg_data.msg_ask.args[0]));
							sprintf(cflags,"%x",sys_msg->msg_data.msg_ask.args[0]);
							event->params[c_param].param_value = strdup(cflags);

							// length
							c_param++;
							event->params[c_param].param_name = strdup("length");
							sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[1]);
							event->params[c_param].param_value = strdup(cflags);



							if (sys_msg->msg_type==SYSTR_MSG_RES){
								c_param++;
								event->params[c_param].param_name = strdup("retval");
//                                sprintf(cflags,"0x%x(%s)",sys_msg->msg_data.msg_ask.rval[0],byte_to_binary(sys_msg->msg_data.msg_ask.rval[0]));
								sprintf(cflags,"%x",sys_msg->msg_data.msg_ask.rval[0]);
								event->params[c_param].param_value = strdup(cflags);
							}


							break;
						 */




					default:
/*						event = (event_t *)malloc(sizeof(event_t));
						event->event_name = strdup(syscallnames[sys_msg->msg_data.msg_ask.code]);
						event->mode = M_EALL;
						// Parameters: filename
						event->n_params = 0;
						event->params = NULL;
						event->xml_doc=NULL;

*/						event=NULL;




						break;

	}
	if (event!=NULL)event->xml_doc=NULL;
	return event;
}


int main(int argc, char *argv[]) {
	// Systrace structures
	struct str_message sys_msg;
	struct systrace_answer sys_ans;
	struct pollfd pfd[1];
	pid_t pid;
	int k, i, flags;
//	u_int16_t seqnr[100];
//	int seqnr_index=0;


	// XML-RPC service
	SoupServer *server;
	char *handler;
	int port = 8081;
	char *service_name = "/syscall_handler";
	pthread_t gloop_thread;

	// Variables to instantiate a mechanism
	// id + ECA (event, condition, and actions)
	char *id;
	event_t *event;
	// max size of condition is 2048
	// this should be fixed later on
	char condition[2048] = "";
	mechanism_actions_t *actions;
	mechanism_actions_t *r_actions;

	// Time step associated with mechanism and past monitors
	int64_t usec_timestep;

	// Update thread sleep
	int64_t thread_usleep;

	// Mechanisms XML representation
	char *schemaFilename = "Master-EnfLang-FHG.xsd";
	char *mechanismsFilename = "mechanisms.xml";
	xmlSchemaPtr schema = NULL;
	xmlLineNumbersDefault(1);
	xmlNode *rootNode;
	xmlNode *node0;

	if(!init_control_monitor()){
		plog(LOG_ERROR, "Error initializing control monitors");
		return 0;
	}


	// sleep thread in control monitor is 200 miliseconds
	thread_usleep = 200000;
	start_past_monitor_update_thread(thread_usleep);

	plog(LOG_TRACE, "Loading mechanisms.xml");

	xmlDocPtr doc = loadXmlDoc(mechanismsFilename, schemaFilename);
	rootNode = xmlDocGetRootElement(doc);

	// 2 seconds time step for all monitors
	usec_timestep = 2000000;

	// Iterate through all control mechanisms
	for (node0 = rootNode->children; node0; node0 = node0->next) {
		if (strcmp(node0->name, "controlMechanism")==0) {
			plog(LOG_TRACE,"------PARSING NEW MECHANISM-----");

			parseControlMechanism(node0, &id, &event, condition, &actions);

			if(add_mechanism(id, event, condition, actions, usec_timestep)!=R_ERROR) {
				activate_mechanism(id);
			} else {
				plog(LOG_DEBUG,"Error adding mechanism '%s' NOT added", id);
			}
		}
	}

	// Initialize Systrace
	init_systrace(&cfd);
	initializeFDTable();

	// Initialize XML-RPC service
	plog(LOG_TRACE, "Creating XML-RPC HTTP server port %d", port);
	init_glib();
	if (!(server = soup_server_new("port", port, NULL))) {
		plog(LOG_ERROR, "Unable to bind SoupServer to port %d", port);
		exit(1);
	}
	plog(LOG_INFO, "Adding handler [%s]", service_name);
	soup_server_add_handler (server, service_name, event_handler, NULL, NULL);
	gloop = g_main_loop_new (NULL, TRUE);
	soup_server_run_async (server);
	pthread_create(&gloop_thread, NULL, run_gloop, NULL);

	// Code for intercepting system calls
	pfd[0].fd = cfd;
	pfd[0].events = POLLIN;
	char filename[PATH_MAX];
	char scriptname[PATH_MAX];
	struct systrace_scriptname sn;
	void *p;
	int reset_flag = 0;


	char *printlog;

	while(1) {



		if (reset_flag == 1) {
			system("./reload_monitor");
			// execl("/home/workspace/OSLFramework/syscall_handler/syscall_handler","",0);
			// delete_all_mechanisms();
			reset_monitor_table();
			reset_control_table();


			//IF_reset();

			plog(LOG_TRACE, "Loading mechanisms.xml");

			xmlDocPtr doc = loadXmlDoc(mechanismsFilename, schemaFilename);
			rootNode = xmlDocGetRootElement(doc);

			// 2 seconds time step for all monitors
			usec_timestep = 2000000;

			// Iterate through all control mechanisms
			for (node0 = rootNode->children; node0; node0 = node0->next) {
				if (strcmp(node0->name, "controlMechanism")==0) {

					parseControlMechanism(node0, &id, &event, condition, &actions);

					if(add_mechanism(id, event, condition, actions, usec_timestep)!=R_ERROR) {

						activate_mechanism(id);
					} else {
						plog(LOG_DEBUG,"Error adding mechanism '%s' NOT added", id);
					}
				}
			}
			reset_flag =0;
		}

		if(poll(pfd,1,0) > 0){
			if(read(cfd, &sys_msg, sizeof(sys_msg)) == sizeof(sys_msg)){
				// Checks is the received systrace event is a request or a response
				switch (sys_msg.msg_type) {

				// This message is sent whenever the emulation of a process changes
				// SYSTR_MSG_EMUL:

				// This message is sent whenever the effective UID or
				// GID has changed during the execution of a system call.
				// SYSTR_MSG_UGID:

				// This is sent whenever the kernel frees the policy
				// identified by msg_policy.
				// SYSTR_MSG_POLICYFREE:

				// This message is sent whenever, before a call to
				// execve(2) a process is privileged (technically, the
				// process has the P_SUGID or P_SUGIDEXEC flag set),
				// but after the call these privileges have been
				// dropped. The new image name is specified in the
				// path argument.
				// SYSTR_MSG_EXECVE:

				// This message is sent whenever a process gains or loses a child
				case SYSTR_MSG_CHILD:
					// plog(LOG_TRACE,"- Father %d ",sys_msg.msg_pid);
					// plog(LOG_TRACE,"- Child created ", sys_msg.msg_data.msg_child.new_pid);
					// we keep track of the childs to control user information related to the child

					break;

				case SYSTR_MSG_ASK:
					// We handle the system call before
					// it is executed, TRY event

					// The PID of the process being monitored
					// sys_msg.msg_pid

					memset(&sys_ans, 0, sizeof(sys_ans));
					sys_ans.stra_pid = sys_msg.msg_pid;
					sys_ans.stra_seqnr = sys_msg.msg_seqnr;
					sys_ans.stra_flags = SYSTR_FLAGS_RESULT;
					reset_deny_because_of_if();
					event = handle_system_call(cfd, &sys_msg);


					if (event!=NULL){
						for(k = 0; k < event->n_params; k++) {

							event->params[k].param_type=strdup("contUsage"); //events we detect are always contusage
						}

					} else {
//						printf ("<..event=null msg_code=%d> \n",sys_msg.msg_data.msg_ask.code);
						fflush(NULL);
					}



					/*
					 *
					 *
					 * EXECVE must be handled as SYSTR_MSG_ASK, not SYSTR_MSG_RES (otherwise wrong parameter)
					 * EXIT doesn't always have a SYSTR_MSG_RES
					 * WRITE has to be forbidden before being executed
					 *
					 *
					 */
					if (event!=NULL) {
						if (PRINT_TRACE>0){
							if ((PRINT_TRACE>1)||  (event->n_params>0) ){
//									(strcmp(event->event_name,"execve")==0)||
//									(strcmp(event->event_name,"exit")==0)) {
								//plog(LOG_TRACE, "New event-request [%s](%d)", event->event_name, event->n_params);


								printf("++ New ask [%s](%d)\n", event->event_name, event->n_params);
								for(k = 0; k < event->n_params; k++){
									//                    plog(LOG_TRACE, "  - [%s] = [%s]", event->params[k].param_name, event->params[k].param_value);
									printf ("++ [%s= %s]\n", event->params[k].param_name, event->params[k].param_value);
								}
								printf("\n");


								//data_flow_monitor_update(data_flow_monitor, event);
							}
						}
						// In case we have XPath expressions in the formulas

						if (event->n_params>0){
							if ((strcmp(event->event_name,"execve")==0)||
									(strcmp(event->event_name,"exit")==0)||
									(strcmp(event->event_name,"write")==0)) {
								IF_update(event);
							}
						}


						create_xml_doc(event);
						// Print out debug information


						plog(LOG_DEBUG, "New event request [%s](%d)", event->event_name, event->n_params);

						for(k = 0; k < event->n_params; k++){
							plog(LOG_DEBUG, "  - [%s] = [%s]", event->params[k].param_name, event->params[k].param_value);
						}
						// Look for mechanisms
						r_actions = new_event_request(event);

						if(r_actions == NULL){
							plog(LOG_DEBUG, "- Event allowed because there is no triggered mechanism");
						} else {
							plog(LOG_INFO, "- Mechanism triggered, processing actions");

							plog(LOG_INFO, "  Event [%s]", event->event_name);
							for(k = 0; k < event->n_params; k++){
								plog(LOG_INFO, "  - [%s] = [%s]", event->params[k].param_name, event->params[k].param_value);
							}

							plog_actions(LOG_DEBUG, r_actions);

							if ((r_actions->response == ALLOW)) {
								plog(LOG_INFO, "  Syscall allowed");
								sys_ans.stra_policy = SYSTR_POLICY_PERMIT;
							} else {
								plog(LOG_INFO, "  Syscall denied");
								sys_ans.stra_policy = SYSTR_POLICY_NEVER;
							}

							// delay
							if (r_actions->delay !=0) {
								sleep(r_actions->delay);
							}

							// modify
							if (r_actions->n_params >0) {
								switch (sys_msg.msg_data.msg_ask.code) {

								case SYS_open:
									// Modify open filename or flags
									for(i = 0; i < r_actions->n_params; i++) {
										if( strcmp(r_actions->modify[i].param_name,"filename")==0) {
											plog(LOG_INFO, "  Changing filename to [%s]", r_actions->modify[i].param_value);
											open_filename_replace(cfd, sys_msg.msg_pid, sys_msg.msg_seqnr, r_actions->modify[i].param_value);
										} else if(strcmp(r_actions->modify[i].param_name,"flags")==0) {
											if(r_actions->modify[i].param_value){
												plog(LOG_INFO, "  Changing flags to [%s]", r_actions->modify[i].param_value);
												flags = atoi(r_actions->modify[i].param_value);
												open_flag_replace(cfd, sys_msg.msg_pid, sys_msg.msg_seqnr, flags);
											}
										}
									}
									break;

								case SYS_unlink:
									// TODO: update the control monitor and process mechanism actions
									for(i = 0; i < r_actions->n_params; i++) {
										if(strcmp(r_actions->modify[i].param_name,"filename")==0) {
											plog(LOG_INFO, "  Changing filename to [%s]", r_actions->modify[i].param_value);
											open_filename_replace(cfd, sys_msg.msg_pid, sys_msg.msg_seqnr, r_actions->modify[i].param_value);
										}
									}
									break;
								}
							}

							// execute
							for(i = 0; i < r_actions->n_actions; i++) {
								printf ("\n\n\n\n\n\nr_actions->n_actions=%d r_actions->execute[i].id=%s\n\n\n\n\n\n",r_actions->n_actions,r_actions->execute[i].id);
								if( strcmp(r_actions->execute[i].id,"resetMonitor")==0) {
									plog(LOG_TRACE, " Resetting monitor");
									reset_flag=1;
								}else if( strcmp(r_actions->execute[i].id,"encryptWrite")==0) {
									printf("<Encrypt Write>\n");
								} else if( strcmp(r_actions->execute[i].id,"encryptRead")==0) {
									printf("<Encrypt Read>\n");
								}else if( strcmp(r_actions->execute[i].id,"encryptOpen")==0) {
									printf("<Encrypt Open>\n");
								} else if( strcmp(r_actions->execute[i].id,"encryptClose")==0) {
									printf("<Encrypt Close>\n");
  							    }
							}

						}
						// free event memory
						//free_event(event);  _+_+_+_+_+_+_+_+ MOVED DOWN
						if (event!=NULL){
							free_event(event);
							event=NULL;
						}

					}



					if (get_deny_because_of_if()!=0){
						printf ("Syscall denied because of IF's requirements violation\n");
						reset_deny_because_of_if();
						sys_ans.stra_policy = SYSTR_POLICY_NEVER;
					}


					if (ioctl(cfd, STRIOCANSWER, &sys_ans) == -1) {
						err(1, "STRIOCANSWER");
					}

					break;

				case SYSTR_MSG_RES:
					// Syscall response

					switch(sys_msg.msg_data.msg_ask.code){
					case SYS_open:
						memcpy(&p, &sys_msg.msg_data.msg_ask.args[0], sizeof(p) );
						get_string_buffer( cfd, sys_msg.msg_pid, SYSTR_READ, p, &filename[0] );
						store_fd_filename_mapping(sys_msg.msg_pid, sys_msg.msg_data.msg_ask.rval[0],filename );
						break;

					}

					event = handle_system_call(cfd, &sys_msg);
					if (event!=NULL) for(k = 0; k < event->n_params; k++) event->params[k].param_type=strdup("contUsage"); //events we detect are always contusage


					if (event!=NULL) {
						if (PRINT_TRACE > 0){
							if ((PRINT_TRACE > 1)||
									((event->n_params>0) &&
											(event->event_name!=NULL)&&
											(strcmp(event->event_name,"execve")!=0) &&
											(strcmp(event->event_name,"exit")!=0))){
								//plog(LOG_TRACE, "New event-reply [%s](%d)", event->event_name, event->n_params);
								printf("++ New res [%s](%d)\n", event->event_name, event->n_params);
								for(k = 0; k < event->n_params; k++){
									//                    plog(LOG_TRACE, "  - [%s] = [%s]", event->params[k].param_name, event->params[k].param_value);
									printf ("++ [%s= %s]\n", event->params[k].param_name, event->params[k].param_value);
								}
								printf("\n");


							}

						}

						if (event->n_params>0){
							if ((event->event_name!=NULL)&& (strcmp(event->event_name,"execve")!=0) && (strcmp(event->event_name,"exit")!=0) && (strcmp(event->event_name,"write")!=0)) {
								//                  data_flow_monitor_update(data_flow_monitor, event);
								IF_update(event);
							}
						}
					}

					// Send answer to systrace
					memset(&sys_ans, 0, sizeof(sys_ans));
					sys_ans.stra_pid = sys_msg.msg_pid;
					sys_ans.stra_seqnr = sys_msg.msg_seqnr;
					if (ioctl(cfd, STRIOCANSWER, &sys_ans) == -1)
						err(1, "STRIOCANSWER");

					if (event!=NULL){
						free_event(event);
						event=NULL;
					}

					break;
				}
			}
			usleep(1);
		}
		else
			usleep(100);
	}

	return 0;
}


/*
 *
 * TODO
 *
 * - handling mmap
 *
 */

