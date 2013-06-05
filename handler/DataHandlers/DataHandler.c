
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>	/* perror() */

#include "../headers/definitions.h"
#include "../headers/variables.h"
#include "./DataHandler.h"
#include "../utilities/encryption.h"
#include "../utilities/functions.h"


int buffer = 1024;
unsigned char *mykey = NULL;

/*
 * This method initializes the DataHandler
 */
void initializeDataHandler()
{
	mykey = createKey( PASSWORD, 32 );
	setKey( mykey );
}


/*
 * This method prepares the data so that it can be
 * stored in the container
 *
 * The buffer should be written to the position where
 * the file pointer points at the moment
 *
 * @PARAM:	pid	the process that is traced
 * @PARAM:	seqnr	unique seqnr
 * @PARAM:	fd	the file we process
 * @PARAM	cfd	pointer to /dev/systrace
 * @PARAM: 	buffer	the data to be processed
 * @PARAM:	size (size_t = unsigned long)	size of the buffer
 * 
 */
struct processedDataToContainer moveToContainer( pid_t pid, u_int16_t seqnr, int fd, int cfd, void *buffer, size_t size )
{

	DEBUG_PRINT(("(moveToContainer): size=%lu\n", size));

	char *ptrBuffer = (char *)buffer;
	int i, j;
	size_t newBlockSize;
	
	/* the filepointer is positioned at that location
	 * where the first byte should be written.
	 */
	off_t startAddress = moveFilepointer( pid, cfd, fd, (off_t)0 );
		DEBUG_PRINT(("(moveToContainer): startAddress=%lld\n", startAddress));
	off_t currentPosition = startAddress;
		DEBUG_PRINT(("(moveToContainer): currentPosition=%lld\n", currentPosition));
	off_t endAddress = startAddress + (size_t)size;
		DEBUG_PRINT(("(moveToContainer): endAddress=%lld\n", endAddress));

	/* in which block does buffer start */
	off_t startBlockAddress = getStartBlockAddress( currentPosition );
		DEBUG_PRINT(("(moveToContainer): startBlockAddress=%lld\n", startBlockAddress));

	/* in which block does buffer end */
	off_t endBlockAddress = getStartBlockAddress( endAddress );
		DEBUG_PRINT(("(moveToContainer): endBlockAddress=%lld\n", endBlockAddress));
	
	/* how many blocks are needed */
	size_t numberOfRounds = ( (endBlockAddress - startBlockAddress) / AES_BLOCK_SIZE ) + 1;
		DEBUG_PRINT(("(moveToContainer): numberOfRounds=%lu\n", numberOfRounds));
	
	
	/* result will store all encrypted bytes
	 * result will be internally divided into blocks
	 */
	void *result = malloc( numberOfRounds * AES_BLOCK_SIZE );
	char *ptrResult = (char *)result;
	memset( result, 0, numberOfRounds * AES_BLOCK_SIZE );
		
