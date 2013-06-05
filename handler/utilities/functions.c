#include <fcntl.h>
#include <errno.h>	/* perror() */
#include <sys/stat.h>	/* for fstat() */
#include <sys/limits.h>	/* LONG_MAX, LONG_MIN */

#include "../headers/definitions.h"
#include "../headers/variables.h"
#include "./functions.h"
#include <dev/systrace.h>
#include "./linked_list.h"
#include "./encryption.h"
//#include "../gui/ControllerGUI.h"
#include "../DataHandlers/DataHandler.h"		/* for moveToContainer(),... */

/*
 * This method sets all bytes to 0 
 */
void initializeFDTable(void)
{
	memset( fd_table, 0, AMOUNT_OF_FDS * sizeof( struct fd_table_entry ) );
}


/*
 * This method checks if the metadata header
 * exists in the file
 *
 * @PARAM: 	filename	the filename
 *
 * @RETURN:	0	metadata header does not exist
 * 		1	metadata header exists
 */
int metadataHeaderExist( char *filename )
{
	int res = 0;
	struct containerHeader *header = readHeader( filename );

	if( header != NULL )
	{
		if( header->magicNumber == MAGICNUMBER )
		{
			res = 1;
		}
	} 
	return res;
}

/*
 * This method stores the header
 *
 * @PARAM:	filename	the filename where the header has to be stores
 * @PARAM:	header		the header
 *
 * @RETURN:	0		everthing went well
 * 		-1		there was an error
 */
int storeHeader( char *filename, struct containerHeader *header )
{
	FILE *filp;
	struct containerHeader encryptedHeader;
	
	/* open file */
	filp = fopen( filename, "w" );
	if( filp == NULL )
	{
		perror("ERROR during writing the header\n");
		printf("errno = %d\n", errno );
		return -1;
	}

	/* encrypt header */
	encryptText( (unsigned char *)header, (unsigned char *)&encryptedHeader, sizeof( struct containerHeader ) );
	
	/* write header */
	int writtenBytes = fwrite( &encryptedHeader, 1, sizeof( struct containerHeader ), filp );

	/* close file */
	fclose( filp );

	if( writtenBytes == sizeof( struct containerHeader ) )
		return 0;
	else
		return -1;
}


/*
 * This method reads the header
 *
 * @PARAM:	filename	the filename of the file we want to read the header
 *
 * @RETURN:	struct containerHeader		the struct that contains the headerData
 * 		NULL				if there was an error
 */
struct containerHeader *readHeader( char *filename )
{
	FILE *filp;
	struct containerHeader *header = malloc( sizeof( struct containerHeader ) );
	
	/* set all values in the header to 0 */
	memset( header, 0, sizeof( struct containerHeader ) );
	
	/* open file */
	filp = fopen( filename, "r" );
	if( filp == NULL )
	{
		perror("ERROR during reading the header\n");
		printf("errno = %d\n", errno );
		return NULL;
	}

	/* read header */
	fseek( filp, 0, SEEK_SET );	/* go to the beginning of the file */
	fread( header, 1, sizeof( struct containerHeader ), filp );
	
	/* close file */
	fclose( filp );

	/* decrypt header */
	decryptText( (unsigned char *)header, sizeof( struct containerHeader ) );
	
	if( header->magicNumber == MAGICNUMBER )
	{
		return header;
	}
	else
	{
		return NULL;
	}
}

/*
 * This method tells you if fd is a container file
 *
 * @PARAM: 	fd	filepointer of the file
 *
 * @RETURN	0	 if the file is NOT a container file
 * 		1	 if the file IS a container file
 */
int is_container_file_by_filepointer( int fd )
{
	return fd_table[ fd ].monitored;
}

/*
 * This method tells you if fd is a container file
 *
 * @PARAM: 	filename	the filename of the considered file
 *
 * @RETURN	0	if the file is NOT a container file
 * 		1	if the file IS a container file
 */
int is_container_file_by_filename( char *filename )
{
	int i = -1;
	int result = 0;
	while( ++i < AMOUNT_OF_CONTAINERS && result == 0 )
	{
		result = !strcmp( filename, container_file_list[ i ] );
	}

	return result;
}


/*
 * This method stores the mapping between the
 * filedescriptor and the filename
 *
 * @PARAM:	fd		the filepointer
 * @PARAM:	filename	the corresponding filename
 *
 * @RETURN	0	if everything is fine
 * 		-1	if there was an error
 */
