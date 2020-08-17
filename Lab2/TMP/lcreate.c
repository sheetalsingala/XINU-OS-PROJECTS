/* lcreate.c - signal */

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


/* Disable and restore ps ?? */
/* Setting Count needed ?? */
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
			ltable[lock].lstate = LUSED;
            		ltable[lock].lcnt = 1;
			ltable[lock].lprio = -1;
			ltable[lock].numreaders = 0;
			ltable[lock].ltype = LINIT;
			for(j=0; j<NPROC;j++){
				ltable[lock].lholdprocs[j] = 0;
			}
			//kprintf("\n %d lock created", lock+ltable[lock].ver*NPROC);
			restore(ps);
			return(lock+ltable[lock].ver*NPROC);
		}
	}
	restore(ps);
	return(SYSERR);

}
