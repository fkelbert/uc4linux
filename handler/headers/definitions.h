#ifndef DEFINITIONS_H_
#define DEFINITIONS_H_

#include <string.h>	/* for size_t definition */

/*
 * Number of syscall arguments
 * that will be covered in our policy.
 */
#define INTERCEPT_MAXSYSCALLARGS	10

#define	ICTRANS_NOLINKS	1	/* translation should have no symlinks */
			/* TAKEN FROM INTERCEPT.H */
			
/*
 * definition to use boolean
 */
enum boolean { TRUE = 1, FALSE = 0 };

#define AMOUNT_OF_FDS 128	//TODO: get this number from kernel header file

struct intercept_replace
{
	int num;
	int ind[INTERCEPT_MAXSYSCALLARGS];
	char *address[INTERCEPT_MAXSYSCALLARGS];
	size_t len[INTERCEPT_MAXSYSCALLARGS];
	unsigned int flags[INTERCEPT_MAXSYSCALLARGS];
};

struct processedDataToContainer
{
	void *buffer;
	size_t size;
};

#define handler_debug 1
#define DEBUG_PRINT(y)		if( handler_debug ) printf y;

#endif // DEFINITIONS_H_
