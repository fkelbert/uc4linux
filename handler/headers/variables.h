#ifndef VARIABLES_H_
#define VARIABLES_H_

#include <sys/types.h>

#define MAGICNUMBER 1637
#define AES_BLOCK_SIZE 32	/*
				 * AES_BLOCK_SIZE may not be longer than size_t
				 * 0 <= AES_BLOCK_SIZE <= 4294967295
				 */
/*
 * the container header
 */
struct containerHeader
{
	int magicNumber;
	int metadata1;
	int metadata2;
	int metadata3;
	int metadata4;
};


/*
 * array to map the public filedescripter
 * to the private filedescripter and store
 * the filename
 */
struct fd_table_entry
{
	char *filename;
	enum boolean monitored;	
};
extern struct fd_table_entry fd_table[];

/*
 * define files that are container-files
 * REMARK: this list can be assumed as given
 */
extern const int AMOUNT_OF_CONTAINERS;
extern char *container_file_list[];

#endif // VARIABLES_H_
