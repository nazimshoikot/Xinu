/* limit.c - limit */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  tlimit -  limits the number of time slices for a process
 *------------------------------------------------------------------------
 */
syscall	tlimit (
	pid32		pid,		/* process ID		*/
	int32		limit		/* time slice limit */	
	)
{
	
	if (currpid == pid) {
 		limit = limit + 1;	
    }
    (&proctab[pid])->nslices = limit;

}

