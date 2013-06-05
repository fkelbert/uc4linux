#ifndef FUNCTIONS_H_
#define FUNCTIONS_H_

#include <sys/param.h>
#include <sys/ioctl.h>
#include <dev/systrace.h>

#include <err.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <string.h>

/*
 * the container header
 */
struct container_header
{
	int a;
	int b;
	int c;
	int d;
};

/*
 * This method sets all bytes to 0 
 */
void initializeFDTable(void);

/*
 * This method stores the header
 *
 * @PARAM:	filename	the filename
 * @PARAM:	header		the header
 *
 * @RETURN:	0		everthing went well
 * 		-1		there was an error
 */
int storeHeader( char *filename, struct containerHeader *header );

/*
 * This method reads the header
 *
 * @PARAM:	filename	the filename
 *
 * @RETURN:	containerHeader		the struct that contains the headerData
 * 		NULL			if there was an error
 */
struct containerHeader *readHeader( char *filename );

/*
 * This method checks if the metadata header
 * exists in the file
 *
 * @PARAM: 	filename	the filename
 *
 * @RETURN:	0	metadata header does not exist
 * 		1	metadata header exists
 */
int metadataHeaderExist( char *filename );

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
int store_fd_filename_mapping( int fd, char *filename );


/*
 * This method deletes the mapping between the
 * filedescriptor and the filename
 *
 * @PARAM:	fd	the filepointer
 *  @RETURN	0	if everything went well
 * 		-1	if there was an error
 */
int delete_fd_filename_mapping( int fd );


/*
 * This method tells you if fd is a container file
 *
 * @PARAM:	fd	filepointer of the file
 *
 * @RETURN	0 if the file is NOT a container file
 * 		1 if the file IS a container file
 */
int is_container_file_by_filepointer( int fd );

/*
 * This method tells you if fd is a container file
 *
 * @PARAM:	filename	the filename of the considered file
 *
 * @RETURN	0 if the file is NOT a container file
 * 		1 if the file IS a container file
 */
int is_container_file_by_filename( char *filename );

/*
 * Method used to copy bytes from the address
 * space of the traced program to the address
 * space of the handler
 * 
 * @PARAM:	fd	the filepointer of the DEVICE
 * @PARAM:	pid	the PID of the traces process
 * @PARAM:	op	do we want to read or write?
 * @PARAM:	addr	from where do we read (addr in userspace)
 * @PARAM:	buf	where do we store the read bytes
 * @PARAM:	size	how many bytes shoud be read
 *
 * @RETURN how many bytes are indeed read
 */
int copy_io(int fd, pid_t pid, int op, void *addr, void *buf, size_t size);


/*
 * This method reads the string where source points to
 *
 * @PARAM:	fd		the filepointer of the DEVICE
 * @PARAM:	pid		the PID of the traces process
 * @PARAM:	op		do we want to read or write?
 * @PARAM:	source		from where do we read
 * @PARAM:	target		where do we store the read bytes
 */
int get_string_buffer( int fd, pid_t pid, int op, void *source, void *target );


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
int get_fixed_size_buffer( int fd, pid_t pid, void *source, void *target, size_t size );
	

/*
 * This method writes SIZE bytes from where SOURCE points to
 * and stores the bytes where TARGET points to
 * 
 * @PARAM: 	fd		the filepointer of the DEVICE
 * @PARAM: 	pid		the PID of the traces process
 * @PARAM: 	source		from where do we read
 * @PARAM: 	target		where do we store the read bytes
 * @PARAM: 	size		the amount of bytes we process
 * 
 * @RETURN: how many bytes are indeed processed
 */
int set_fixed_size_buffer( int fd, pid_t pid, void *source, void *target, size_t size );


/*
 * This method moves the filepointer
 * forward or backwards
 *
 * @PARAM:	 pid		the pid of the traced process
 * @PARAM:	 cfd		the file pointer to the interface /dev/systrace
 * @PARAM:	 fd		the file pointer
 * @PARAM:	 offset		how many bytes should the file pointer be moved
 *
 * @RETURN	the new position after moving the filepointer
 * 		-1, if there was an error
 */

off_t moveFilepointer( pid_t pid, int cfd, int fd, off_t offset );


