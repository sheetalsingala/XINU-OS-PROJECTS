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
/* To do - check if at any time you need to return OK */
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
	/*int j;
	for(j=0;j<NPROC; j++){									/* Make all procs holding lock 0 */
	/*	if(ltable[lock].lholdprocs[j] == 1){
			proctab[j].lacquired[lock] = 0 ;
		}
	}*/
	int i;
	for(i=NPROC; i>0; i--){
		if(proctab[i].pstate != PRFREE && proctab[i].lacquired[lock%NLOCKS] ==1){
		proctab[i].lacquired[lock%NLOCKS] = 0;
		ltable[lock%NLOCKS].lholdprocs[i] = 0;
}}
	

    	lptr = &ltable[lock%NLOCKS];
	lptr->lstate = LFREE;           /* Make lock free */
	lptr->ver+=1;
	if(lptr->ver == 10){
		lptr->ver = 0;}
	//kprintf("\n Lock deleted");
	if (nonempty(lptr->lhead)) {
		while( (pid=getfirst(lptr->lhead)) != EMPTY)
		  {
		    proctab[pid].plwaitret = DELETED;        /* Set pwaitret */
			proctab[pid].plock = -1;
		    ready(pid,RESCHNO);
			
		  }
		resched();
	}

	restore(ps);
	return(DELETED);            /* return DELETED */

}
