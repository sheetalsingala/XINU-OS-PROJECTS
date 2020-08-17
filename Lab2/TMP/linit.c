/* linit.c - signal */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>
#include <stdio.h>
#include <lock.h>

/*------------------------------------------------------------------------
 * linit.c  --  initialize a lock
 *------------------------------------------------------------------------
 */


SYSCALL linit(void)
{	
//	kprintf("linit()");
    int i=0;
    struct	lentry	*lptr;
	//kprintf("%d", NLOCKS);
    while(i<NLOCKS) {	/* initialize locks */
		(lptr = &ltable[i])->lstate = LFREE;
		lptr->lhead = newqueue();
		lptr->ltail = 1 + lptr->lhead;
		lptr->ver = 0;
		i++;

	}
//int jk = newqueue();
return(OK);
}
