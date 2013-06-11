/*
 * syscallParser.c
 *
 *  Created on: Jun 10, 2013
 *      Author: Florian Kelbert
 */

#include "syscallParser.h"

char int_to_str_buf[BUFLEN_INT];
char long_to_str_buf[BUFLEN_LONG];
char filename_buf[MAX_FILENAME_LEN];

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
 * @param dataStr provided buffer to read into
 * @param length of buffer dataStr
 */
// FIXME it may be much faster to read from /proc/pid/mem
char *getString(pid_t child, long addr, char *dataStr, int len_buf) {
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
		data.val = ptrace(PTRACE_PEEKDATA, child, addr + MULT_ADDR_SIZE(i),
				NULL);
		memcpy(laddr, data.chars, LONG_SIZE);
		++i;
		laddr += LONG_SIZE;
	}
	j = len_buf % LONG_SIZE;
	/* If we have not finished we do last loop */
	if (j != 0) {
		data.val = ptrace(PTRACE_PEEKDATA, child, addr + MULT_ADDR_SIZE(i),
				NULL);
		memcpy(laddr, data.chars, j);
	}
	/* We leave the string well formated */
	dataStr[len_buf] = '\0';

	return (dataStr);
}

/**
 * Parses the system call and may change the syscallcode
 */
