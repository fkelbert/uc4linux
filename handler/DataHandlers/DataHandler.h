#ifndef DATAHANDLER_H_
#define DATAHANDLER_H_

#define PASSWORD "mypassword"

/*
 * This method initializes the DataHandler
 */
void initializeDataHandler();


/*
 * This method prepares the data so that it can be
 * stored in the container
 * 
 * The buffer should be written to the position where
 * the file pointer points at the moment
 *
 * @PARAM:	pid	the pid of the traced process
 * @PARAM:	seqnr	unique seqnr
 * @PARAM:	fd	the file we want to process
 * @PARAM	cfd	pointer to /dev/systrace
 * @PARAM: 	buffer	the data to be processed
 * @PARAM:	size	size of the buffer
 * 
 */
struct processedDataToContainer moveToContainer( pid_t pid, u_int16_t seqnr, int fd, int cfd, void *buffer, size_t size );

/*
 * This method prepares the container data so that it can be
 * forwarded to the application
 *
 * @PARAM:	pid	the pid of the traced process
 * @PARAM:	fd	the file we want to process
 * @PARAM	cfd	pointer to /dev/systrace
 * @PARAM: 	buffer	the data to be processed
 * @PARAM:	size	size of the buffer
 * 
 */
void moveFromContainer( pid_t pid, int fd, int cfd, void *buffer, size_t size );

#endif // DATAHANDLER_H_
