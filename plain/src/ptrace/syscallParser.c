/*
 * syscallParser.c
 *
 *  Created on: Jun 10, 2013
 *      Author: Florian Kelbert
 */

#include "syscallParser.h"

/*
 * Converts a decimal integer value to a string containing its binary
 * representation.
 *
 * @param x Integer to be converted.
 *
 * @return String representing the converted integer to binary.
 */

const char *byte_to_binary(int x) {
	static char b[17];
	/* Set the end character for the string */
	b[0] = '\0';
	int z;
	/* Shift the bits until we are done */
	for (z = 32768; z > 0; z >>= 1) {
		/* Compare the last bit and concatenate the corresponding bit */
		strcat(b, ((x & z) == z) ? "1" : "0");
	}

	return (b);
}

/*
 * Fetch a string from an address in the memory space of a process.
 *
 * @param child Pid of the child.
 * @param addr Address in the memory space of the child.
 * @param dataStr String we want to retrieve.
 * @param length of buffer dataStr
 */
void getString(pid_t child, long addr, char *dataStr, int len_buf) {
	char *laddr;
	int i, j;
	/* This structure will contain the final string */
	union u {
		long val;
		char chars[LONG_SIZE];
	} data;
	i = 0;
	j = len_buf / LONG_SIZE;
	laddr = dataStr;
	/* We read characters of length long size, as it is how it
	 * is done by ptrace */
	while (i < j) {
		data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * ADDRESS_SIZE,
				NULL);
		memcpy(laddr, data.chars, LONG_SIZE);
		++i;
		laddr += LONG_SIZE;
	}
	j = len_buf % LONG_SIZE;
	/* If we have not finished we do last loop */
	if (j != 0) {
		data.val = ptrace(PTRACE_PEEKDATA, child, addr + i * ADDRESS_SIZE,
				NULL);
		memcpy(laddr, data.chars, j);
	}
	/* We leave the string well formated */
	dataStr[len_buf] = '\0';
}

/**
 * Parses the system call and may change the syscallcode
 */
event_ptr parseSyscall(event_ptr event, const int pid, long *syscallcode,
		struct user_regs_struct *regs) {
	char long_str[BUFLEN_LONG];


	// in case of SYS_socketcall:
	// change syscallcode and overwrite syscall field
	if (*syscallcode == SYS_socketcall) {
		*syscallcode = SOCKET_OFFSET + regs-> ebx;
		eventAddParam(event, "syscall", syscallTable[*syscallcode]);
	}

	switch (*syscallcode) {
	case SYS_exit:
		snprintf(long_str, BUFLEN_LONG, "%ld", regs->ebx);
		eventAddParam(event, "status", long_str);
		break;

	case SYS_socket:
		// Domain and type is an array pointed by the
		// second argument of the socket call

		// FIXME not sure that this is correct; in particular the value of TYPE

		snprintf(long_str, BUFLEN_LONG, "%ld",
				ptrace(PTRACE_PEEKDATA, pid, regs->ecx, NULL));
		eventAddParam(event, "domain", long_str);

		snprintf(long_str, BUFLEN_LONG, "%ld",
				ptrace(PTRACE_PEEKDATA, pid, regs->ecx + ADDRESS_SIZE, NULL));
		eventAddParam(event, "type", long_str);
		break;

	case SYS_accept:
		// FIXME not sure that this is correct

		// The socket file descriptor is the second argument of accept
		snprintf(long_str, BUFLEN_LONG, "%ld",
				ptrace(PTRACE_PEEKDATA, pid, regs->ecx, NULL));
		eventAddParam(event, "sockfd", long_str);
		break;
	case SYS_pipe:
		// The file descriptors of pipe are an array pointed by the first argument of the
		// pipe call, first source then destination

		snprintf(long_str, BUFLEN_LONG, "%ld",
				ptrace(PTRACE_PEEKDATA, pid, regs->ebx, NULL));
		eventAddParam(event, "fd source", long_str);

		snprintf(long_str, BUFLEN_LONG, "%ld",
				ptrace(PTRACE_PEEKDATA, pid, regs->ebx + ADDRESS_SIZE, NULL));
		eventAddParam(event, "fd dest", long_str);

		break;

	case SYS_open:
	case SYS_creat: {
		char filename[STR_LEN];
		// The string located in the first argument
		getString(pid, regs->ebx, filename, STR_LEN);
		eventAddParam(event, "filename", filename);

		// Get the flags of the system call, stored in the second argument
		eventAddParam(event, "flags", (char*) byte_to_binary(regs->ecx));
	}
		break;
	}

	return (event);
}
