/* receive.c - receive */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  receive  -  Wait for a message and return the message to the caller
 *------------------------------------------------------------------------
 */
umsg32	receive(void)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/

	mask = disable();
	prptr = &proctab[currpid];
	
	if (prptr->prhasmsg == FALSE) {
		prptr->prstate = PR_RECV;
		resched();		/* Block until message arrives	*/
	}
	
	msg = prptr->prmsg;		/* Retrieve message		*/
	prptr->prhasmsg = FALSE;	/* Reset message flag		*/

	/* if there is a sender process blocked */
	if ( prptr->hasblockedsender ) {
		/* reset the hasblockedsender value */
		prptr->hasblockedsender = FALSE;

		/* unblock the sender process and call resched */
		ready(prptr->blockedsender);
	}

	/* if callback is in effect */
	if (prptr -> hascb) {
		/* reset the hascb */
		prptr -> hascb = FALSE;
		
		/* call the callback function */
		uint32(* old_func) () = prptr -> cb;
		(*old_func)(msg);
	}

	restore(mask); /* Restore interrupts */
	return msg;
}

/*------------------------------------------------------------------------
 *  receivecallback - save a callback function to be used by senders
 *------------------------------------------------------------------------
 */
syscall	receivecallback(
		callback c
	)
{
	return OK;
}
