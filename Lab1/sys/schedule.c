#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <lab1.h>

int currsched = 3;
int getschedclass(void)
{
    return(currsched) ;  /* Returns current scheduling algorithm */
}

void setschedclass(int sched)
{
    currsched = sched;      /* Set currsched to the current scheduling algorithm value */
}

int getRandom(int val)
{
    return(rand()%(val+1));     /*Get random value between 0 to sum of all priorities */
}

int getRandomProc(int totP)
{
    int random, p0;
    random = getRandom(totP-1);   /* Random number between 0 and totPrio -1 */
    p0 = q[rdytail].qprev;
    while(q[p0].qkey < random)      /* Traverse list as long as random > process prio */
    {
        random = random - q[p0].qkey;
        p0=q[p0].qprev;
    }
    int prev = q[p0].qprev;       /* Removing p0 of the list */
    int next = q[p0].qnext;
    q[prev].qnext = next;
    q[next].qprev = prev;

    return(p0);
}
int getTotPrio(void)
{
    int p0;
    int totPrio = 0;
    p0 = q[rdyhead].qnext;                              /* First element from the head in the queue */
    while (q[q[p0].qnext].qnext != NULL)                /* End of ready queue*/
        {
            totPrio = totPrio + q[p0].qkey;
            p0 = q[p0].qnext;
        }
    return (totPrio);                                   /* Return total prio */
}

void goodness(void)                 /*Dummy Function that sets the goodness value - now done by quantum()*/ 
                                    /* Once quantum is used up, set goodness as 0 */
{
    int i;

    for(i=0; i<NPROC; i++)
    {
        if(proctab[i].pstate != PRFREE)
        {
            proctab[i].goodness = proctab[i].counter + proctab[i].pprio;
	   // kprintf("\n Goodness %d", proctab[i].goodness);
	    //kprintf("\n pprio %d", proctab[i].pprio); 
        }

    }
}

void quantum(void)                  /* Function that sets the quantum value or change this function to get quantum*/ 
{
    int i;
    for(i=0; i<NPROC; i++)
    {
        if(proctab[i].pstate != PRFREE)
        {
            
                proctab[i].quantum = (int)(proctab[i].counter/2) + proctab[i].pprio;
		proctab[i].counter = proctab[i].quantum;
		proctab[i].goodness = proctab[i].counter + proctab[i].pprio;
	//	kprintf("\nInitial Quantum %d counter %d", proctab[i].quantum, proctab[i].counter);
        }
    }
}

int newProc(void)
{
    int p0 = q[rdytail].qprev; 
    int max = 0;    /* How to check if a new epoch is needed ? If new epoch returns NULL */
    int maxP = 0;	
    while ( p0 != rdyhead )  /* Traverse till end of ready queue */
    {
//	kprintf("\nGoodness  %d %d", p0,proctab[p0].goodness );
        if (max < proctab[p0].goodness)     /* safe to assume that goodness is zero when counter is zero? */
        {
            max = proctab[p0].goodness;
	    maxP = p0;
	  //  kprintf("\n max %d p0 %d", max, p0);
        }

	p0=q[p0].qprev;
    }
//	kprintf("\npid: %d", maxP);
	//kprintf("\n Process state %d", proctab[max].pstate);
    return(maxP);        /* Returns process ID with highest goodness */
}

void updateVal(int pid)
{
             int oldCount = proctab[pid].counter;
	 	
	     if(preempt <= 0)
		{//	kprintf("\n Proc still exists");
			proctab[pid].counter = proctab[pid].counter - QUANTUM;
		}
	     else
		{     //  kprintf("\n proctab[pid].counter, %d  preempt %d", proctab[pid].counter, preempt);
			proctab[pid].counter = proctab[pid].counter - QUANTUM + preempt;
		}
             /* Update counter value of current process*/
             if(proctab[pid].counter <= 0)        /* Processes with used up quantum */
            {
                proctab[pid].goodness = 0;
		proctab[pid].counter = 0;
            }
            else
            {
                proctab[pid].goodness = proctab[pid].goodness + proctab[pid].counter - oldCount;
            }
		//kprintf("\n Updated state %d", proctab[pid].pstate);
//		if(pid){
//		kprintf("\n Updated counter %d %d", pid, proctab[pid].counter);
//		kprintf("\n Updated gooodness %d %d",pid, proctab[pid].goodness); 
}