int store_fd_filename_mapping( int fd, char *filename )
{
	/* check if fd is in range */
	if( fd < 0 || fd > ( AMOUNT_OF_FDS - 1 ) )
	{
		/* fd is out of range */
		return -1;
	}

	/* check if filename is != NULL */
	if( filename == NULL )
	{
		return -1;
	}
	
	size_t size = strlen( filename );
	
	fd_table[ fd ].filename = (char *)malloc( size+1 );	/* allocate memory, +1 due to \0 */
	if( fd_table[ fd ].filename == NULL )
	{
		return -1;	/* no memory could be allocated */
	}
	memcpy( fd_table[ fd ].filename, filename, size+1 );	/* copy filename to allocated memory, +1 due to \0 */

	if( is_container_file_by_filename( filename ) )
	{
		fd_table[ fd ].monitored = TRUE;
	}
	else
	{
		fd_table[ fd ].monitored = FALSE;
	}

	/* everything is fine */
	return 0;
}


/*
 * This method deletes the mapping between the
 * filedescriptor and the filename
 *
 * @PARAM:	fd	the filepointer
 *
 * @RETURN	0	if everything went well
 * 		-1	if there was an error
 */
int delete_fd_filename_mapping( int fd )
{
	/* check if fd is in range */
	if( fd < 0 || fd > ( AMOUNT_OF_FDS - 1 ) )
	{
		/* fd is out of range */
		return -1;
	}
	
	free( fd_table[ fd ].filename );		/* free memory */
	fd_table[ fd ].filename = NULL;			/* set char * to NULL */
	fd_table[ fd ].monitored = FALSE;		/* set the monitored bit to FALSE */

	/* everything went well */
	return 0;
}


/*
 * Method used to copy bytes from the address
 * space of the traced program to the address
 * space of the handler
 * 
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	op		do we want to read or write?
 * @PARAM:	addr		from where do we read (addr in userspace)
 * @PARAM:	buf		where do we store the read bytes
 * @PARAM:	size		how many bytes shoud be read
 *
 * @RETURN how many are indeed read
 */
int copy_io(int fd, pid_t pid, int op, void *addr, void *buf, size_t size)
{
	struct systrace_io io;

	memset(&io, 0, sizeof(io));
	io.strio_pid = pid;
	io.strio_addr = buf;
	io.strio_len = size;
	io.strio_offs = addr;
	io.strio_op = op;
	
	if (ioctl(fd, STRIOCIO, &io) == -1) {
		perror( "ioctl(STRIOCIO):" );
		return (-1);
	}

	return (0);
}


/*
 * This method reads the string where source points to
 *
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	op		do we want to read or write?
 * @PARAM:	source		from where do we read
 * @PARAM:	target		where do we store the read bytes
 *
 * @RETURN:	how many bytes are read
 */
int get_string_buffer( int fd, pid_t pid, int op, void *source, void *target )
{
	(unsigned char *)source -= sizeof(char);
	(unsigned char *)target -= sizeof(char);
	int read = -1;
	do
	{
		(unsigned char *)source += sizeof(char);
		(unsigned char *)target += sizeof(char);
		copy_io( fd, pid, op, source, target, 1 );
		read++;
	} while( (*(char *)target != '\0') );

	return read;
}


/*
 * This method reads SIZE bytes from where SOURCE points to
 * and stores the bytes where TARGET points to
 * 
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	source		from where do we read
 * @PARAM:	target		where do we store the read bytes
 * @PARAM:	size		the amount of bytes we process
 * 
 * @RETURN: how many bytes are indeed processed
 */
int get_fixed_size_buffer( int fd, pid_t pid, void *source, void *target, size_t size )
{
	int i, ret;

	/* backup pointers */
	void *source_org = source;
	void *target_org = target;
	
	(unsigned char *)source -= sizeof(char);
	(unsigned char *)target -= sizeof(char);
	
	for( i=0; i < size; i++ )
	{
		(unsigned char *)source += sizeof(char);
		(unsigned char *)target += sizeof(char);
		ret = copy_io( fd, pid, SYSTR_READ, source, target, 1 );
	}

	/* restore pointers */
	source = source_org;
	target = target_org;
	
	return ret;
}


