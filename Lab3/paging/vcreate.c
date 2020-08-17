/* vcreate.c - vcreate */
    
#include <conf.h>
#include <i386.h>
#include <kernel.h>
#include <proc.h>
#include <sem.h>
#include <mem.h>
#include <io.h>
#include <paging.h>

/*
static unsigned long esp;
*/

LOCAL	newpid();
/*------------------------------------------------------------------------
 *  create  -  create a process to start running a procedure
 *------------------------------------------------------------------------
 */
SYSCALL vcreate(procaddr,ssize,hsize,priority,name,nargs,args)
	int	*procaddr;		/* procedure address		*/
	int	ssize;			/* stack size in words		*/
	int	hsize;			/* virtual heap size in pages	*/
	int	priority;		/* process priority > 0		*/
	char	*name;			/* name (for debugging)		*/
	int	nargs;			/* number of args that follow	*/
	long	args;			/* arguments (treated like an	*/
					/* array in the code)		*/
{	
	unsigned long	savsp, *pushsp;
	STATWORD 	ps;    
	int		pid;		/* stores new process id	*/
	struct	pentry	*pptr;		/* pointer to proc. table entry */
	int		i;
	unsigned long	*a;		/* points to list of args	*/
	unsigned long	*saddr;		/* stack address		*/
	int		INITRET();

	disable(ps);
	int p;
	p = create(procaddr,ssize,priority,name,nargs,args);	/*create using create.c */

	/* This section provides a private heap, perform only when hsize is greater than 0 */
	if(hsize > 0 && hsize < 128){
		int bsmentry, retVal;
		retVal = get_bsm(&bsmentry);
		//kprintf("\nBSMENTRY %d status %d", bsmentry, bsm_tab[bsmentry].bs_status);
		if(retVal == SYSERR){
			restore(ps);
		//	kprintf("\nNo space for private heap");
			return(SYSERR);
		}			/* Gives the first free BSM */
		if(bsmentry < 0 || bsmentry >= BSELIMIT){
		//	kprintf("\nNo free entry. Cannot allocate heap");
			restore(ps);
			return(SYSERR);
		}
		//kprintf("\nBSMENTRY %d", bsmentry);
		proctab[p].store = bsmentry;
		proctab[p].vhpnpages = hsize;
		proctab[p].vhpno = BASEHEAP ;
		proctab[p].vmemlist.mnext = BASEHEAP * NBPG;
		bsm_tab[bsmentry].bs_private = PRIVATE; /* set the BS as private */
 		bsm_tab[bsmentry].bs_status = BSM_MAPPED;
		struct mblock *heapmem = BACKING_STORE_BASE + bsmentry * BACKING_STORE_UNIT_SIZE; /* Point to the first page of allocated BS */
		heapmem->mlen = hsize * NBPG;
		//kprintf("\nHeap size %d", heapmem->mlen);
		heapmem->mnext =  (struct mblock *) NULL;		
		bsm_map(p, BASEHEAP, bsmentry, hsize);	/* Create a map entry */
		//kprintf("\nCreated heap!");	
	}
	restore(ps);

	return(p);

}

/*------------------------------------------------------------------------
 * newpid  --  obtain a new (free) process id
 *------------------------------------------------------------------------
 */
LOCAL	newpid()
{
	int	pid;			/* process id to return		*/
	int	i;

	for (i=0 ; i<NPROC ; i++) {	/* check all NPROC slots	*/
		if ( (pid=nextproc--) <= 0)
			nextproc = NPROC-1;
		if (proctab[pid].pstate == PRFREE)
			return(pid);
	}
	return(SYSERR);
}
