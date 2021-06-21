/* recvclr.c - recvclr */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  recvclr  -  Clear incoming message, and return message if one waiting
 *------------------------------------------------------------------------
 */
umsg32	recvclr(void)
{	
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/
	umsg32	msg;			/* Message to return		*/
	
	mask = disable();
	prptr = &proctab[currpid];
	
	if (prptr->prhasmsg == TRUE) {
		msg = prptr->prmsg;	/* Retrieve message		*/
		prptr->prhasmsg = FALSE;/* Reset message flag		*/

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
			kprintf("The message parameter: %d\n", msg);
			(*old_func)(msg);
		}

	} else {
		msg = OK;
	}
	
	restore(mask);
	return msg;
}
