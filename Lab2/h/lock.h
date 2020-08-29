/* lock.h */

#ifndef _LOCK_H_
#define _LOCK_H_

#ifndef	NLOCKS				/* set the number of locks	*/
#define	NLOCKS		50		
#endif

#define READ 1    /* Semaphore type read */
#define WRITE 2    /* Semaphore type write */
#define LINIT 3		/* Lock created but unused */
#define lockPolicy 40   /* to accomodate for ctr1000 and sleep */
#define	LFREE	0		/* this lock is free		*/
#define	LUSED	1		/* this lock is used		*/

struct	lentry	{		/* lock table entry		*/
	int lstate;		/* state of lock - free or used */
	int	lcnt;		/* count for this lock, for write type semaphore it should be 1*/
	int	lhead;		/* q index of head of list		*/
	int	ltail;		/* q index of tail of list		*/
    int ltype;  	/* lock type */
	int numreaders; 	/* Number of readers */
	int lprio; 		/* Process with highest prio waiting on the lock */
	int lholdprocs[NPROC];	/*All processes waiting on the lock */
	int ver; 		/* Version of the lock */	
};

extern	struct	lentry	ltable[];
extern	int	nextlock;
extern int pchanged;

unsigned long ctr1000;
#define	isbadlock(l) (l<0 || l>=NLOCKS)

int maxReader(int ldes1);
int maxWriter(int ldes1);
int maxProc(int ldes1);
void updatePrio(int ldes1);
void releaseAll(int pid);
void  deqenq(int pid, int pr);
#endif
