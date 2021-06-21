/* mymeminit.c - mymeminit */

#include <xinu.h>

/* ------------------------------------------------------------------
 *   mymeminit - initializes the cache with the entries required
 *------------------------------------------------------------------
 */
void mymeminit(

)

{
    
    int i, count;
    
    for (i = 0; i < 20; i++) {
        
        /* create new struct with blockSize 0 */
        struct myStruct *newStruct;
        newStruct = (struct myStruct*) getmem(sizeof(struct myStruct));
        
                
        newStruct->blockSize = 0;

        /* set the list to NULL, it will initialized in freemem when needed */
        newStruct->list = NULL;

        /* insert the struct into the cache */
        cache[i] = *(newStruct);
    }
}
