/* releaseall.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <sem.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * releaseall.c  --  calling process releases all locks
 *------------------------------------------------------------------------
 */

int releaseall(int nlocks, long locks)
{
    struct	pentry	*pptr;
    struct	lentry	*lptr;
    STATWORD ps; 
    unsigned long *lock = (unsigned long *)(&locks);
    int i;
    int flag=1;
    disable(ps);
    for(i=0;i<=nlocks;i++)          /*Parse the entire list of locks */
    {   
        lptr= &ltable[*lock]);
        if(proctab[currpid].lacquired[*lock] == 0 || isbadlock(*lock) || (lptr->lstate==LFREE ){ /* Return SYSERR if lock is invalid*/  
            flag = 0;
        }
        pptr = lptr->ltail;
        if(q[q[pptr].qprev].qprev == NULL){     /*Wait queue is empty */
                lptr->lcnt+=1;
                lptr->lholdprocs[currpid] = 0;
                proctab[currpid].lacquired[*lock] = 0;
                lptr->ltype = LINIT;
        }
       
        if(ltable[lock].ltype==WRITE){             /* Lock type is write, remove first waiting proc-> if its a reader
                                                    Remove all waiting readers higher than highest waiting writer*/                      
           if(q[maxReader(*lock)].qkey == q[maxWriter(*lock)].qkey ) 
                {
                    proctab[maxReader(lock)].plwaittime  = ctr1000 - proctab[maxReader(lock)].plwaittime;
                    proctab[maxWriter(lock)].plwaittime  = ctr1000 - proctab[maxWriter(lock)].plwaittime;
                    while(proctab[maxWriter(*lock)].plwaittime < proctab[maxReader(*lock)].plwaittime - 400 ){
                        /* remove reader from queue */
                        /* Does this loop end */
                        ready(deque(maxReader()), RESCHNO);
                        ltable[*lock].ltype = READ;
                        ltable[*lock].numreaders +=1;
                        proctab[currpid].lacquired[*lock] = 1;      
                        ltable[*lock].lholdprocs[currpid] = 1;  
                    }
                }
                else{                                   /* Condition when prio is not same */
                                                        /* Dequeue first proc and decide */
                        pptr = getfirst(lptr->lhead);
                        if(pptr->pltype == READ){       /* If dequeued proc is reader */
                        ready(pptr, RESCHNO);
                        ltable[*lock].ltype = READ;
                        ltable[*lock].numreaders +=1;
                        proctab[currpid].lacquired[*lock] = 1;      
                        ltable[*lock].lholdprocs[currpid] = 1; 
                        p0 = q[lptr->ltail].qprev;
                        while(q[q[p0].qprev].qprev != NULL || proctab[p0].pltype == WRITE){ /* Reach end of list or first writer */
                                ready(getfirst(sptr->sqhead), RESCHNO);
                                ltable[*lock].numreaders +=1 ;
                                proctab[currpid].lacquired[*lock] = 1;      
                                ltable[*lock].lholdprocs[currpid] = 1; 
                                
                                }
                            }
                        else{                           /* If dequeued proc is writer */
                            ready(pptr, RESCHYES);
                            ltable[*lock].ltype = WRITE;
                            proctab[currpid].lacquired[*lock] = 0;      
                            ltable[*lock].lholdprocs[currpid] = 0;

                        }
                    
            }
        }
        if(ltable[*lock].ltype==READ){
            ltable[*lock].numreaders -= 1; 
            if(ltable[*lock].numreaders != 0){       /* If number of readers is not zero */
                lptr->lholdprocs[currpid] = 0;
                proctab[currpid].lacquired[*lock] = 0;
            }
            if(ltable[*lock].numreaders  == 0){             /* Lock type is read and number of readers is zero */
                if(q[maxReader(*lock)].qkey == q[maxWriter(*lock)].qkey ) 
                {
                    proctab[maxReader(*lock)].plwaittime  = ctr1000 - proctab[maxReader(*lock)].plwaittime;
                    proctab[maxWriter(*lock)].plwaittime  = ctr1000 - proctab[maxWriter(*lock)].plwaittime;
                    while(proctab[maxWriter(*lock)].plwaittime < proctab[maxReader(*lock)].plwaittime - 400  && q[maxreader(*lock)].qkey == q[maxWriter(*lock)].qkey){
                        /* remove reader from queue */
                        /* Does this loop end */
                        ready(deque(maxReader()), RESCHNO);
                        ltable[*lock].ltype = READ;
                        ltable[*lock].numreaders +=1;
                        proctab[currpid].lacquired[*lock] = 1;      
                        ltable[*lock].lholdprocs[currpid] = 1;  
                    }
                }
                else{                                   /* Condition when prio is not same */
                                                        /* Dequeue first proc and decide */
                        pptr = getfirst(lptr->lhead);
                        if(pptr->pltype == READ){       /* If dequeued proc is reader */
                        ready(pptr, RESCHNO);
                        ltable[*lock].ltype = READ;
                        ltable[*lock].numreaders +=1;
                        proctab[currpid].lacquired[*lock] = 1;      
                        ltable[*lock].lholdprocs[currpid] = 1; 
                        p0 = q[lptr->ltail].qprev;
                        while(q[q[p0].qprev].qprev != NULL || proctab[p0].pltype == WRITE){ /* Reach end of list or first writer */
                                ready(getfirst(sptr->sqhead), RESCHNO);
                                ltable[*lock].numreaders +=1 ;
                                proctab[currpid].lacquired[*lock] = 1;      
                                ltable[*lock].lholdprocs[currpid] = 1; 
                                
                                }
                            }
                        else{                           /* If dequeued proc is writer */
                            ready(pptr, RESCHNO);
                            ltable[*lock].ltype = WRITE;
                            proctab[currpid].lacquired[*lock] = 0;      
                            ltable[*lock].lholdprocs[currpid] = 0;
                        }
                    
            }
            }
            }
            }
               

    if(flag == 0){
        restore(ps);
        return(SYSERR);
    }
   restore(ps)
   return(OK);
}


