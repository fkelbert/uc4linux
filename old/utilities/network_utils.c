/*
 * network_utils.c
 *
 *  Created on: 2011/08/08
 *      Author: Florian Kelbert
 */
#include "network_utils.h"


/**
 * Returns the ip address that identifies this host in dest,
 * which must provide memory for at least 9 chars.
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
		return (uint32_t) -1;
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
 * Returns a string of all ip addresses of this host in buf of size size. The
 * addresses are separated by IP_ADDR_SEP. The return value is the address of buf.
 * If parameter lo == 1, then the localhost ip address (127.0.0.1 = 7F000001) is included,
 * otherwise it is not.
 *
 * The ip addresses are in hexadecimal format.
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
	int BUFSIZE = 512;

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

		// execute lsof -a -n -i4 -P -p<PID> -d<FD> -Fn
		execl("/usr/local/sbin/lsof","lsof","-a","-n","-i4","-P",buf_pid,buf_fd,"-Fn",(void*)0);

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

		// wait for child
		int status;
		wait(&status);

		close(pfd[0]);
	}

	return 0;
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
			snprintf(buf, buf_size, "%s", AF_INET_STR);
			break;
		case AF_INET6:
			snprintf(buf, buf_size, "%s", AF_INET6_STR);
			break;
		case AF_UNIX:
			snprintf(buf, buf_size, "%s", AF_UNIX_STR);
			break;
		default:
			snprintf(buf, buf_size, "%s", AF_UNSPEC_STR);
			break;
	}

	return buf;
}


/**
 * Converts the socket type specified by type (e.g. SOCK_STREAM, SOCK_DGRAM) to
 * a string. The result is returned in buf, which provides memory
 * for  buf_size chars. If the type is not known "UNKNOWN" is returned.
 *
 * The method returns the address of buf.
 */
char* type_to_str(char *buf, int buf_size, int type) {
	memset(buf,0,buf_size);

	switch(type) {
		case SOCK_STREAM:
			snprintf(buf, buf_size, "%s", SOCK_STREAM_STR);
			break;
		case SOCK_DGRAM:
			snprintf(buf, buf_size, "%s", SOCK_DGRAM_STR);
			break;
		case SOCK_SEQPACKET:
			snprintf(buf, buf_size, "%s", SOCK_SEQPACKET_STR);
			break;
		case SOCK_RAW:
			snprintf(buf, buf_size, "%s", SOCK_RAW_STR);
			break;
		case SOCK_RDM:
			snprintf(buf, buf_size, "%s", SOCK_RDM_STR);
			break;
		default:
			snprintf(buf, buf_size, "%s", SOCK_UNKNOWN_STR);
			break;
	}

	return buf;
}



/**
 * Returns a string for shut_how (shutdown system call) in buf of size buf_size.
 */
char* shut_to_str(char *buf, int buf_size, int shut_how) {
	memset(buf,0,buf_size);

	switch(shut_how) {
		case SHUT_RD:
			snprintf(buf, buf_size, "%s", SHUT_RD_STR);
			break;
		case SHUT_WR:
			snprintf(buf, buf_size, "%s", SHUT_WR_STR);
			break;
		case SHUT_RDWR:
			snprintf(buf, buf_size, "%s", SHUT_RDWR_STR);
			break;
		default:
			snprintf(buf, buf_size, "%s", SHUT_INVAL_STR);
			break;
	}

	return buf;
}



/**
 * Creates IP:Port string in dest of size size.
 * Returns the address of dest.
 *//*
char *toADDR(char *dest, int size, char *ip, char *port) {
	snprintf(dest,size,"%.*s:%.*s",IP_LEN,ip,PORT_LEN,port);
	return dest;
}
*/


/**
 * Creates IP:Port->IP:Port string in dest of size size.
 * Returns the address of dest.
 */
char *toADDRxADDR(char *dest, int size, char *ip1, char *port1, char *ip2, char *port2) {
	snprintf(dest,size,"%.*s:%.*s->%.*s:%.*s",IP_LEN_HEX,ip1,PORT_LEN,port1,IP_LEN_HEX,ip2,PORT_LEN,port2);
	return dest;
}


/**
 * Creates IP:Port->unspec string in dest of size size.
 * Returns the address of dest.
 */
char *toADDRxUNSPEC(char *dest, int size, char *ip1, char *port1) {
	snprintf(dest,size,"%.*s:%.*s->unspec",IP_LEN_HEX,ip1,PORT_LEN,port1);
	return dest;
}


/**
 * Returns the canonical ip address: ddd.ddd.ddd.ddd in dest
 *
 * Therefore dest should provide memory for at least 16 chars (including '\0').
 * size gives the size of dest.
 *
 * The function returns dest or NULL on failure.
 */
char *getCanonicalIP(char *dest, int size, char *hexIP) {
	uint32_t ip;
	if (sscanf(hexIP,"%8X",&ip) == 1) {
		snprintf(dest,size,"%d.%d.%d.%d",ip>>24,(ip>>16)%256,(ip>>8)%256,ip%256);
		return dest;
	}

	return NULL;
}


char *getHexIP(char *dest, int size, char *canonicalIP) {
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t d;

	if (sscanf(canonicalIP,"%d.%d.%d.%d",&a,&b,&c,&d) == 4) {
		snprintf(dest, size, "%2X%2X%2X%2X",a,b,c,d);
		return dest;
	}

	return NULL;
}




int is_ADDR(char *pid) {
	if (	(*(pid+14) == 'A')
		&&	(*(pid+15) == 'D')
		&&	(*(pid+16) == 'D')
		&&	(*(pid+17) == 'R')) {
		return 0;
	}

	return -1;
}


char *to_ADDR(char *HOSTxADDR, char *HOSTxPID) {
	memcpy(HOSTxADDR,HOSTxPID,strlen(HOSTxPID)+1);	// also copy null-byte
	strncpy(HOSTxADDR+12,"  ADDR",6);
	return HOSTxADDR;
}


/**
 * Returns 0, if addr is indeed global.
 * Returns -1, if addr is localhost (0x7f000001)
 */
int is_GLOBAL_IP(char *addr) {
	uint32_t ip;
	if (sscanf(addr,"%8X",&ip) == 1) {
		if (ip != 0x7f000001) {
			return 0;
		}
	}

	return -1;
}



char *reverseADDRxADDR(char *dest, int size, char *src) {
	char local_ip[IP_LEN_HEX+1];
	char remote_ip[IP_LEN_HEX+1];
	char local_port[PORT_LEN+1];
	char remote_port[PORT_LEN+1];

	sscanf(src, "%[^:]:%[^-]->%[^:]:%s",local_ip,local_port,remote_ip,remote_port);
	return toADDRxADDR(dest, size, remote_ip, remote_port, local_ip, local_port);
}


char *getLocalIP(char *dest, char *ADDRxADDR) {
	sscanf(ADDRxADDR, "%[^:]:", dest);
	return dest;
}


char *getRemoteIP(char *dest, char *ADDRxADDR) {
	char trash[ADDRxADDR_STR_LEN+1];
	sscanf(ADDRxADDR, "%[^>]>%[^:]", trash, dest);
	return dest;
}
