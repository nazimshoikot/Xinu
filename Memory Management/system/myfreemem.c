/* myfreemem.c - myfreemem */

#include <xinu.h>

struct myStruct cache[20];

/*-----------------------------------------------------------------------
 * myfreemem - puts memory freed from processes into a cache list or calls
 * normal freemem appropriately
 *-----------------------------------------------------------------------
 */
syscall myfreemem(
    char    *blkaddr,   /* pointer to memory block to be freed */
    uint32  nbytes      /* size of block in bytes  */
)
{
    intmask	mask;			/* Saved interrupt mask		*/
    mask = disable();

    
    /* check if the cache is full */
    int i;                              
    int cache_count = 0;                /* counter to check if cache is full */
    int exist_count;                /* counter to check if block size exists in cache */
    
    /* check how many entries are there in the cache */
    for (i = 0; i < 20; i++) {
        if (cache[i].blockSize != 0){
            cache_count++;
        }         
    }

    /* if the cache is not full */
    if (cache_count < 20) {
        
        int exist_count = 0;
        
        for (i = 0; i < 20; i++){

            /* if size exists, create a myBlockList* structure and 
            add it to the existing list of that block size */
            if (cache[i].blockSize == nbytes){

                struct myBlockList *oldHead = cache[i].list;
                
                struct myBlockList *newHead = (struct myBlockList *) 
                                    getmem(sizeof(struct myBlockList));                
                
                newHead->blockaddr = blkaddr;
                newHead->next = oldHead;

                cache[i].list = newHead;
                exist_count++;

                break;          
            }
        }
        
        /* If no entry of this block size exists, make a new entry */
        if (exist_count == 0) {

            for (i = 0; i < 20; i++){
                if (cache[i].blockSize == 0) {
                                    
                    cache[i].blockSize = nbytes;
        
                    /* create new list */
                    struct myBlockList *newList;
                    newList = (struct myBlockList*)                                     getmem(sizeof(struct myStruct));
    
                    newList -> blockaddr = blkaddr;
                    newList -> next = NULL;
                        
                    cache[i].list = newList;
        
                    break;
                }
            }
        }
    } else {
        /* use original freemem to return to the freelist */
        freemem(blkaddr, nbytes);   
    }
    

    restore(mask);
	return OK;
}
