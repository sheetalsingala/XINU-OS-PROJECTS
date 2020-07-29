#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab1.h>
unsigned long currSP;   /* REAL sp of current process */
extern int ctxsw(int, int, int, int);

/*------------------------------------------------------------------------------------
 *  *  *  * resched  --  reschedule process
 *  *  *  * Types of scheduling  - Random scheduling, linux like scheduling
 *  *  *  * and default xinu scheduling(highest priority first)
 *  *  *  *------------------------------------------------------------------------
 */
int resched()
{
    register struct	pentry	*optr;	/* pointer to old process entry */
    register struct	pentry	*nptr;	/* pointer to new process entry */

    int p0, totPrio;

    if (currsched == RANDOMSCHED)                       /* If scheduling algorithm selected is random */
    {
        if ( (optr= &proctab[currpid])->pstate == PRCURR)   /* Inserting the current process into the ready queue */
        {
            optr->pstate = PRCURR;
            insert(currpid,rdyhead,optr->pprio);
        }
        totPrio = getTotPrio();
        if (totPrio == 0 )  /* Null Process, getRandom takes in totPrio-1, so better to have a separate condition for null*/
        {
            nptr = &proctab[ (currpid = getlast(rdytail)) ];
            nptr->pstate = PRCURR;		/* mark it currently running	*/
            #ifdef	RTCLOCK
                preempt = QUANTUM;		/* reset preemption counter	*/
            #endif

            ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
            return OK;
        }

        p0 = getRandomProc(totPrio);
        nptr = &proctab[(currpid = p0)];
        nptr->pstate = PRCURR;		/* mark it currently running	*/
        #ifdef	RTCLOCK
            preempt = QUANTUM;		/* reset preemption counter	*/
        #endif

        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
        return OK;
    }
    else if (currsched == LINUXSCHED)                       /* If scheduling algorithm selected is linux like */
    {
        int p1;
	    if ( (optr= &proctab[currpid])->pstate == PRCURR)
        {
            optr->pstate = PRREADY;
            insert(currpid,rdyhead,optr->pprio);
        }

	    updateVal(currpid);                                 /* Update goodness and counter for a process when resched() is called */                                                                                                     /* Update goodness, counter of current process */

	    p1 = newProc();                                      /* Returns process with highest goodness value */

        if (p1 == 0 && q[q[rdytail].qprev].qprev == rdyhead)   /* Null Process - Quantum to what value */
        {
            nptr = &proctab[ (currpid = getlast(rdytail)) ];
            nptr->pstate = PRCURR;		/* mark it currently running	*/
            #ifdef	RTCLOCK
                preempt = QUANTUM;		/* reset preemption counter	*/
            #endif

            ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

            return OK;
        }
        if (p1 == 0)                                           /* New epoch */
        {
		    quantum();                                        /* Recalculate goodness */
            p1 = newProc();                                     /* find a new proc */
        }

        int flag = 0;
	    if(currpid == p1){
		flag = 1;
}
        nptr = &proctab[ (currpid = dequeue(p1)) ];
        nptr->pstate = PRCURR;		                            /* mark it currently running	*/
        if(proctab[p1].counter == 0 && proctab[p1].quantum < QUANTUM)
        {
            preempt = proctab[p1].quantum;
        }
        else if(proctab[p1].counter < QUANTUM)
        {
            preempt = proctab[p1].counter;    /* Set the preempt value to counter */
        }
        else
        {
            preempt = QUANTUM;
        }
	if(flag){
            return OK;}
	 ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);
            return OK;
    }

    else                            /* Default Xinu scheduler */
    {
	/* no switch needed if current process priority higher than next*/
        if ( ( (optr= &proctab[currpid])->pstate == PRCURR) &&
        (lastkey(rdytail)<optr->pprio)) {
            return(OK);
        }
        /* force context switch */
        if (optr->pstate == PRCURR) {
            optr->pstate = PRREADY;
            insert(currpid,rdyhead,optr->pprio);
        }
        /* remove highest priority process at end of ready list */
        nptr = &proctab[ (currpid = getlast(rdytail)) ];
        nptr->pstate = PRCURR;		/* mark it currently running	*/
        #ifdef	RTCLOCK
            preempt = QUANTUM;		/* reset preemption counter	*/
        #endif
        ctxsw((int)&optr->pesp, (int)optr->pirmask, (int)&nptr->pesp, (int)nptr->pirmask);

        /* The OLD process returns here when resumed. */
        return OK;
    }
}



