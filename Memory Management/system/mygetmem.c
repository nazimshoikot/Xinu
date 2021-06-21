/* mygetmem.c - mygetmem */

#include <xinu.h>

int mem_init_count;

/*------------------------------------------------------------------------
 * mygetmem - gets memory block from cache or call normal getmem appropriately
 *-----------------------------------------------------------------------
 */
char *mygetmem(
    uint32 nbytes   /* size of memory block to get */
    )
{
    intmask mask;           /* Saved interrupt mask		*/
    mask = disable();

    int i;

    /* traverse the cache  */
    for(i = 0; i < 20; i++){
        
        if (cache[i].blockSize == nbytes){
                
                /* get the next element */
                struct myBlockList *oldHead = cache[i].list;
                struct myBlockList *newHead = oldHead -> next;

                /* return the required block  */
                char *ret_addr; 
                ret_addr = oldHead -> blockaddr;

                /* delete the old element	*/        
                cache[i].list = newHead;

                /* if no more block remaining, set blocksize to 0 so it is no longer used*/
                if (cache[i].list == NULL) {

                    cache[i].blockSize = 0;
                    
                }
                
                restore(mask);
                return ret_addr;         
        }
    }

    /* Try to get memory from the normal free list */
    char * normal_ret = getmem(nbytes);
    
    /* If memory can be got from the free list without freeing cache*/
    if ( normal_ret != (char*) SYSERR) {
               
        /* restore mask and return the address from normal getmem */
        
        restore(mask);
        return normal_ret;
    
    }  else {

        /* if the memory could not be got from the cache or free list  */
        
        /* free all the memory in cache */
        int i = 0;
        
        /* traverse through cache */
        for (i = 0; i < 20; i++){
            
            if (cache[i].blockSize != 0) {
                
                /* traverse through the list of blocks */
                
                while(cache[i].list != NULL) {
                    
                    /* get size and address of block to be freed */
                    int size = cache[i].blockSize;
                    char *block_address = cache[i].list -> blockaddr;

                    /* Update the list */
                    cache[i].list = cache[i].list -> next;
        
                    /* free the previous block */
                    freemem(block_address, size);
        
                    /* if null reached, set the block size to 0 */
                    if (cache[i].list == NULL) {
                        cache[i].blockSize = 0;
                    }
                }

            }
        } 
	
        /* call the normal getmem after freeing the cache */
        char *ret_addr = getmem(nbytes);

        /* If we can get memmory now, return the memory */
        if (ret_addr != (char *) SYSERR) {
            restore(mask);
	        return ret_addr;    
        }

        /* If memory cannot be got, return error */
        restore(mask);
        return (char*) SYSERR;
    }

}

