/* chprio.c - chprio */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * chprio  --  change the scheduling priority of a process
 *------------------------------------------------------------------------
 */
SYSCALL chprio(int pid, int newprio)
{
	STATWORD ps;    
	struct	pentry	*pptr;

	disable(ps);
	if (isbadpid(pid) || newprio<=0 ||
	    (pptr = &proctab[pid])->pstate == PRFREE) {
		restore(ps);
		return(SYSERR);
	}
	if(pptr->pinh != -1 && newprio >= pptr->pinh){
		pptr->pinh = -1;
	}
	pptr->pprio = newprio;
	if(proctab[pid].plock != -1){
		updatePrio(proctab[pid].plock);
	}
	restore(ps);
	return(newprio);
}