	for( i = 0; i < numberOfRounds; i++ )
	{

		/* the beginning of the block?
		 * xxxxx = bytes that have to be changed
		 * ----------------------------------------------------------------------------------------------
		 * |            block        |             xxxxxxxxxxxxxxxxx            |                       |
		 * ----------------------------------------------------------------------------------------------
		 *                           |             |               |            |
		 *                 startBlockAddress  startAddress    endAddress    endBlockAddress
		 *                      (absolut)        (absolut)      (absolut)
		 *
		 *                                  currentPosition
		 *                                     (absolut)
		 */
		 
		/* how many bytes are in the current block */
		off_t nBytesToProcess = AES_BLOCK_SIZE - ( currentPosition % AES_BLOCK_SIZE );
		
		/* maybe, we process to many bytes
		 * if so, stop after size bytes processed */
		if( currentPosition - startAddress + nBytesToProcess > size )
		{
			nBytesToProcess = endAddress - currentPosition;
		}
			DEBUG_PRINT(("(moveToContainer): nBytesToProcess=%lld", nBytesToProcess));
		
		
		/* read the block from the file and store it in the result array */
			DEBUG_PRINT(("(moveToContainer): readBlockFromFile..."));
		off_t address = getStartBlockAddress( currentPosition );
			DEBUG_PRINT(("(moveToContainer): address=%lld\n", address ));
		size_t nBytesRead = readBlockFromFile( fd, address, ptrResult );
			DEBUG_PRINT(("%lu bytes read, done\n", nBytesRead));
		
		if( nBytesRead > 0 )
		{
			/* we first have to decrypt the block */
				DEBUG_PRINT(("(moveToContainer): decryptText..."));
			decryptText( (unsigned char *)ptrResult, nBytesRead );
				DEBUG_PRINT(("done\n"));
		}

		/* where within the block do we have to start replacing the bytes */
		off_t offset = currentPosition - getStartBlockAddress( currentPosition );

		/* replace the bytes */
			DEBUG_PRINT(("(moveToContainer): replaceBytes..."));
		replaceBytes( ptrResult + offset, ptrBuffer, nBytesToProcess );
			DEBUG_PRINT(("done\n"));

		/* how many bytes is the current block */
		newBlockSize = nBytesRead;
		if( nBytesToProcess > nBytesRead )
		{
			newBlockSize = nBytesToProcess;
		}
			DEBUG_PRINT(("(moveToContainer): newBlockSize=%ld\n", newBlockSize));
		
		/* encrypt the block */
		if( handler_debug )
		{
			int i;
			printf("ptrResult: ");
			for( i = 0; i < newBlockSize; i++ )
			{
				printf("%x", *(((char *)ptrResult) + i));
			}
			printf("\n");
		}

		DEBUG_PRINT(("(moveToContainer): encryptText..."));
		char cipher[ newBlockSize ];
		memset( &cipher[0], 0, newBlockSize );
		encryptText( (unsigned char *)ptrResult, (unsigned char *)&cipher[0], newBlockSize );
		memcpy( ptrResult, &cipher[0], newBlockSize );
		DEBUG_PRINT(("done\n"));
		
		if( handler_debug )
		{
			int i;
			printf("cipher: ");
			for( i = 0; i < newBlockSize; i++ )
			{
				printf("%x", cipher[i]);
			}
			printf("\n");
			printf("ptrResult: ");
			for( i = 0; i < newBlockSize; i++ )
			{
				printf("%x", *(((char *)ptrResult) + i));
			}
			printf("\n");
			printf("result: ");
			for( i = 0; i < newBlockSize; i++ )
			{
				printf("%x", *(((char *)result) + i));
			}
			printf("\n");
		}

	
		/* update variables */
		currentPosition += nBytesToProcess;
		ptrBuffer += nBytesToProcess;
		ptrResult += newBlockSize;
	}
		

	/* return buffer that has to be
	 * written to the file
	 */
	struct processedDataToContainer processedData;
	processedData.buffer = result;
	processedData.size = (size_t)(numberOfRounds * AES_BLOCK_SIZE - AES_BLOCK_SIZE + newBlockSize);

	/* store the endAddress in the list so that after the systemcall
	 * the correct filepointer position can be set */
	storeFilePointerPosition( seqnr, endAddress );
	
		if( handler_debug )
		{
			printf("size=%lu\n", processedData.size );
			char tmp[ processedData.size ];
			memcpy( &tmp, processedData.buffer, processedData.size );
			printf("tmp: ");
			for( j = 0; j < processedData.size; j++ )
			{
				printf("%x", tmp[j] );
			}
			printf("\n");
			printf("processedData.buffer: ");
			for( j = 0; j < processedData.size; j++ )
			{
				printf("%x", ((char *)(processedData.buffer))[j] );
			}
			printf("\n");
			decryptText( (unsigned char *)&tmp, processedData.size );
			for( j = 0; j < processedData.size; j++ )
			{
				printf("%x", tmp[j] );
			}
			printf("\n");
			printf("tmp=%s\n", tmp );
		}
	
