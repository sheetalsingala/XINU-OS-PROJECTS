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
//    int oldP = lptr->lprio;
    struct lentry *lptr;
    lptr = &ltable[ldes1%NLOCKS];
    int oldP = lptr->lprio;
    p0 = q[lptr->ltail].qprev;
    lptr->lprio = 0; 
    //kprintf("\n Highest proc %d now %d", p0, proctab[p0].pprio);
    while(p0 != lptr->lhead){       /* If queue empty */
	    //kprintf("Proc %d prio %d", p0, proctab[p0].pprio);
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
 	  //		kprintf("\nI'm here and mP is %d %d", mP, pchanged);
    	//		kprintf("val %d", ltable[ldes1].lholdprocs[48]);
    if(pchanged){
        int j;
        for(j=NPROC; j>0;j--){
        //		kprintf("Holdprocs %d pprio %d", ltable[ldes1].lholdprocs[j], proctab[j].pprio);
           if(proctab[j].pstate != PRFREE){
            if(ltable[ldes1%NLOCKS].lholdprocs[j] == 1 && (mP >proctab[j].pprio) ){                    
                            proctab[j].pinh = mP;
			    deqenq(j, proctab[j].pinh);
	//		kprintf("Here too!");
            }
            if(ltable[ldes1%NLOCKS].lholdprocs[j] == 1 &&  mP <= proctab[j].pprio){
                proctab[j].pinh = -1;
		deqenq(j, proctab[j].pprio);
            }            
        //		deqenq(j, max(proctab[j].pinh, proctab[j].pprio)); 
        	//kprintf("\n plock = %d", proctab[j].plock);
		//kprintf("\n maxProc %d", maxProc(ltable[proctab[j].plock].lprio));
		//kprintf("\n mac Val = %d", max(proctab[j].pinh, proctab[j].pprio));         
                if((proctab[j].plock !=-1 && ltable[ldes1%NLOCKS].lholdprocs[j] == 1 && ltable[proctab[j].plock].lprio != max(proctab[j].pinh, proctab[j].pprio))){
                  updatePrio(proctab[j].plock);
		//	kprintf("\nhi! %d %d", j,proctab[j].plock);
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
     //       updatePrio(i);
        }
    }
}

void  deqenq(int pid, int pr){
    if(proctab[pid].pstate == PRREADY){         /* PLWAIT and PRWAIT ?*/
        dequeue(pid);
        insert(pid,rdyhead,pr);

    }
   


}
