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

#define IP				"ip"
#define HOST_X_PID_STR 	"host_x_pid"




const char *byte_to_binary (int x){
	static char b[17];
	b[0] = '\0';

	int z;
	for (z = 32768; z > 0; z >>= 1)
	{
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return b;
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
	int pid;
	char *in, *out;
	if (!soup_value_array_get_nth (params, 0, G_TYPE_STRING, &in)) {
		type_error (msg, G_TYPE_STRING, params, 0);
		return -1;
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
	return 0;
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
		switch(i) {
			case SYS_accept:
			//case SYS_bind:
			case SYS_close:
			case SYS_connect:
			case SYS_dup2:
			case SYS_dup:
			case SYS_execve:
			case SYS_exit:
			case SYS_fcntl:
			case SYS_fork:
			case SYS_ftruncate:
			case SYS_kill:
			//case SYS_listen:
			case SYS_mmap:
			case SYS_munmap:
			case SYS_open:
			case SYS_pipe:
			case SYS_pread:
			case SYS_pwrite:
			case SYS_read:
			case SYS_readv:
			case SYS_recvfrom:
			case SYS_recvmsg:
			case SYS_rename:
			case SYS_sendmsg:
			case SYS_sendto:
			case SYS_shutdown:
			case SYS_socket:
			case SYS_truncate:
			case SYS_unlink:
			case SYS_vfork:
			case SYS_wait4:
			case SYS_write:
			case SYS_writev:
				ask_syscalls(cfd,i,strpol);
				break;
		}
	}

	plog(LOG_DEBUG,"Policy installed");
	return 0;
}


int ignore_ip(char *ip) {
	if (strstr(ip, "C0A8A701") == ip			// 192.168.138.1
	||  strstr(ip, "C0A8A702") == ip) {
		return 1;
	}

	return 0;
}


int may_ignore(char *filename, int fd) {

	if 	((filename!=NULL) && (
			substr(filename,".profile") ||
			substr(filename,"terminfo") ||
			substr(filename,".bashrc") ||
			substr(filename,".bash_login") ||
			substr(filename,"/.Xauthority") ||
			substr(filename,".bash_logout") ||
			substr(filename,".bash_history") ||
			substr(filename,".bash_profile") ||
			substr(filename,".curlrc") ||

			strcmp(filename,".")==0 ||
			strcmp(filename,"..")==0 ||
			strcmp(filename,"/etc/group")==0 ||
			strcmp(filename,"/etc/pwd.db")==0 ||
			strcmp(filename,"/etc/pwd.db")==0 ||
			strcmp(filename,"/etc/spwd.db")==0 ||
			strcmp(filename,"/etc/localtime")==0 ||

			strstr(filename,"/dev") == filename ||
			strstr(filename,"/usr/lib/") == filename  ||
			strstr(filename,"/var/tmp/") == filename  ||
			strstr(filename,"/var/run/") == filename  ||
			strstr(filename,"/usr/lib/") == filename  ||
			strstr(filename,"/etc/fonts") == filename ||
			strstr(filename,"/etc/hosts") == filename ||
			strstr(filename,"/etc/pango") == filename ||
			strstr(filename,"/var/cache") == filename ||
			strstr(filename,"/usr/X11R6/") == filename ||
			strstr(filename,"/usr/share/") == filename  ||
			strstr(filename,"/etc/profile") == filename ||
			strstr(filename,"/usr/local/share") == filename ||
			strstr(filename,"/usr/local/lib") == filename ||
			strstr(filename,"/var/www/lib/php/modules") == filename ||
			strstr(filename,"/var/www/dev") == filename ||
			strstr(filename,"/var/www/etc") == filename ||
			strstr(filename,"/var/www/logs") == filename ||
			strstr(filename,"/var/www/tmp") == filename ||
			strstr(filename,"/var/www/usr") == filename ||
			strstr(filename,"/var/www/conf") == filename ||
			strstr(filename,"/var/www/var") == filename ||
			strstr(filename,"/var/www/php.ini") == filename ||
			strstr(filename,"/var/www/php-apache.ini") == filename ||
			strstr(filename,"/etc/rc.conf") == filename ||
			strstr(filename,"/etc/suid_profile") == filename ||
			strstr(filename,"/etc/resolv.conf") == filename ||
			strstr(filename,"/etc/protocols") == filename
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
	int fd;
	int arrfd[2];
	int c_param;
	int* pointertoint;
	struct passwd *user_info;
	event_t *event=NULL;
	char *command;
	int handle=0;

	char rel_filename[PATH_MAX];
	char rel_filename2[PATH_MAX];


	switch (sys_msg->msg_data.msg_ask.code) {

		case SYS_exit:
			user_info = getUserInfo(sys_msg->msg_pid);

			event = (event_t *)malloc(sizeof(event_t));
			event->event_name = strdup("exit");
			event->mode = M_EALL;

			// Event parameters: command, user, status   (+retval)

			if (sys_msg->msg_type==SYSTR_MSG_RES){
				event->n_params = 6;
			}else {
				event->n_params = 5;
			}


			c_param = 0;
			event->params = (param_t*)malloc(sizeof(param_t) * event->n_params );

			// 0: PID
			event->params[c_param].param_name = strdup(HOST_X_PID_STR);
			event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

			// IP Address
			getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
			c_param++;
			event->params[c_param].param_name = strdup(IP);
			event->params[c_param].param_value = strdup(cflags);

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
				event->n_params = 7;
			} else {
				event->n_params = 6;
			}

			c_param = 0;
			event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

			// 0: PID
			event->params[c_param].param_name = strdup(HOST_X_PID_STR);
			event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

			// IP Address
			getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
			c_param++;
			event->params[c_param].param_name = strdup(IP);
			event->params[c_param].param_value = strdup(cflags);

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

			memset(rel_filename,0,PATH_MAX);

			// get filename according to system call
			get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &rel_filename[0] );

			// make absolute filename out of it
			get_absolute_filename(filename, PATH_MAX, rel_filename, sys_msg->msg_pid, command);

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
					event->n_params = 7;
				} else {
					event->n_params = 6;
				}

				c_param = 0;
				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

				// 0: PID
				event->params[c_param].param_name = strdup(HOST_X_PID_STR);
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				// IP Address
				getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
				c_param++;
				event->params[c_param].param_name = strdup(IP);
				event->params[c_param].param_value = strdup(cflags);

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
					event->n_params = 8;
				}else {
					event->n_params = 7;
				}

				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup(HOST_X_PID_STR);
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				// IP Address
				getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
				c_param++;
				event->params[c_param].param_name = strdup(IP);
				event->params[c_param].param_value = strdup(cflags);

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

				memset(rel_filename,0,PATH_MAX);

				// get filename according to system call
				get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &rel_filename[0] );

				// make absolute filename out of it
				get_absolute_filename(filename, PATH_MAX, rel_filename, sys_msg->msg_pid, command);

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
					event->n_params = 7;
				} else {
					event->n_params = 6;
				}
				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup(HOST_X_PID_STR);
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				// IP Address
				getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
				c_param++;
				event->params[c_param].param_name = strdup(IP);
				event->params[c_param].param_value = strdup(cflags);

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
					event->n_params = 7;
				} else {
					event->n_params = 6;
				}


				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup(HOST_X_PID_STR);
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				// IP Address
				getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
				c_param++;
				event->params[c_param].param_name = strdup(IP);
				event->params[c_param].param_value = strdup(cflags);

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
				event->n_params = 8;
			} else {
				event->n_params = 7;
			}


			event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
			c_param = 0;

			// 0: PID
			event->params[c_param].param_name = strdup(HOST_X_PID_STR);
			event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

			// IP Address
			getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
			c_param++;
			event->params[c_param].param_name = strdup(IP);
			event->params[c_param].param_value = strdup(cflags);

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
				memset(rel_filename,0,PATH_MAX);

				memcpy( &p, &sys_msg->msg_data.msg_ask.args[0], sizeof( p ) );

				// get filename according to system call
				get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &rel_filename[0] );

				// make absolute filename out of it
				get_absolute_filename(filename, PATH_MAX, rel_filename, sys_msg->msg_pid, command);

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
					event->n_params = 6;
				}else {
					event->n_params = 5;
				}

				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup(HOST_X_PID_STR);
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				// IP Address
				getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
				c_param++;
				event->params[c_param].param_name = strdup(IP);
				event->params[c_param].param_value = strdup(cflags);

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
					event->n_params = 7;
				}else {
					event->n_params = 6;
				}


				event->params = (param_t*)malloc(sizeof(param_t) * event->n_params);
				c_param = 0;

				// 0: PID
				event->params[c_param].param_name = strdup(HOST_X_PID_STR);
				event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

				// IP Address
				getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
				c_param++;
				event->params[c_param].param_name = strdup(IP);
				event->params[c_param].param_value = strdup(cflags);

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
					memcpy( &p2, &sys_msg->msg_data.msg_ask.args[1], sizeof(p2) );


					memset(rel_filename,0,PATH_MAX);
					memset(rel_filename2,0,PATH_MAX);

					// get filenames according to system call
					get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p, &rel_filename[0] );
					get_string_buffer( cfd, sys_msg->msg_pid, SYSTR_READ, p2, &rel_filename2[0] );

					// make absolute filename out of it
					get_absolute_filename(filename, PATH_MAX, rel_filename, sys_msg->msg_pid, command);
					get_absolute_filename(filename2, PATH_MAX, rel_filename2, sys_msg->msg_pid, command);



					user_info = getUserInfo(sys_msg->msg_pid);

					event = (event_t *)malloc(sizeof(event_t));
					event->event_name = strdup("rename");
					event->mode = M_EALL;

					// Event parameters: command, user, old filename, new filename (+retval)

					//retvalue
					if (sys_msg->msg_type==SYSTR_MSG_RES){
						event->n_params = 7;
					} else {
						event->n_params = 6;
					}


					c_param = 0;
					event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

					// 0: PID
					event->params[c_param].param_name = strdup(HOST_X_PID_STR);
					event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

					// IP Address
					getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
					c_param++;
					event->params[c_param].param_name = strdup(IP);
					event->params[c_param].param_value = strdup(cflags);

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
						event->n_params = 5;
					} else {
						event->n_params = 4;
					}


					c_param = 0;
					event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

					// 0: PID
					event->params[c_param].param_name = strdup(HOST_X_PID_STR);
					event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

					// IP Address
					getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
					c_param++;
					event->params[c_param].param_name = strdup(IP);
					event->params[c_param].param_value = strdup(cflags);

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
								event->n_params = 6;
							} else {
								event->n_params = 5;
							}

							c_param = 0;
							event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

							// 0: PID
							event->params[c_param].param_name = strdup(HOST_X_PID_STR);
							event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

							// IP Address
							getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
							c_param++;
							event->params[c_param].param_name = strdup(IP);
							event->params[c_param].param_value = strdup(cflags);

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
							event->n_params = 8;
						}else {
							event->n_params = 7;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;


						// 0: PID
						event->params[c_param].param_name = strdup(HOST_X_PID_STR);
						event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

						// IP Address
						getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
						c_param++;
						event->params[c_param].param_name = strdup(IP);
						event->params[c_param].param_value = strdup(cflags);

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
							store_fd_filename_mapping(sys_msg->msg_pid, sys_msg->msg_data.msg_ask.rval[0],SOCKET_FILENAME);
						}
						break;


					case SYS_accept:
						event = (event_t *)malloc(sizeof(event_t));
						event->event_name = strdup("accept");
						event->mode = M_EALL;

						// Event parameters: command, user, domain, sockfd, struct sockaddr, addrlen (+return value)

						if (sys_msg->msg_type==SYSTR_MSG_RES){
							event->n_params = 11;
						}else {
							event->n_params = 5;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params) ;


						// 0: PID
						event->params[0].param_name = strdup(HOST_X_PID_STR);
						event->params[0].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

						// 1: IP Address
						getCanonicalIP(cflags,CFLAGS_LEN, event->params[0].param_value+1);
						event->params[1].param_name = strdup(IP);
						event->params[1].param_value = strdup(cflags);

						// 2: Command
						sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
						command = strdup(cflags);
						event->params[2].param_name = strdup("command");
						if (command) {
							event->params[2].param_value = command;
						} else {
							event->params[2].param_value = strdup("< >");
						}

						// 3: User
						user_info = getUserInfo(sys_msg->msg_pid);
						event->params[3].param_name = strdup("user");
						event->params[3].param_value = strdup(user_info->pw_name);


						// 4: Socket file descriptor
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[0]);
						event->params[4].param_name = strdup("sockfd");
						event->params[4].param_value = strdup(cflags);


						// ON RETURN there is: address family, address, port, return value
						if (sys_msg->msg_type==SYSTR_MSG_RES){
							// get struct sockaddr address
							struct sockaddr_in remote_addr;
							memcpy(&p, &sys_msg->msg_data.msg_ask.args[1], sizeof(p));
							copy_io(cfd, sys_msg->msg_pid, SYSTR_READ, p, &remote_addr, sizeof(struct sockaddr_in));

							// 4: address family
							sprintf(cflags,"%d",remote_addr.sin_family);
							domain_to_str(cflags,CFLAGS_LEN,remote_addr.sin_family);
							event->params[5].param_name = strdup("family");
							event->params[5].param_value = strdup(cflags);


							event->params[6].param_name = strdup("local address");
							event->params[7].param_name = strdup("local port");

							char local_addr[BUFSIZE];
							int local_port;
							if (get_local_socket_address(sys_msg->msg_pid, sys_msg->msg_data.msg_ask.rval[0], local_addr, BUFSIZE, &local_port) == -1) {
								// No local ip address was assigned, therefore something went wrong. The systemcall fails.

								// 5: local address
								event->params[6].param_value = strdup("unspec");

								// 6: local port
								event->params[7].param_value = strdup("0");
							}
							else {
								// 5: local address
								event->params[6].param_value = strdup(local_addr);

								// 6: local port
								sprintf(cflags,"%d",local_port);
								event->params[7].param_value = strdup(cflags);
							}



							// 7: address
							snprintf(cflags,CFLAGS_LEN,"%X",ntohl(remote_addr.sin_addr.s_addr));
							event->params[8].param_name = strdup("remote address");
							event->params[8].param_value = strdup(cflags);

							// 8: port
							snprintf(cflags,CFLAGS_LEN,"%d",ntohs(remote_addr.sin_port));
							event->params[9].param_name = strdup("remote port");
							event->params[9].param_value = strdup(cflags);

							// 9: return value
							sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
							event->params[10].param_name = strdup("retval");
							event->params[10].param_value = strdup(cflags);

							// Tell the file descriptor table that the file descriptor is
							// associated to a network socket
							store_fd_filename_mapping(sys_msg->msg_pid, sys_msg->msg_data.msg_ask.rval[0],SOCKET_FILENAME);
						}

						break;



					case SYS_connect: {
							event = (event_t *)malloc(sizeof(event_t));
							event->event_name = strdup("connect");
							event->mode = M_EALL;

							int sockfd = sys_msg->msg_data.msg_ask.args[0];

							// Event parameters: command, user, socket file descriptor, struct sockaddr  (+return value)

							if (sys_msg->msg_type==SYSTR_MSG_RES){
								event->n_params = 11;
							} else {
								event->n_params = 8;
							}

							c_param = 0;
							event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;


							// 0: PID
							event->params[c_param].param_name = strdup(HOST_X_PID_STR);
							event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

							// 1: IP Address
							getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
							c_param++;
							event->params[c_param].param_name = strdup(IP);
							event->params[c_param].param_value = strdup(cflags);

							// 2: Command
							c_param++;
							sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
							command = strdup(cflags);
							event->params[c_param].param_name = strdup("command");
							if (command) {
								event->params[c_param].param_value = command;
							} else {
								event->params[c_param].param_value = strdup("< >");
							}

							// 3: User
							c_param++;
							user_info = getUserInfo(sys_msg->msg_pid);
							event->params[c_param].param_name = strdup("user");
							event->params[c_param].param_value = strdup(user_info->pw_name);


							// 4: Socket file descriptor
							c_param++;
							sprintf(cflags,"%d",sockfd);
							event->params[c_param].param_name = strdup("sockfd");
							event->params[c_param].param_value = strdup(cflags);

							// get struct sockaddr address
							struct sockaddr_in addr;
							memcpy(&p, &sys_msg->msg_data.msg_ask.args[1], sizeof(p));
							copy_io(cfd, sys_msg->msg_pid, SYSTR_READ, p, &addr, sizeof(struct sockaddr_in));

							// 5: address family
							c_param++;
							sprintf(cflags,"%d",addr.sin_family);
							domain_to_str(cflags,CFLAGS_LEN,addr.sin_family);
							event->params[c_param].param_name = strdup("family");
							event->params[c_param].param_value = strdup(cflags);

							// 6: address
							c_param++;
							snprintf(cflags,CFLAGS_LEN,"%X",ntohl(addr.sin_addr.s_addr));
							event->params[c_param].param_name = strdup("remote address");
							event->params[c_param].param_value = strdup(cflags);

							// 7: port
							c_param++;
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
									// No IPv4 address was assigned,
									// therefore either another protocol was used or something went wrong

									// 8: local address
									c_param++;
									event->params[c_param].param_name = strdup("local address");
									event->params[c_param].param_value = strdup("unspec");

									// 9: local port
									c_param++;
									sprintf(cflags,"%d",port);
									event->params[c_param].param_name = strdup("local port");
									event->params[c_param].param_value = strdup("0");

									// in this case we want to ignore further operations on this socket
									delete_fd_filename_mapping(sys_msg->msg_pid, sockfd);
									store_fd_filename_mapping(sys_msg->msg_pid, sockfd, SOCKET_FILENAME_IGNORE);
								}
								else {
									// 8: local address
									c_param++;
									event->params[c_param].param_name = strdup("local address");
									event->params[c_param].param_value = strdup(addr);

									// 9: local port
									c_param++;
									sprintf(cflags,"%d",port);
									event->params[c_param].param_name = strdup("local port");
									event->params[c_param].param_value = strdup(cflags);
								}

								// 10: retvalue
								c_param++;
								sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
								event->params[c_param].param_name = strdup("retval");
								event->params[c_param].param_value = strdup(cflags);
							}


					}
					break;