/*
 * This method writes SIZE bytes from where SOURCE points to
 * and stores the bytes where TARGET points to
 * 
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	source		from where do we read
 * @PARAM:	target		where do we store the read bytes
 * @PARAM:	size		the amount of bytes we process
 * 
 * @RETURN: how many bytes are indeed processed
 */
int set_fixed_size_buffer( int fd, pid_t pid, void *source, void *target, size_t size )
{
	int i;
	
	/* backup pointers */
	void *source_org = source;
	void *target_org = target;
	
	(unsigned char *)source -= sizeof(char);
	(unsigned char *)target -= sizeof(char);
	
	for( i=0; i < size; i++ )
	{
		(unsigned char *)source += sizeof(char);
		(unsigned char *)target += sizeof(char);
		if( copy_io( fd, pid, SYSTR_WRITE, target, source, 1 ) != 0 )
			return -1;	/* an error occured */
	}
	
	/* restore pointers */
	source = source_org;
	target = target_org;
	
	return 0;	/* everything is fine */
}


/*
 * This method moves the filepointer
 * forward or backwards
 *
 * @PARAM:	pid	the pid of the traced process
 * @PARAM:	cfd	the file pointer to the interface /dev/systrace
 * @PARAM:	fd	the file pointer
 * @PARAM:	offset	how many bytes should the file pointer be moved
 *
 * @RETURN	the new position after moving the filepointer
 * 		-1, if there was an error
 */
off_t moveFilepointer( pid_t pid, int cfd, int fd, off_t offset )
{
	DEBUG_PRINT(("pid=%d, cfd=%d, fd=%d, offset=%lld\n", pid, cfd, fd, offset ));
	struct filp_modify {			//TODO: move to header
		int fileIndex;
		off_t offset;
		int whence;
		pid_t pid;
		off_t newPosition;
	} my_filp_modify;
	
	/* construct the helper struct */
	my_filp_modify.pid = pid;
	my_filp_modify.offset = offset;
	my_filp_modify.fileIndex = fd;
	my_filp_modify.whence = SEEK_CUR;

	
	/* get current position */
	if( ioctl( cfd, STRIOCFILP, &my_filp_modify ) != 0 )
	{
		perror("Error STRIOCFILP");
		return -1;	/* an error occured */
	}
	
	return my_filp_modify.newPosition;	/* everything went well */
}


/*
 * This method set the filepointer to a given position
 *
 * We implement this method because the offset of fseek
 * is of type long, but the position is of type off_t.
 * Therefore we have to split the movement.
 *
 * @PARAM:	fd		the filepointer that should be moved
 * @PARAM:	pos (off_t)	the position
 *
 * @RETURN	the new position
 * 		-1	if there was an error
 */
off_t moveLocalFilePointer( FILE *filp, off_t pos )
{
	off_t currentPosition = fseek( filp, 0, SEEK_CUR );
	
	while( currentPosition != pos )
	{
		currentPosition = fseek( filp, 0, SEEK_CUR );

		off_t step = pos - currentPosition;
		DEBUG_PRINT( ( "pos=%lld, current=%lld, step=%lld\n", pos, currentPosition, step ) );
	
		if( ( (off_t)LONG_MIN <= step ) && ( step <= (off_t)LONG_MAX ) )
		{
			return fseek( filp, (size_t)step, SEEK_CUR );
		}
		else
		{
			if( step > 0 )
			{
				currentPosition = fseek( filp, LONG_MAX, SEEK_CUR );
			}
			else
			{
				currentPosition = fseek( filp, LONG_MIN, SEEK_CUR );
			}
		}
	}

	return NULL;
}


/* 
 * This method is used to let the application
 * read a block from a file
 *
 * @PARAM:	fd		the file we want to read from
 * @PARAM:	startPos	the beginning of the block (excluding the header)
 * @PARAM:	buf		the read bytes are stored in buf
 *
 * @RETURN:	amount of read bytes
 * 		-1	if there was an error
 */
