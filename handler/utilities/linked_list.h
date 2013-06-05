#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_

/* data structure of the list */
struct list
{
	u_int16_t seqnr;	/* the unique sequence number */
	off_t value;	/* the value to store */
	struct list *next;	/* pointer to the next element */
	struct list *prev;	/* pointer to the previous element */
};


/*
 * This method adds a new element to the list
 *
 * @PARAM:	value	the value to store
 * @PARAM:	seqnr	the seqnr whose value you want
 * 
 * @RETURN:	0	if everything is ok
 * 		-1	if there was an error
 */
int addToList( u_int16_t seqnr, off_t value );



/*
 * This method looks for a value corresponding to seqnr
 *
 * @PARAM:	seqnr	the seqnr whose value you want
 * @PARAM:	value	location where the value is stored
 *
 * @RETURN	0	if element was found
 * 		-1	if element was not found
 */
int getFromList( u_int16_t seqnr, off_t *value );

#endif // LINKEDLIST_H_