/*

					case SYS_bind:
						event = (event_t *)malloc(sizeof(event_t));
						event->event_name = strdup("bind");
						event->mode = M_EALL;



						// Event parameters: command, user, socket file descriptor, struct sockaddr  (+return value)

						if (sys_msg->msg_type==SYSTR_MSG_RES){
							event->n_params = 9;
						} else {
							event->n_params = 8;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;


						// 0: PID
						event->params[c_param].param_name = strdup(HOST_X_PID_STR);
						event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

						// 1: IP Address
						getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
						c_param++;
						event->params[c_param].param_name = strdup(IP);
						event->params[c_param].param_value = strdup(cflags);

						// 2: Command
						c_param++;
						sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
						command = strdup(cflags);
						event->params[c_param].param_name = strdup("command");
						if (command) {
							event->params[c_param].param_value = command;
						} else {
							event->params[c_param].param_value = strdup("< >");
						}

						// 3: User
						c_param++;
						user_info = getUserInfo(sys_msg->msg_pid);
						event->params[c_param].param_name = strdup("user");
						event->params[c_param].param_value = strdup(user_info->pw_name);


						// 4: Socket file descriptor
						c_param++;
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[0]);
						event->params[c_param].param_name = strdup("sockfd");
						event->params[c_param].param_value = strdup(cflags);

						// get struct sockaddr address
						struct sockaddr_in addr;
						memcpy(&p, &sys_msg->msg_data.msg_ask.args[1], sizeof(p));
						copy_io(cfd, sys_msg->msg_pid, SYSTR_READ, p, &addr, sizeof(struct sockaddr_in));

						// 5: address family
						c_param++;
						sprintf(cflags,"%d",addr.sin_family);
						domain_to_str(cflags,CFLAGS_LEN,addr.sin_family);
						event->params[c_param].param_name = strdup("family");
						event->params[c_param].param_value = strdup(cflags);

						// 6: list of ip address
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

						// 7: port
						c_param++;
						snprintf(cflags,CFLAGS_LEN,"%d",ntohs(addr.sin_port));
						event->params[c_param].param_name = strdup("port");
						event->params[c_param].param_value = strdup(cflags);


						// 8: retvalue
						if (sys_msg->msg_type==SYSTR_MSG_RES){
							c_param++;
							sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
							event->params[c_param].param_name = strdup("retval");
							event->params[c_param].param_value = strdup(cflags);
						}

						break;
*/
/*
					case SYS_listen:
						event = (event_t *)malloc(sizeof(event_t));
						event->event_name = strdup("listen");
						event->mode = M_EALL;

						// Event parameters: command, user, socket file descriptor, backlog (+return value)

						if (sys_msg->msg_type==SYSTR_MSG_RES){
							event->n_params = 7;
						} else {
							event->n_params = 6;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;


						// 0: PID
						event->params[c_param].param_name = strdup(HOST_X_PID_STR);
						event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

						// 1: IP Address
						getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
						c_param++;
						event->params[c_param].param_name = strdup(IP);
						event->params[c_param].param_value = strdup(cflags);

						// 2: Command
						c_param++;
						sprintf(cflags,"%s", getProcCommand(sys_msg->msg_pid));
						command = strdup(cflags);
						event->params[c_param].param_name = strdup("command");
						if (command) {
							event->params[c_param].param_value = command;
						} else {
							event->params[c_param].param_value = strdup("< >");
						}

						// 3: User
						c_param++;
						user_info = getUserInfo(sys_msg->msg_pid);
						event->params[c_param].param_name = strdup("user");
						event->params[c_param].param_value = strdup(user_info->pw_name);


						// 4: Socket file descriptor
						c_param++;
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[0]);
						event->params[c_param].param_name = strdup("sockfd");
						event->params[c_param].param_value = strdup(cflags);

						// 5: backlog
						c_param++;
						sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.args[1]);
						event->params[c_param].param_name = strdup("backlog");
						event->params[c_param].param_value = strdup(cflags);

						// 6: retvalue
						if (sys_msg->msg_type==SYSTR_MSG_RES){
							c_param++;
							sprintf(cflags,"%d",sys_msg->msg_data.msg_ask.rval[0]);
							event->params[c_param].param_name = strdup("retval");
							event->params[c_param].param_value = strdup(cflags);
						}


						break;


					case SYS_shutdown:
						event = handle_syscall_shutdown(sys_msg);
						break;

*/

					case SYS_mmap:
						fd = sys_msg->msg_data.msg_ask.args[4];

						user_info = getUserInfo(sys_msg->msg_pid);

						event = (event_t *)malloc(sizeof(event_t));						event->event_name = strdup("mmap");
						event->mode = M_EALL;

						// Parameters: command user start length prot flags fd offset (+retval)
						if (sys_msg->msg_type==SYSTR_MSG_RES){
							event->n_params = 11;
						} else{
							event->n_params = 10;
						}

						c_param = 0;
						event->params = (param_t*)malloc(sizeof(param_t) * event->n_params ) ;

						// 0: PID
						event->params[c_param].param_name = strdup(HOST_X_PID_STR);
						event->params[c_param].param_value = strdup(host_x_pid(cflags,CFLAGS_LEN,sys_msg->msg_pid));

						// 1: IP Address
						getCanonicalIP(cflags,CFLAGS_LEN, event->params[c_param].param_value+1);
						c_param++;
						event->params[c_param].param_name = strdup(IP);
						event->params[c_param].param_value = strdup(cflags);

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
	int k, i, flags;


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


	// start syscall_handler service
	init_syscall_handler_service();


	// Code for intercepting system calls
	pfd[0].fd = cfd;
	pfd[0].events = POLLIN;
	char filename[PATH_MAX];
	void *p;
	int reset_flag = 0;


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

					if (sys_msg.msg_data.msg_ask.code < 310){  ///fuck the portability I won't use MAX_SYSCALLS
						event = handle_system_call(cfd, &sys_msg);
						//event = NULL;
					} else {
						event=NULL;
					}


					if (event!=NULL){
						for(k = 0; k < event->n_params; k++) {

							event->params[k].param_type=strdup("contUsage"); //events we detect are always contusage
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

						if (PRINT_TRACE>0){
							if ((PRINT_TRACE>1)||  (event->n_params>0) ){
//									(strcmp(event->event_name,"execve")==0)||
//									(strcmp(event->event_name,"exit")==0)) {
								plog(LOG_TRACE, "New event-request [%s](%d)", event->event_name, event->n_params);


								for(k = 0; k < event->n_params; k++){
									                    plog(LOG_TRACE, "  - [%s] = [%s]", event->params[k].param_name, event->params[k].param_value);
								}


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
						//err(1, "STRIOCANSWER");
					}

					break;

				case SYSTR_MSG_RES:
					// Syscall response

					switch(sys_msg.msg_data.msg_ask.code){
						case SYS_open: {
								char rel_filename[PATH_MAX];
								memset(rel_filename,0,PATH_MAX);

								memcpy(&p, &sys_msg.msg_data.msg_ask.args[0], sizeof(p) );
								get_string_buffer( cfd, sys_msg.msg_pid, SYSTR_READ, p, &rel_filename[0] );

								// make absolute filename out of it
								if (event != NULL && event->n_params >= 3) {
									get_absolute_filename(filename, PATH_MAX, rel_filename, sys_msg.msg_pid, event->params[2].param_value);
								}
								else {
									get_absolute_filename(filename, PATH_MAX, rel_filename, sys_msg.msg_pid, NULL);
								}

								store_fd_filename_mapping(sys_msg.msg_pid, sys_msg.msg_data.msg_ask.rval[0],filename );
							}
							break;
						case SYS_dup:
						case SYS_dup2:
							store_fd_filename_mapping(sys_msg.msg_pid,
													sys_msg.msg_data.msg_ask.rval[0],
													get_file_name(sys_msg.msg_pid, sys_msg.msg_data.msg_ask.args[0]));
					}

					if (sys_msg.msg_data.msg_ask.code < 310){  ///fuck the portability I won't use MAX_SYSCALL, because it changed from 309 to 313, but with a different format that crashes this code.
						event = handle_system_call(cfd, &sys_msg);
						//event = NULL;
					} else {
						event=NULL;
					}

					if (event!=NULL) for(k = 0; k < event->n_params; k++) event->params[k].param_type=strdup("contUsage"); //events we detect are always contusage


					if (event!=NULL) {
						if (PRINT_TRACE > 0){
							if ((PRINT_TRACE > 1)||
									((event->n_params>0) &&
											(event->event_name!=NULL)&&
											(strcmp(event->event_name,"execve")!=0) &&
											(strcmp(event->event_name,"exit")!=0))){
								plog(LOG_TRACE, "New event-reply [%s](%d)", event->event_name, event->n_params);

								for(k = 0; k < event->n_params; k++){
									plog(LOG_TRACE, "  - [%s] = [%s]", event->params[k].param_name, event->params[k].param_value);
								}


							}

						}

						if (event->n_params>0){
							if ((event->event_name!=NULL)&& (strcmp(event->event_name,"execve")!=0) && (strcmp(event->event_name,"exit")!=0) && (strcmp(event->event_name,"write")!=0)) {
								//data_flow_monitor_update(data_flow_monitor, event);
								IF_update(event);
							}
						}
					}

					// Send answer to systrace
					memset(&sys_ans, 0, sizeof(sys_ans));
					sys_ans.stra_pid = sys_msg.msg_pid;
					sys_ans.stra_seqnr = sys_msg.msg_seqnr;
					if (ioctl(cfd, STRIOCANSWER, &sys_ans) == -1) {
						//err(1, "STRIOCANSWER");
					}


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


event_t *handle_syscall_shutdown(struct str_message* sys_msg) {
	char buf[BUFSIZE];

	event_t *event = (event_t *)malloc(sizeof(event_t));
	event->event_name = strdup("shutdown");
	event->mode = M_EALL;

	// Event parameters: command, user, sockfd, how (+retval)

	if (sys_msg->msg_type==SYSTR_MSG_RES){
		event->n_params = 7;
	}else {
		event->n_params = 6;
	}


	event->params = (param_t*)malloc(sizeof(param_t) * event->n_params) ;


	// 0: PID
	event->params[0].param_name = strdup(HOST_X_PID_STR);
	event->params[0].param_value = strdup(host_x_pid(buf,BUFSIZE,sys_msg->msg_pid));

	// 1: IP Address
	getCanonicalIP(buf,BUFSIZE, event->params[0].param_value+1);
	event->params[1].param_name = strdup(IP);
	event->params[1].param_value = strdup(buf);

	// 2: Command
	snprintf(buf,BUFSIZE,"%s", getProcCommand(sys_msg->msg_pid));
	event->params[2].param_name = strdup("command");
	if (buf) {
		event->params[2].param_value = strdup(buf);
	} else {
		event->params[2].param_value = strdup("< >");
	}


	// 3: User
	struct passwd *user_info = getUserInfo(sys_msg->msg_pid);
	event->params[3].param_name = strdup("user");
	event->params[3].param_value = strdup(user_info->pw_name);


	// 4: Socket file descriptor
	snprintf(buf,BUFSIZE,"%d",sys_msg->msg_data.msg_ask.args[0]);
	event->params[4].param_name = strdup("sockfd");
	event->params[4].param_value = strdup(buf);


	// 5: how
	shut_to_str(buf,BUFSIZE,sys_msg->msg_data.msg_ask.args[1]);
	event->params[5].param_name = strdup("how");
	event->params[5].param_value = strdup(buf);


	// 6: return value
	if (sys_msg->msg_type==SYSTR_MSG_RES){
		snprintf(buf,BUFSIZE,"%d",sys_msg->msg_data.msg_ask.rval[0]);
		event->params[6].param_name = strdup("retval");
		event->params[6].param_value = strdup(buf);
	}

	return event;
}



void init_syscall_handler_service() {
	threads_init();

	SoupServer *server;

	plog(LOG_TRACE, "Creating XML-RPC HTTP server port %d", SYSCALL_HANDLER_SERVICE_PORT);
	if (!(server = soup_server_new("port", SYSCALL_HANDLER_SERVICE_PORT, NULL))) {
		plog(LOG_ERROR, "Unable to bind SoupServer to port %d", SYSCALL_HANDLER_SERVICE_PORT);
		exit(1);
	}

	plog(LOG_INFO, "Adding handler [%s]", SYSCALL_HANDLER_SERVICE_NAME);
	soup_server_add_handler (server, SYSCALL_HANDLER_SERVICE_NAME, event_handler, NULL, NULL);
	soup_server_run_async (server);
}
