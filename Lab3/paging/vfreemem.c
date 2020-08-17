/* vfreemem.c - vfreemem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>
extern struct pentry proctab[];
/*------------------------------------------------------------------------
 *  vfreemem  --  free a virtual memory block, returning it to vmemlist
 *------------------------------------------------------------------------
 */
SYSCALL	vfreemem(block, size)
	struct	mblock	*block;
	unsigned size;
{
	STATWORD ps;    
	struct	mblock	*p, *q;
	unsigned top;
	disable(ps);
	//kprintf("\nvfreemem %d", size);
	if (size==0 || (unsigned)block<(unsigned)(proctab[currpid].vhpno * NBPG) || (unsigned)block>(unsigned)((proctab[currpid].vhpno+proctab[currpid].vhpnpages)*NBPG)){
		
		restore(ps);//kprintf("\nReturning error in the first block %lu maxaddr %lu pages %d", (unsigned)block, (unsigned)((BASEHEAP+proctab[currpid].vhpno)*NBPG), proctab[currpid].vhpno);
		return(SYSERR);}
	size = (unsigned)roundmb(size);
//	disable(ps);
	for( q= &proctab[currpid].vmemlist, p = proctab[currpid].vmemlist.mnext;
	     p != (struct mblock *) NULL && p < block ;
	     q=p,p=p->mnext )
		;
	//kprintf("\nTraversing done");
	if (((top=q->mlen+(unsigned)q)>(unsigned)block && q!= &proctab[currpid].vmemlist) ||
	    (p!=NULL && (size+(unsigned)block) > (unsigned)p )) {
	//	kprintf("\nReturning syserr from vfreemem");
		restore(ps);
		return(SYSERR);
	}
	if ( q!= &proctab[currpid].vmemlist && top == (unsigned)block )
	//		kprintf("\nHere");
			q->mlen += size;
	else {
	//	kprintf("\nThere");
		block->mlen = size;
		block->mnext = p;
		q->mnext = block;
		q = block;
	}
	if ( (unsigned)( q->mlen + (unsigned)q ) == (unsigned)p) {
	//	kprintf("\n and here");
		q->mlen += p->mlen;
		q->mnext = p->mnext;
	}
	//kprintf("\nReturning OK from vfreemem");
	restore(ps);
	return(OK);
}