size_t readBlockFromFile( int fd, off_t startPos, void *buf )
{
	DEBUG_PRINT(("\nfd=%d, startPos=%lld\n", fd, startPos ));
	
	FILE *filp;
	
	/* open file */
	filp = fopen( fd_table[ fd ].filename, "r" );
	if( filp == NULL )
	{
		perror("ERROR during reading a block\n");
		printf("errno = %d\n", errno );
		return -1;
	}
	/* set the filepointer to the position startPos */
	DEBUG_PRINT(("header=%lld\n", (off_t)sizeof( struct containerHeader ) ));
	DEBUG_PRINT(("value=%lld\n", startPos + (off_t)sizeof( struct containerHeader ) ));
	if( moveLocalFilePointer( filp, startPos + (off_t)sizeof( struct containerHeader ) ) >= 0 )
	{
		return fread( buf, 1, AES_BLOCK_SIZE, filp );	/* return type of fread = size_t */
	
		/* close file */
		fclose( filp );
	}
	else
	{
		return -1;
	}
}


int obsd_replace(int fd, pid_t pid, u_int16_t seqnr, struct intercept_replace *repl)
{
	struct systrace_replace replace;
	size_t len, off;
	int i, ret;

	memset(&replace, 0, sizeof(replace));

	for (i = 0, len = 0; i < repl->num; i++) {
		len += repl->len[i];
	}

	replace.strr_pid = pid;
	replace.strr_seqnr = seqnr;
	replace.strr_nrepl = repl->num;
	replace.strr_base = malloc(len);
	replace.strr_len = len;
	if (replace.strr_base == NULL)
		err(1, "%s: malloc", __func__);

	for (i = 0, off = 0; i < repl->num; i++) {
		replace.strr_argind[i] = repl->ind[i];
		replace.strr_offlen[i] = repl->len[i];
		if (repl->len[i] == 0)
		{		/* address[i] contains an integer and not an address */
			replace.strr_off[i] = (size_t)repl->address[i];
			continue;	/* go to the next for-loop */
		}

		replace.strr_off[i] = off;	/* strr_off[i] points to the malloc-area where the value is stored */
		memcpy(replace.strr_base + off, repl->address[i], repl->len[i]);	/* copy value into handler-userspace malloc area */

		if (repl->flags[i] & ICTRANS_NOLINKS)	/* check if flags == ICTRANS_NOLINKS(=1) */
		{
			replace.strr_flags[i] = SYSTR_NOLINKS;
		} else
		{
			replace.strr_flags[i] = 0;
		}

		off += repl->len[i];
	}

	ret = ioctl(fd, STRIOCREPLACE, &replace);
	if (ret == -1 && errno != EBUSY) {
		warn("%s: ioctl", __func__);
	}

	free(replace.strr_base);
	
	return (ret);
}


/* This method will replace the filename of
 * the open syscall
 *
 * @PARAM:	cfd		filepointer to /dev/systrace
 * @PARAM:	pid		the pid of the traced process
 * @PARAM:	seqnr		the sequence number of the open syscall
 * 				where you want to change the filename
 * @PARAM: 	newFilename	the new filename
 *
 */
void open_filename_replace( int cfd, pid_t pid, u_int16_t seqnr, char* newFilename )
{
	/* change the filename */
	struct intercept_replace repl;
	repl.num = 1;	/* we want to change 1 argument in total */
	repl.ind[ 0 ] = 0;	/* we want to change the first argument */
	repl.address[ 0 ] = newFilename;	/* address of the new filename */
	repl.len[ 0 ] = strlen( newFilename ) + 1;	/* how many bytes should be read from address, +1 due to the '\0' */
	repl.flags[ 0 ] = 0;
	
	obsd_replace( cfd, pid, seqnr, &repl );
}


/* This method replaces some bytes in buffer
*
* @PARAM:	buffer	the buffer where bytes should be replaced
* @PARAM:	new	the new bytes
* @PARAM:	size	how many bytes should be replaced
*/
void replaceBytes( void *buffer, void *new, size_t size )
{
	if( handler_debug )
	{
		printf("\nsize=%lu\n", size );
		int i;
		printf("buffer: ");
		for( i = 0; i < size; i++ )
		{
			printf("%x", *(((char *)buffer) + i));
		}
		printf("\n");
		printf("new: ");
		for( i = 0; i < size; i++ )
		{
			printf("%x", *(((char *)new) + i));
		}
		printf("\n");
	}

	/* modify bytes */
	memcpy( ((char *)buffer), new, size );	/* size has to be of type size_t */
	
	if( handler_debug )
	{
		int i;
		printf("buffer: ");
		for( i = 0; i < size; i++ )
		{
			printf("%x", *(((char *)buffer) + i));
		}
		printf("\n");
	}
}


