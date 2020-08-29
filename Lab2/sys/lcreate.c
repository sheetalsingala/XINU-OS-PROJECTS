/* lcreate.c - lcreate */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <q.h>
#include <stdio.h>

/*------------------------------------------------------------------------
 * lcreate.c  --  create a lock
 *------------------------------------------------------------------------
 */

int lcreate (void) 
{

    int	lock;
	int	i,j;
	STATWORD ps;
	disable(ps);
	for (i=0 ; i<NLOCKS ; i++) {
		lock=nextlock--;
		if (nextlock < 0)
			nextlock = NLOCKS-1;
		if (ltable[lock].lstate==LFREE) {
			ltable[lock].lstate = LUSED;		/*Lock descriptor used*/
            ltable[lock].lcnt = 1;				/*Lock unused*/
			ltable[lock].lprio = -1;
			ltable[lock].numreaders = 0;
			ltable[lock].ltype = LINIT;
			for(j=0; j<NPROC;j++){
				ltable[lock].lholdprocs[j] = 0;		/* No process holds lock */
			}
			restore(ps);
			return(lock+ltable[lock].ver*NLOCKS);	
		}
	}
	restore(ps);
	return(SYSERR);

}
