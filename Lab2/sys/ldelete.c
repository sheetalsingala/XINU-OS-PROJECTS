/* ldelete.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * ldelete.c  --  delete a lock
 *------------------------------------------------------------------------
 */
int ldelete (int lock)
{
    STATWORD ps; 
    int	pid;
    struct lentry	*lptr;

    disable(ps);

    if (isbadlock(lock%NLOCKS) || ltable[lock%NLOCKS].lstate==LFREE || lock != (lock%NLOCKS)+ltable[lock%NLOCKS].ver*NPROC) {    /* Check if lock descriptor is valid*/
        	restore(ps);
		return(SYSERR);
	}
	int i;
	for(i=NPROC; i>0; i--){				/*All processes holding the lock */
		if(proctab[i].pstate != PRFREE && proctab[i].lacquired[lock%NLOCKS] ==1){
		proctab[i].lacquired[lock%NLOCKS] = 0;
		ltable[lock%NLOCKS].lholdprocs[i] = 0;
	}
	}
	
    lptr = &ltable[lock%NLOCKS];
	lptr->lstate = LFREE;           
	lptr->ver+=1;
	if(lptr->ver == 10){
		lptr->ver = 0;
	}
	if (nonempty(lptr->lhead)) {			/* Processes waiting on the lock */
		while( (pid=getfirst(lptr->lhead)) != EMPTY)
		  {
		    proctab[pid].plwaitret = DELETED;        /* Set pwaitret */
			proctab[pid].plock = -1;
		    ready(pid,RESCHNO);		
		  }
		resched();
	}

	restore(ps);
	return(DELETED);            /* Waiting on a lock returns DELETED and not OK */

}
