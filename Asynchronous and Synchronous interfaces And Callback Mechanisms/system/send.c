/* send.c - send, sendblk */

#include <xinu.h>

/*------------------------------------------------------------------------
 *  send  -  Pass a message to a process and start recipient if waiting
 *------------------------------------------------------------------------
 */
syscall	send(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* Contents of message		*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prhasmsg) {
		restore(mask);
		return SYSERR;
	}
	prptr->prmsg = msg;		/* Deliver message		*/
	prptr->prhasmsg = TRUE;		/* Indicate message is waiting	*/

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	restore(mask);		/* Restore interrupts */
	return OK;
}

/*------------------------------------------------------------------------
 *  sendblk  -  Pass a message to a process and block until it is received
 *------------------------------------------------------------------------
 */
syscall	sendblk(
	  pid32		pid,		/* ID of recipient process	*/
	  umsg32	msg		/* Contents of message		*/
	)
{	
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *receiving_prptr;		/* Ptr to process's table entry	*/
	struct procent *curr_prptr; 			/* Ptr to current process */
	
	
	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	curr_prptr = &proctab[currpid];
	receiving_prptr = &proctab[pid];  /* pointer to receiving process */
	if (receiving_prptr->prhasmsg) {
		restore(mask);
		return SYSERR;
	}
	
	receiving_prptr->prmsg = msg;		/* Deliver message		*/
	receiving_prptr->prhasmsg = TRUE;		/* Indicate message is waiting	*/

	/* If recipient waiting or in timed-wait make it ready */

	if (receiving_prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (receiving_prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	} else {

		/* record the process ID of the sending process in the recipient's process table */ 
		receiving_prptr->hasblockedsender = TRUE;
		receiving_prptr->blockedsender = currpid;

		/* set the state of the sending process to PR_SENDING and call resched */
		curr_prptr->prstate = PR_SENDING;
		resched();
	}
	
	restore(mask);		/* Restore interrupts */

	return OK;
}

/*------------------------------------------------------------------------
 *  sendcallback  -  Pass a message to a process using its callback function
 *------------------------------------------------------------------------
 */
syscall	sendcb(
		pid32	pid,		/* ID of recipient process	*/
		umsg32	msg,		/* Contents of message		*/
		uint32 (* callback) (umsg32)	/* Callback pointer			*/
	)
{
	intmask	mask;			/* Saved interrupt mask		*/
	struct	procent *prptr;		/* Ptr to process's table entry	*/

	mask = disable();
	if (isbadpid(pid)) {
		restore(mask);
		return SYSERR;
	}

	prptr = &proctab[pid];
	if (prptr->prhasmsg) {
		restore(mask);
		return SYSERR;
	}
	prptr->prmsg = msg;		/* Deliver message		*/
	prptr->prhasmsg = TRUE;		/* Indicate message is waiting	*/

	/* set the callback bit and store the callback function pointer */
	prptr -> cb = callback;		/* store the callback function pointer */

	uint32( * check_func) () = prptr -> cb;
	
	/* if the callback function has not been successfully stored in the
	process table of the recipient process, return SYSERR */
	if (check_func != callback) {
		restore(mask);		/* Restore interrupts */
		return SYSERR;
	}

	prptr -> hascb = TRUE; 	/* indicate tht callback is in effect */
	

	/* If recipient waiting or in timed-wait make it ready */

	if (prptr->prstate == PR_RECV) {
		ready(pid);
	} else if (prptr->prstate == PR_RECTIM) {
		unsleep(pid);
		ready(pid);
	}
	
	restore(mask);		/* Restore interrupts */
	return OK;
}
