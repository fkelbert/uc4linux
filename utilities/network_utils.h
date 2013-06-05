/*
 * network_utils.h
 *
 *  Created on: 2011/08/08
 *      Author: Florian Kelbert
 */

#ifndef _network_utils_h
#define _network_utils_h


#include <sys/wait.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <ifaddrs.h>
#include <netdb.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define INADDR_ANY_IP 			"0.0.0.0"
#define SOCKET_FILENAME 		"***NETWORK_SOCKET***"
#define SOCKET_FILENAME_IGNORE	"***NETWORK_SOCKET_IGNORE***"
#define IP_ADDR_SEP 			','
#define UNSPEC					"unspec"
#define ADDR_STR_LEN			15
#define IP_LEN_HEX				8
#define PORT_LEN				5
#define ADDRxADDR_STR_LEN		31
#define GLOBAL_ADDR				"[GLOBAL   x   ADDR]"
#define CANONICAL_IP_LEN		15

#define AF_INET_STR				"AF_INET"
#define AF_INET6_STR			"AF_INET6"
#define AF_UNIX_STR				"AF_UNIX"
#define AF_UNSPEC_STR			"AF_UNSPEC"

#define SOCK_STREAM_STR			"SOCK_STREAM"
#define SOCK_DGRAM_STR			"SOCK_DGRAM"
#define SOCK_SEQPACKET_STR		"SOCK_SEQPACKET"
#define SOCK_RAW_STR			"SOCK_RAW"
#define SOCK_RDM_STR			"SOCK_RDM"
#define SOCK_UNKNOWN_STR		"SOCK_UNKNOWN"

#define SHUT_RD_STR				"SHUT_RD"
#define SHUT_WR_STR				"SHUT_WR"
#define SHUT_RDWR_STR			"SHUT_RDWR"
#define SHUT_INVAL_STR			"SHUT_INVAL"
#define SHUT_MAXLEN				11


//char *toADDR(char *dest, int size, char *ip, char *port);
char *toADDRxADDR(char *dest, int size, char *ip1, char *port1, char *ip2, char *port2);
char *toADDRxUNSPEC(char *dest, int size, char *ip1, char *port1);
char *reverseADDRxADDR(char *dest, int size, char *src);

char* type_to_str(char *buf, int buf_size, int type);
char* domain_to_str(char *buf, int buf_size, int domain);
char* shut_to_str(char *buf, int buf_size, int shut_how);
int get_local_socket_address(int pid, int fd, char *ipaddr, int size, int *port);
char *get_all_ip_addresses(char *buf, int size, int lo);
char *host_x_pid(char *buf, int size, int pid);
uint32_t get_host_ip(char *dest, char *ip_addr_list);
char *getCanonicalIP(char *dest, int size, char *hexIP);

char *getRemoteIP(char *dest, char *ADDRxADDR);
char *getLocalIP(char *dest, char *ADDRxADDR);


int is_ADDR(char *pid);
char *to_ADDR(char *HOSTxADDR, char *HOSTxPID);
int is_GLOBAL_IP(char *addr);

#endif