/*
 * This method replaces the buffer
 * that should be written by the application
 *
 * @PARAM:	cfd		the filepointer to /dev/systrace
 * 		strmsg		information about the traced process
 * 		buffer		the new bytes that should be written
 * 		size		the size of the buffer
 */
void replaceWriteBuffer( int cfd, struct str_message strmsg, void *buffer, size_t size )
{
	DEBUG_PRINT(("buffer=%s, size=%lu", (char *)buffer, size));
	
	struct intercept_replace repl;
	repl.num = 2;	/* we want to change 2 arguments in total */
							
				
	/* in the cells nr 1 are the information for the 1st argument */
	repl.ind[ 0 ] = 1;	/* we want to change the 2nd argument */
	repl.address[ 0 ] = buffer;	/* new pointer to file content we want to write */
	repl.len[ 0 ] = size;	/* to how many bytes points the address, +1 due to '\0' */
	repl.flags[ 0 ] = 0;

						
	/* in the cells nr 2 are the information for the 2nd argument */
	repl.ind[ 1 ] = 2;	/* we want to change the 3rd argument */
	(int)repl.address[ 1 ] = size; /* the pointer value is interpreted as an integer that representates the amount of bytes we want to write, without +1 because '\0' does not count here */
	repl.len[ 1 ] = 0; /* because we write an integer, the length has to be zero, because "the address" is not an address and points to 0 bytes */
	repl.flags[ 1 ] = 0;
	
	/* do replacement */
	obsd_replace( cfd, strmsg.msg_pid, strmsg.msg_seqnr, &repl );
}


/*
 * This method returns the startaddress of the block, where a given byte lies in
 *
 * @PARAM:	currentPosition		the current position of the filePointer
 *
 * @RETURN:	the address of the startBlock
 */
off_t getStartBlockAddress( off_t currentPosition )
{
	DEBUG_PRINT(("(getStartBlockAddress): currentPosition=%lld\n", currentPosition ));
	DEBUG_PRINT(("(getStartBlockAddress): currentPosition mod AES_BLOCK_SIZE=%lld\n", (currentPosition % AES_BLOCK_SIZE) ));
	DEBUG_PRINT(("(getStartBlockAddress): returnvalue:=%lld\n", currentPosition - ( currentPosition % AES_BLOCK_SIZE ) ));
	/* return start address */
	return currentPosition - ( currentPosition % AES_BLOCK_SIZE );
}


/*
 * This method stores the filepointer position
 *
 * @PARAM:	seqnr	the seqnr whose value you want
 * @PARAM:	value	the value to store
 */
void storeFilePointerPosition( u_int16_t seqnr, off_t value )
{
		DEBUG_PRINT(("\n(storeFilePointerPosition) seqnr=%d, value=%lld\n", seqnr, value));
	addToList( seqnr, value );
}


/*
 * This method gets the filepointer location for a given seqnr
 *
 * @PARAM:	seqnr	the seqnr whose value you want
 * @PARAM:	value	location where the value is stored
 *
 * @RETURN	0	if element was found
 * 		-1	if element was not found
 */
int getFilePointerPosition( u_int16_t seqnr, off_t *value )
{
		DEBUG_PRINT(("\n(getFilePointerPosition) seqnr=%d\n", seqnr));
	/* we assume that the answer of a syscall has seqnr+1 compared to the
	 * intercepted syscall
	 */
	return getFromList( seqnr-1, value );
}


/*
 * This method modifies the open systemcall
 * in advance
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleOpenBefore( int cfd, u_int16_t seqnr, struct str_message *strmsg )
{
	void *p;
	
	/* Print out the path argument to open(2). */
	int flags = strmsg->msg_data.msg_ask.args[1];
	int mode = strmsg->msg_data.msg_ask.args[2];
	memcpy( &p, &strmsg->msg_data.msg_ask.args[0], sizeof( p ) );

	char filename[255];
	int length = get_string_buffer( cfd, strmsg->msg_pid, SYSTR_READ, p, &filename[0] );
						
	DEBUG_PRINT( ("BEFORE - seqnr: %d - open(pathname: %s, flags: %d, mode: %d)\n", seqnr, filename, flags, mode) );

	/* replace filename */
	if( is_container_file_by_filename( filename ) )
	{
		/* change the filename */
		//open_filename_replace( cfd, strmsg.msg_pid, strmsg.msg_seqnr, "./container_new" );
		memcpy( &p, &strmsg->msg_data.msg_ask.args[0], sizeof( p ) );
		char filename1[255];
		length = get_string_buffer( cfd, strmsg->msg_pid, SYSTR_READ, p, &filename1[0] );
		DEBUG_PRINT(("The filename will change to: %s\n", &filename1[0]));
	}
}