/*
 * This method sets the filepointer to a given position
 *
 * We implement this method because the offset of fseek
 * is of type long, but the position is of type off_t.
 * Therefore we have to split the movement.
 *
 * @PARAM:	fd	the filepointer that should be moved
 * @PARAM:	pos (off_t)	the position
 *
 * @RETURN	the new position
 * 		-1	if there was an error
 */
off_t moveLocalFilePointer( FILE *filp, off_t pos );


/* 
 * This method is used to let the application
 * read a block from a file
 *
 * @PARAM:	 fd		the file we want to read from
 * @PARAM:	 startPos	the beginning of the block (excluding the header)
 * @PARAM:	 buf		the read bytes are stored in buf
 *
 * @RETURN:	amount of read bytes
 * 		-1	if there was an error
 */
size_t readBlockFromFile( int fd, off_t startPos, void *buf );


int obsd_replace(int fd, pid_t pid, u_int16_t seqnr, struct intercept_replace *repl);

/* This method will replace the filename of
 * the open syscall
 *
 * @PARAM:	 cfd		filepointer to /dev/systrace
 * @PARAM:	 pid		the pid of the traced process
 * @PARAM:	 seqnr		the sequence number of the open syscall
 * 				where you want to change the filename
 * @PARAM:	 newFilename	the new filename
 *
 */
void open_filename_replace( int cfd, pid_t pid, u_int16_t seqnr, char* newFilename );


/* This method replaces some bytes in buffer
*
* @PARAM:	buffer	the buffer where bytes should be replaced
* @PARAM:	new	the new bytes
* @PARAM:	size	how many bytes should be replaced
*/
void replaceBytes( void *buffer, void *new, size_t size );


/*
 * This method replaces the buffer
 * that should be written by the application
 *
 * @PARAM:	cfd		the filepointer to /dev/systrace
 * 		strmsg		information about the traced process
 * 		buffer		the new bytes that should be written
 * 		size		the size of the buffer
 */
void replaceWriteBuffer( int cfd, struct str_message strmsg, void *buffer, size_t size );


/*
 * This method returns the startaddress of the block, where a given byte lies in
 *
 * @PARAM:	currentPosition	the current position of the filePointer
 *
 * @RETURN:	the address of the startBlock
 */
off_t getStartBlockAddress( off_t currentPosition );


/*
 * This method stores the filepointer position
 *
 * @PARAM:	seqnr	the seqnr whose value you want
 * @PARAM:	value	the value to store
 */
void storeFilePointerPosition( u_int16_t seqnr, off_t value );



/*
 * This method gets the filepointer location for a given seqnr
 *
 * @PARAM:	seqnr	the seqnr whose value you want
 * @PARAM:	value	location where the value is stored
 *
 * @RETURN	0	if element was found
 * 		-1	if element was not found
 */
int getFilePointerPosition( u_int16_t seqnr, off_t *value );


/*
 * This method modifies the open systemcall
 * in advance
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleOpenBefore( int cfd, u_int16_t seqnr, struct str_message *strmsg );


/*
 * This method modifies the close systemcall
 * in advance
 *
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleCloseBefore( u_int16_t seqnr, struct str_message *strmsg );


/*
 * This method modifies the read systemcall
 * in advance
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleReadBefore( int cfd, u_int16_t seqnr, struct str_message *strmsg );


/*
 * This method modifies the write systemcall
 * in advance
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleWriteBefore( int cfd, u_int16_t seqnr, struct str_message *strmsg );


/*
 * This method modifies the open systemcall
 * after the systemcall has taken place
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleOpenAfter( int cfd, u_int16_t seqnr, struct str_message *strmsg );


/*
 * This method modifies the read systemcall
 * after the systemcall has taken place
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleReadAfter( int cfd, u_int16_t seqnr, struct str_message *strmsg );


/*
 * This method modifies the write systemcall
 * after the systemcall has taken place
 *
 * @PARAM:	cfd	the filepointer to /dev/systrace
 * @PARAM:	seqnr	the sequence number of the current message
 * @PARAM:	strmsg	information about the systemcall
 */
void handleWriteAfter( int cfd, u_int16_t seqnr, struct str_message *strmsg );

#endif // FUNCTIONS_H_
