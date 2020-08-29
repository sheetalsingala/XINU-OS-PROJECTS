/* linit.c - linit */

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
    int i=0;
    struct	lentry	*lptr;
    while(i<NLOCKS) {	/* initialize locks */
		(lptr = &ltable[i])->lstate = LFREE;
		lptr->lhead = newqueue();
		lptr->ltail = 1 + lptr->lhead;
		lptr->ver = 0;
		i++;
	}
return(OK);
}