	/* set the filepointer to the startBlock position */
	currentPosition = moveFilepointer( pid, cfd, fd, (off_t)0 );
	off_t offset = startBlockAddress - currentPosition;
	moveFilepointer( pid, cfd, fd, offset );

	return processedData;

}


/*
 * This method prepares the container data so that it can be
 * forwarded to the application
 *
 * @PARAM:	pid	the process that is traced
 * @PARAM:	fd	the file we process
 * @PARAM	cfd	pointer to /dev/systrace
 * @PARAM: 	buffer	the data to be processed
 * @PARAM:	size	size of the buffer
 * 
 */
void moveFromContainer( pid_t pid, int fd, int cfd, void *buffer, size_t size )
{

	DEBUG_PRINT(("size=%lu\n", size ));

	int i;


	/* the filepointer is positioned at that location
	 * where the last byte was read
	 * move it back to the beginning
	 */
	off_t startAddress = moveFilepointer( pid, cfd, fd, -(off_t)size );
	off_t resetPosition = startAddress + (off_t)size;
		DEBUG_PRINT(("(moveFromContainer): startAddress=%lld\n", startAddress));

	/* in which block does buffer start */
	off_t startBlockAddress = getStartBlockAddress( startAddress );
		DEBUG_PRINT(("(moveFromContainer): startBlockAddress=%lld\n", startBlockAddress));

	/* at which position is the first byte we have to read */
	off_t offset = startAddress - startBlockAddress;
		DEBUG_PRINT(("(moveFromContainer): offset=%lld\n", offset));

	/* at which position is the last byte we have to read */
	off_t endAddress = startAddress + size;
		DEBUG_PRINT(("(moveFromContainer): endAddress=%lld\n", endAddress));

	/* at wich position does the block with the last byte start */
	off_t endBlockAddress = getStartBlockAddress( endAddress );
		DEBUG_PRINT(("(moveFromContainer): endBlockAddress=%lld\n", endBlockAddress));

	/* how many blocks are needed */
	size_t numberOfRounds = ( (endBlockAddress - startBlockAddress) / AES_BLOCK_SIZE ) + 1;
		DEBUG_PRINT(("(moveFromContainer): numberOfRounds=%lu\n", numberOfRounds));

	/* prepare memory to store the plaintext */
	void *plainText = malloc( numberOfRounds * AES_BLOCK_SIZE );
	memset( plainText, 0, numberOfRounds * AES_BLOCK_SIZE );
	char *ptrPlainText = (char *)plainText;

	off_t currentPosition = startBlockAddress;

	for( i = 0; i < numberOfRounds; i++ )
	{

		/* read whole encrypted block */
		size_t nBytesRead = readBlockFromFile( fd, currentPosition, ptrPlainText );

		if( handler_debug )
		{
			int i = 0;
			printf("full block: ");
			for( i = 0; i < nBytesRead; i++ )
				printf("%x", *(((char *)ptrPlainText) + i) );
			printf("\n");
		}
		
		/* decrypt block */
		if( nBytesRead > 0 )
		{	
			/* decrypt */
			decryptText( (unsigned char *)ptrPlainText, nBytesRead );
		}
		
		if( handler_debug )
		{
			int i = 0;
			printf("full block: ");
			for( i = 0; i < nBytesRead; i++ )
				printf("%x", *(((char *)ptrPlainText) + i) );
			printf("\n");
		}
		
		/* update pointers */
		ptrPlainText += nBytesRead;
		currentPosition += nBytesRead;
	}

	/* return the correct bytes */
	memcpy( buffer, plainText + offset, size );

	/* set filepointer position to original value */
	off_t cur = moveFilepointer( pid, cfd, fd, (off_t)0 );
	off_t pos = resetPosition - cur;
	moveFilepointer( pid, cfd, fd, pos );

}
