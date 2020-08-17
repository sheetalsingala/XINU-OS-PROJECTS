/* kill.c - kill */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <q.h>
#include <stdio.h>
#include <paging.h>

/*------------------------------------------------------------------------
 * kill  --  kill a process and remove it from the system
 *------------------------------------------------------------------------
 */
SYSCALL kill(int pid)
{
	STATWORD ps;    
	struct	pentry	*pptr;		/* points to proc. table for pid*/
	int	dev;
//	kprintf("\nKilled %d state %d", pid, proctab[pid].pstate);
	disable(ps);
	if (isbadpid(pid) || (pptr= &proctab[pid])->pstate==PRFREE){
		        if(proctab[pid].vhpno != 0){
                int i = proctab[pid].store;
                free_bsm(i);
                pptr->vhpno = 0;
                pptr->store = -1;
                pptr->vhpnpages = 0;
        }
        int i;
        for(i = 0;i<BSELIMIT;i++){
                if(bsm_tab[i].bs_pid[pid] == 1){
                        bsm_unmap(pid, bsm_tab[i].bs_vpno[pid]);      
			bsm_unmapped(i, pid);            
                }
        }
        switch_writeouts(pid);
                int pageDir = (pptr->pdbr / NBPG) - FRAME0;
                free_frm(pageDir);
		restore(ps);
		return(SYSERR);
	}
	if (--numproc == 0)
		xdone();

	dev = pptr->pdevs[0];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->pdevs[1];
	if (! isbaddev(dev) )
		close(dev);
	dev = pptr->ppagedev;
	if (! isbaddev(dev) ){
		close(dev);
	}
	
        if(proctab[pid].vhpno != 0){   
                int i = proctab[pid].store;
                free_bsm(i);
  //              kprintf("\nFreed %d", i);
                pptr->vhpno = 0;
                pptr->store = -1;
                pptr->vhpnpages = 0;
        }
        int i;
//	kprintf("\nEntering unmap");
        for(i = 0;i<BSELIMIT;i++){              
                if(bsm_tab[i].bs_pid[pid] == 1){
                        bsm_unmap(pid, bsm_tab[i].bs_vpno[pid]);
			bsm_unmapped(i, pid);
                }
        }

	//kprintf("\nHere?");
	switch_writeouts(pid);
                int pageDir = (pptr->pdbr / NBPG) - FRAME0;
                free_frm(pageDir);

	send(pptr->pnxtkin, pid);

	freestk(pptr->pbase, pptr->pstklen);
	switch (pptr->pstate) {

	case PRCURR:	//kprintf("\nCurrent");
			pptr->pstate = PRFREE;	/* suicide */
			resched();
		//	pptr->pdbr = 0;

	case PRWAIT:	semaph[pptr->psem].semcnt++;

	case PRREADY:	dequeue(pid);
			pptr->pstate = PRFREE;
			break;

	case PRSLEEP:
	case PRTRECV:	unsleep(pid);
						/* fall through	*/
	default:	pptr->pstate = PRFREE;
	}
//	kprintf("\nDone with resched");
	/* Freeing all frames */
	/* Remove all entries from BSM */
	/* Check if there's a virtual heap */
//	kprintf("Killing 2 %d", pid);
//	if(proctab[pid].vhpno != 0){	/* There exists a virtual heap */
/*		int i = proctab[pid].store;
		free_bsm(i);
		kprintf("\nFreed %d", i);
		pptr->vhpno = 0;
		pptr->store = -1;
		pptr->vhpnpages = 0;
	}
	int i;
	for(i = 0;i<BSELIMIT;i++){	
		if(bsm_tab[i].bs_pid[pid] == 1){
			bsm_unmap(pid, bsm_tab[i].bs_vpno[pid]);
		}
	}*/
	
	/* Delete page directory */
/*		int pageDir = (pptr->pdbr / NBPG) - FRAME0;
		pptr->pdbr = 0;
		free_frm(pageDir);


	kprintf("\nDone killing");*/
	restore(ps);
	return(OK);
}
