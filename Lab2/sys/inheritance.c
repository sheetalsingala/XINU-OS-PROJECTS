/* inheritance.c - priority inheritance */
#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>

int maxProc(int ldes1)      /* return max prio of proc in waiting queue, takes care of transitivity */ 
{
    int  p0;
    struct lentry *lptr;
    lptr = &ltable[ldes1%NLOCKS];
    int oldP = lptr->lprio;
    p0 = q[lptr->ltail].qprev;
    lptr->lprio = 0; 
    while(p0 != lptr->lhead){       /* If queue empty */
            if(lptr->lprio < proctab[p0].pinh){
                lptr->lprio = proctab[p0].pinh;
            }
            if(lptr->lprio < proctab[p0].pprio){
                lptr->lprio = proctab[p0].pprio;
            }            
            p0 = q[p0].qprev;
    }
    if(oldP != lptr->lprio){
        pchanged = 1;
    } 
    return(lptr->lprio);
}

void updatePrio(int ldes1){
    int mP;
    struct lentry *lptr;
    lptr = &ltable[ldes1%NLOCKS];
    mP = maxProc(ldes1);
    if(pchanged){
        int j;
        for(j=NPROC; j>0;j--){
           if(proctab[j].pstate != PRFREE){
            if(ltable[ldes1%NLOCKS].lholdprocs[j] == 1 && (mP >proctab[j].pprio) ){                    
                            proctab[j].pinh = mP;
			                deqenq(j, proctab[j].pinh);
            }
            if(ltable[ldes1%NLOCKS].lholdprocs[j] == 1 &&  mP <= proctab[j].pprio){
                proctab[j].pinh = -1;
		        deqenq(j, proctab[j].pprio);
            }             
                if((proctab[j].plock !=-1 && ltable[ldes1%NLOCKS].lholdprocs[j] == 1 && ltable[proctab[j].plock].lprio != max(proctab[j].pinh, proctab[j].pprio))){
                    updatePrio(proctab[j].plock);
                    pchanged = 0;
                }
            }
        }  
    }
}

void releaseAll(int pid){
    int i;
    struct pentry *pptr;
    pptr = &proctab[pid];
    for(i = NLOCKS;i>0; i--){
        if(proctab[pid].lacquired[i] ==1){
            releaseall(1,i);
        }
    }
}

void  deqenq(int pid, int pr){
    if(proctab[pid].pstate == PRREADY){         /* PLWAIT and PRWAIT ?*/
        dequeue(pid);
        insert(pid,rdyhead,pr);
    }
}
