#include <stdio.h>
#include <stdlib.h>
#include "linked_list.h"
#include "../headers/definitions.h"

struct list *first = NULL;
struct list *last = NULL;

/*
 * This method adds a new element to the list
 *
 * @PARAM:	value	the value to store
 * @PARAM:	value	location where the value is stored
 * 
 * @RETURN:	0	if everything is ok
 * 		-1	if there was an error
 */
int addToList( u_int16_t seqnr, off_t value )
{
	DEBUG_PRINT(("\n(addToList) seqnr=%d, value=%lld\n", seqnr, value));
	
	/* allocate memory */
	struct list *newElement = (struct list *)malloc( sizeof( struct list ) );
	if( newElement == NULL )	/* malloc failed */
	{
		return -1;
	}

	/* set links */
	newElement->seqnr = seqnr;
	newElement->value = value;
	newElement->next = NULL;
	newElement->prev = last;

	if( first == NULL )	/* insert first element into list */
	{
		first = newElement;
		last = newElement;
	}
	else	/* insert not the first element into list */
	{
		last->next = newElement;
		last = newElement;
	}

	return 0;
}


/*
 * This method looks for a value corresponding to seqnr
 *
 * @PARAM:	seqnr	the seqnr whose value you want
 * @PARAM:	value	location where the value is stored
 *
 * @RETURN	0	if element was found
 * 		-1	if element was not found
 */
int getFromList( u_int16_t seqnr, off_t *value )
{
	struct list *ptr = first;

	while( ptr != NULL )
	{
		if( ptr->seqnr == seqnr )
		{
			*value = ptr->value;

			/* delete element */
			if( first == last )
			{
				first = NULL;
				last = NULL;
			}
			else if( ptr == first )
			{
				ptr->next->prev = NULL;
				first = ptr->next;
			}
			else
			{
				ptr->prev->next = ptr->next;
				if( ptr->next != NULL )
				{
					ptr->next->prev = ptr->prev;
				}
			}
			free( ptr );

			return 0;
		}

		ptr = ptr->next;
	}

	return -1;
}
