/* ready.c - ready */

#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <q.h>

/*------------------------------------------------------------------------
 * ready  --  make a process eligible for CPU service
 *------------------------------------------------------------------------
 */
int ready(int pid, int resch)
{
	register struct	pentry	*pptr;

	if (isbadpid(pid))
		return(SYSERR);
	pptr = &proctab[pid];
	pptr->pstate = PRREADY;
	if(proctab[pid].pinh!=-1 && proctab[pid].pinh > proctab[pid].pprio){
		insert(pid, rdyhead, pptr->pinh);}
	else{
	insert(pid,rdyhead,pptr->pprio);
	if(proctab[pid].pinh != -1){
	proctab[pid].pinh = -1;}
}
	if (resch)
		resched();
	return(OK);
}