/*
 * This method modifies the close systemcall
 * in advance
 *
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleCloseBefore( u_int16_t seqnr, struct str_message *strmsg )
{
	/* get file descriptor */
	int fd = strmsg->msg_data.msg_ask.args[0];

	/* delete the filepointer - filename mapping */
	delete_fd_filename_mapping( fd );
						
	DEBUG_PRINT( ("BEFORE - seqnr: %d - close(fd: %d)\n", seqnr, fd) );
}


/*
 * This method modifies the read systemcall
 * in advance
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleReadBefore( int cfd, u_int16_t seqnr, struct str_message *strmsg )
{
	int fd = strmsg->msg_data.msg_ask.args[0];
	size_t size = strmsg->msg_data.msg_ask.args[2];	/* get the size of the buffer */
	DEBUG_PRINT( ("BEFORE - seqnr: %d - read(fd: %d, nbyte: %lu)\n", seqnr, fd, size) );
				
	/*
	 * check if fd is a container file
	 */
	if( is_container_file_by_filepointer( fd ) )
	{
		/* because there are the metainformation
		 * as a header of a container file
		 * we first have to move forward the fd by
		 * the size of the header
		 */
		off_t pos = moveFilepointer( strmsg->msg_pid, cfd, fd, (off_t)sizeof( struct containerHeader ) );
		DEBUG_PRINT(("(handler) position after moving fd=%d: %lld\n", fd, pos ));
	}
}


/*
 * This method modifies the write systemcall
 * in advance
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleWriteBefore( int cfd, u_int16_t seqnr, struct str_message *strmsg )
{
	void *p;

	int fd = strmsg->msg_data.msg_ask.args[0];
	size_t size = strmsg->msg_data.msg_ask.args[2];
						
	char buf[size+1];
	memcpy( &p, &strmsg->msg_data.msg_ask.args[1], sizeof( p ) );
	get_fixed_size_buffer( cfd, strmsg->msg_pid, p, &buf[0], size );
	buf[size] = '\0';
					
	DEBUG_PRINT( ( "BEFORE - seqnr: %d - write(fd: %d, buf: %s, nbyte: %lu)\n", seqnr, fd, &buf[0], size ) );
						
	/* replace content */
	if( is_container_file_by_filepointer( fd ) )
	{
		/*
		 * process the data, so that it can be stored in the container
		 */
		struct processedDataToContainer processedData = moveToContainer( strmsg->msg_pid, seqnr, fd, cfd, &buf, size );
							
		DEBUG_PRINT(("new buffer=%s\nnew size=%lu\n", (char *)processedData.buffer, processedData.size ));
						
		/*
		 * replace the arguments
		 */
		replaceWriteBuffer( cfd, *strmsg, processedData.buffer, processedData.size );
		/* because there are the metainformation
		 * as a header of a container file
		 * we first have to move forward the fd by
		 * the size of the header
		 *
		 * we potentially have to write a full block
		 * if so, we have to start writing at a position
		 * which is before the position the the filepointer
		 * we don't know that, so the Datahandler has to
		 * reposition the filepointer to the correct location
		 * so that we can start writing here without bothering
		 * about the position
		 */
		off_t pos = moveFilepointer( strmsg->msg_pid, cfd, fd, (off_t)sizeof( struct containerHeader ) ); 
		DEBUG_PRINT(("(handler) position after moving fd=%d: %lld\n", fd, pos ));
	}
}


