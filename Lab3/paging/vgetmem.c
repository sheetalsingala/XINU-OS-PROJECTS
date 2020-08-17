/* vgetmem.c - vgetmem */

#include <conf.h>
#include <kernel.h>
#include <mem.h>
#include <proc.h>
#include <paging.h>

extern struct pentry proctab[];
/*------------------------------------------------------------------------
 * vgetmem  --  allocate virtual heap storage, returning lowest WORD address
 *------------------------------------------------------------------------
 */
WORD	*vgetmem(nbytes)
	unsigned nbytes;
{
	STATWORD ps;    
	struct	mblock	*p, *q, *leftover;
	//kprintf("\n nbytes %d ", nbytes);
	disable(ps);
	if (nbytes==0 || proctab[currpid].vmemlist.mnext== (struct mblock *) NULL) {
		restore(ps);
	//	kprintf("\n Here? %d", nbytes);
		return( (WORD *)SYSERR);
	}

	nbytes = (unsigned int) roundmb(nbytes);
	//kprintf("\n vgetmem entered  nbytes %d", nbytes);
	for (q= &proctab[currpid].vmemlist,p=proctab[currpid].vmemlist.mnext ;
	     p != (struct mblock *) NULL ;
	     q=p,p=p->mnext){
		//kprintf("\nqlen %d plen %d", q->mlen, p->mlen);
		if ( p->mlen == nbytes) {
			q->mnext = p->mnext;
			restore(ps);
			return( (WORD *)p );
		} else if ( p->mlen > nbytes ) {
			//kprintf("\nShould hit here %d", nbytes);
			leftover = (struct mblock *)( (unsigned)p + nbytes );
			q->mnext = leftover;
			leftover->mnext = p->mnext;
			leftover->mlen = p->mlen - nbytes;
	//		kprintf("\nShould hit here %d leftover %d len %d", nbytes, leftover->mlen, p->mlen);
			restore(ps);
			return( (WORD *)p );
		}
	/*struct mblock *iter = &proctab[currpid].vmemlist;
	while(iter != (WORD *)NULL){
		kprintf("\n Length %d", iter->mlen);
		iter = iter->mnext;
	}*/}
	restore(ps);
	return( (WORD *)SYSERR );
	
}


