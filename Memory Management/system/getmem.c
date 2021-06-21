/* getmem.c - getmem */

#include <xinu.h>

extern volatile int debugmem;
/*------------------------------------------------------------------------
 *  getmem  -  Allocate heap storage, returning lowest word address
 *------------------------------------------------------------------------
 */
char  	*getmem(
	  uint32	nbytes		/* Size of memory requested	*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	memblk	*prev, *curr, *leftover, *best_prev, *best_curr;

	mask = disable();
	if (nbytes == 0) {
		restore(mask);
		return (char *)SYSERR;
	}

    if(debugmem) {
        kprintf("Getmem: nbytes %u \n", nbytes);
    }

	nbytes = (uint32) roundmb(nbytes);	/* Use memblk multiples	*/

	prev = &memlist;
	curr = memlist.mnext;
	best_prev = NULL;			/* pointer to keep track of best fit prev block */
	best_curr = NULL;			/* pointer to keep track of best fit curr block */

	int difference = -1; 

	while (curr != NULL) {			/* Search free list	*/

		if (curr->mlength == nbytes) {	/* Block is exact match	*/
			
			best_curr = curr;
			best_prev = prev;

			difference = curr->mlength - nbytes; /* should evaluate to 0 */

		} else if (curr->mlength > nbytes) { /* Keep track of the last eligible block	*/
			
			/* if no eligible block has been found yet, initialize it */
			if (difference == -1) {
				
				best_prev = prev;
				best_curr = curr;
				difference = curr->mlength - nbytes;
				
				
			} else if ((curr->mlength - nbytes) <= difference) {
				
				/* if the difference is smaller, then this block is a 
				better fit than previous block, so replace it */
				best_prev = prev;
				best_curr = curr;
				difference = curr->mlength - nbytes;
			
			}
			
		} 
		
		/* Move to next block	*/
			prev = curr;
			curr = curr->mnext;
		
	}

	/* if a block has been found */   
	if (difference != -1) {
		
		
		/* If a block with exact match of nbytes has been found */
		if (difference == 0) {
			
			best_prev->mnext = best_curr->mnext;
			memlist.mlength -= nbytes;
			
			restore(mask);
			return (char *)(best_curr);
		
		} else if (best_prev != NULL && best_curr != NULL) {
			
			/* adjust for leftover and return the best fit block */
			leftover = (struct memblk *)((uint32) best_curr +
					nbytes);
			
			best_prev->mnext = leftover;
			leftover->mnext = best_curr->mnext;
			leftover->mlength = best_curr->mlength - nbytes;
			memlist.mlength -= nbytes;
			
			restore(mask);
			return (char *)(best_curr);
		}
	} 
	
	/* if no block has been found */
	restore(mask);
	return (char *)SYSERR;
}