/*
 * This method modifies the open systemcall
 * after the systemcall has taken place
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleOpenAfter( int cfd, u_int16_t seqnr, struct str_message *strmsg )
{
	DEBUG_PRINT( ("(handleOpenAfter): cfd=%d, seqnr=%d\n", cfd, seqnr) );
	void *p;
	int metadata1 = 1, metadata2 = 2, metadata3 = 3, metadata4 = 4;
	
	/* get filename */
	char filename[255];
	memcpy( &p, &strmsg->msg_data.msg_ask.args[0], sizeof( p ) );
	get_string_buffer( cfd, strmsg->msg_pid, SYSTR_READ, p, &filename[0] );

	/* get filedescriptor */
	int fd = strmsg->msg_data.msg_ask.rval[0];
						
	/* store the filepointer - filename mapping */
	store_fd_filename_mapping( fd, filename );
						
	if( is_container_file_by_filepointer( fd ) )
	{
		/* ask for metadata */
		if( !metadataHeaderExist( filename ) )
		{
//			showPopup( &metadata1, &metadata2, &metadata3, &metadata4 );

			/* prepare header */
			struct containerHeader header;
			memset( &header, 0, sizeof( header ) );
			header.magicNumber = MAGICNUMBER;
			header.metadata1 = metadata1;
			header.metadata2 = metadata2;
			header.metadata3 = metadata3;
			header.metadata4 = metadata4;
	
			/* store header */
			storeHeader( filename, &header );
		}
	}
						
	DEBUG_PRINT( ("AFTER open - seqnr: %d - returned filepointer: %d, filename: %s\n", seqnr, fd, filename) );
}


/*
 * This method modifies the read systemcall
 * after the systemcall has taken place
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleReadAfter( int cfd, u_int16_t seqnr, struct str_message *strmsg )
{
	void *p;
	
	int fd = strmsg->msg_data.msg_ask.args[0];	/* get the file pointer */
	size_t amount_of_read_bytes = strmsg->msg_data.msg_ask.rval[0];	/* get the amount of read bytes */
	memcpy( &p, &strmsg->msg_data.msg_ask.args[1], sizeof( p ) );
	char buf[amount_of_read_bytes];
	get_fixed_size_buffer( cfd, strmsg->msg_pid, p, &buf[0], amount_of_read_bytes );
		DEBUG_PRINT( ("AFTER - seqnr: %d - read(fd: %d, buf: %s, read bytes: %lu)\n", seqnr, fd, &buf[0], amount_of_read_bytes) );

	/* replace content */
	if( is_container_file_by_filepointer( fd ) )
	{
		/* move the file pointer back to its correct position */
		off_t pos = moveFilepointer( strmsg->msg_pid, cfd, fd, -(off_t)(sizeof( struct containerHeader )) );
		DEBUG_PRINT(("(handler) position after moving fd=%d: %lld\n", fd, pos ));
						
		/*
		 * process the data, so that it
		 * can be forward to the application
		 */
		moveFromContainer( strmsg->msg_pid, fd, cfd, &buf, amount_of_read_bytes );
		
		/*
		 * replace the buffer
		 */
		set_fixed_size_buffer( cfd, strmsg->msg_pid, &buf[0], p, amount_of_read_bytes );				
	}
}


/*
 * This method modifies the write systemcall
 * after the systemcall has taken place
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleWriteAfter( int cfd, u_int16_t seqnr, struct str_message *strmsg )
{
	int fd = strmsg->msg_data.msg_ask.args[0];	/* get the file pointer */
		DEBUG_PRINT(("(handler: after write) fd=%d\n", fd ));
	size_t size = strmsg->msg_data.msg_ask.args[2];	/* get the size */
		DEBUG_PRINT(("(handler: after write) size=%lu\n", size ));

	if( is_container_file_by_filepointer( fd ) )
	{
		/* we potentially had to write a full block instead of 
		 * a few bytes. Therefore we have to position the 
		 * filepointer in such a way that for the application
		 * it looks that the filepointer just moved a few bytes
		 * instead of a full block
		 * move the file pointer back to its correct position
		 */
		off_t currentPosition = moveFilepointer( strmsg->msg_pid, cfd, fd, (off_t)0 );
			DEBUG_PRINT(("(handler) currentPosition=%lld\n", currentPosition ));
					
		off_t correctPosition;
		if( getFilePointerPosition( seqnr, &correctPosition ) == NULL )
		{
			off_t offset = correctPosition - currentPosition;
			moveFilepointer( strmsg->msg_pid, cfd, fd, offset );
		}
							
		off_t offset = -(off_t)(sizeof( struct containerHeader ) );
			DEBUG_PRINT(("(handler: after write) offset=%lld\n", offset ));
	}
}