event_ptr parseSyscall(event_ptr event, const int pid, long *syscallcode,
		struct user_regs_struct *regs) {

	// in case of SYS_socketcall:
	// change syscallcode and overwrite syscall field
	if (*syscallcode == SYS_socketcall) {
		*syscallcode = SOCKET_OFFSET + regs->ebx;
		eventAddParam(event, EVENT_PARAM_SYSCALL, syscallTable[*syscallcode]);
	}

	switch (*syscallcode) {
		case SYS_exit:
			long_to_str(regs->ebx, long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, "status", long_to_str_buf);
			break;

		case SYS_socket:
			// Domain and type is an array pointed by the
			// second argument of the socket call

			// FIXME not sure that this is correct; in particular the value of TYPE

			long_to_str(ptrace(PTRACE_PEEKDATA, pid, regs->ecx, NULL),
					long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, "domain", long_to_str_buf);

			long_to_str(
					ptrace(PTRACE_PEEKDATA, pid, regs->ecx + ADDRESS_WIDTH, NULL),
					long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, "type", long_to_str_buf);
			break;

		case SYS_accept:
			// FIXME not sure that this is correct

			// The socket file descriptor is the second argument of accept
			long_to_str(ptrace(PTRACE_PEEKDATA, pid, regs->ecx, NULL),
					long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, "sockfd", long_to_str_buf);
			break;
		case SYS_pipe:
			// The file descriptors of pipe are an array pointed by the first argument of the
			// pipe call, first source then destination

			long_to_str(ptrace(PTRACE_PEEKDATA, pid, regs->ebx, NULL),
					long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, "fd src", long_to_str_buf);

			long_to_str(
					ptrace(PTRACE_PEEKDATA, pid, regs->ebx + ADDRESS_WIDTH, NULL),
					long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, "fd dst", long_to_str_buf);

			break;

		case SYS_open:
		case SYS_creat: {
			// filename: first argument
			eventAddParam(event, EVENT_PARAM_FILENAME, getString(pid, regs->ebx, filename_buf, MAX_FILENAME_LEN));

			// flags: second argument
			eventAddParam(event, "flags", (char*) byte_to_binary(regs->ecx));
		}
			break;

			// FIXME: for read: make getting size and buffer conditional for read().
			// Reading the buffer content is particularly stupid if it is the attempted call.
		case SYS_recvfrom:
		case SYS_recvmsg:
		case SYS_readv:
		case SYS_pread64:
		case SYS_read: {
			long destbufaddr = -1;
			long readsize = -1;

			switch (*syscallcode) {
				case SYS_read:
				case SYS_readv:
				case SYS_preadv:
					// file descriptor in the first argument
					long_to_str(regs->ebx, long_to_str_buf, BUFLEN_INT);
					eventAddParam(event, EVENT_PARAM_FILEDESCR,
							long_to_str_buf);
					break;
				case SYS_recvfrom:
				case SYS_recvmsg:
					// file descriptor in the address pointed by its second argument
					long_to_str(ptrace(PTRACE_PEEKDATA, pid, regs->ecx, NULL),
							long_to_str_buf, BUFLEN_LONG);
					eventAddParam(event, EVENT_PARAM_FILEDESCR,
							long_to_str_buf);
					break;
			}

			// buffer destination address
			switch (*syscallcode) {
				case SYS_read:
				case SYS_readv:
				case SYS_pread64:
					// destination buffer address in the second argument
					destbufaddr = regs->ecx;
					break;
				case SYS_recvfrom:
				case SYS_recvmsg:
					// destination buffer address in the second position of an
					// array pointed by the second argument of the call
					destbufaddr = ptrace(PTRACE_PEEKDATA, pid,
							regs->ecx + ADDRESS_WIDTH, NULL);
					break;
			}

			if (destbufaddr != -1) {
				long_to_hex(destbufaddr, long_to_str_buf, BUFLEN_LONG);
				eventAddParam(event, "buf addr", long_to_str_buf);
			}

			// size parameter
			switch (*syscallcode) {
				case SYS_read:
				case SYS_pread64:
					// third argument of the system call
					readsize = regs->edx;
					break;
				case SYS_readv: {
					// sum all the lengths of the strings in the vector pointed by the
					//second argument; the number of strings is given by the third argument
					int count = 0;
					for (int i = 0; i < regs->edx; i++) {
						count += ptrace(PTRACE_PEEKDATA, pid,
								regs->ecx + MULT_ADDR_SIZE(((i << 1) + 1)),
								NULL);
					}
					readsize = count;
					break;
				}
				case SYS_recvfrom:
					// third position of the vector pointed by the second argument of the call
					readsize = ptrace(PTRACE_PEEKDATA, pid,
							regs->ecx + MULT_ADDR_SIZE(2), NULL);
					break;
				case SYS_recvmsg: {
					// In recvmsg, it is like in readv but the address of the vector
					// is given by the third position of the vector pointed by the
					// second argument of recvmsg, and the arguments are pointed by
					// the second argument of the socketcall system call

					// Get the address of the msghdr structure
					long struct_addr = ptrace(PTRACE_PEEKDATA, pid,
							regs->ecx + ADDRESS_WIDTH, NULL);
					// Get the address of the iovector
					long iovect_addr = ptrace(PTRACE_PEEKDATA, pid,
							struct_addr + MULT_ADDR_SIZE(2), NULL);
					// Get the number of strings to count
					long nr = ptrace(PTRACE_PEEKDATA, pid,
							struct_addr + MULT_ADDR_SIZE(3), NULL);
					// Proceed like in readv
					int count = 0;
					for (int i = 0; i < nr; i++) {
						count += ptrace(PTRACE_PEEKDATA, pid,
								iovect_addr + MULT_ADDR_SIZE(((i << 1) + 1)),
								NULL);
					}
					readsize = count;
					break;
				}
			}

			long_to_str(readsize, long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, EVENT_PARAM_SIZE, long_to_str_buf);

			char mybuf[readsize];
			if (destbufaddr != -1) {
				// FIXME getString() reads from child using ptrace().
				// Maybe there is a way to read from the proc file system (/proc) way faster!?
				getString(pid, destbufaddr, mybuf, readsize);
			}
//			eventAddParam(event, "content", mybuf);
		}
			break;
			break;

		case SYS_close:
			// The filedescriptor is the first argument
			long_to_str(regs->ebx, long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, EVENT_PARAM_FILEDESCR, long_to_str_buf);
			break;

		case SYS_unlink:
		case SYS_execve:
			if (DESIRED(event)) {
				char filename[512];
				// The filename is in the first argument of the call
				eventAddParam(event, EVENT_PARAM_FILENAME,
						getString(pid, regs->ebx, filename, 512));
			}

			break;

		case SYS_kill:
			// target process
			long_to_str(regs->ebx, long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, "target", long_to_str_buf);

			// signal parameter
			long_to_str(regs->ecx, long_to_str_buf, BUFLEN_LONG);
			eventAddParam(event, "signal", long_to_str_buf);
			break;

		case SYS_rename:
			// old filename: first argument
			// new filename: second argument
			eventAddParam(event, "oldfilename", getString(pid, regs->ebx, filename_buf, MAX_FILENAME_LEN));
			eventAddParam(event, "newfilename", getString(pid, regs->ecx, filename_buf, MAX_FILENAME_LEN));
			break;
	}

	return (event);
}
