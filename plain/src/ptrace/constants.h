
#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#define R_SUCCESS  0
#define R_ERROR    1

#define SYS_STATUS_IN 0
#define SYS_STATUS_OUT 1
#define SYS_STATUS_SKIP 2

#define YES 1
#define NO 0

#define PTRACE_ONLY NO
#define MONITORED_APPLICATION "/usr/bin/xterm"

#define MULT8(INT) (INT << 3)
#define MULT4(INT) (INT << 2)
#define MULT2(INT) (INT << 1)

#define ADDRESS_SIZE 4
#define MULT_ADDR_SIZE(INT) (MULT4(INT))

#define STR_LEN 4096

#define LONG_SIZE sizeof(long)
#define BUFLEN_INT 24
#define BUFLEN_LONG 48




#endif /* CONSTANTS_H_ */
