#include <conf.h>
#include <kernel.h>
#include <proc.h>
#include <lock.h>
#include <sem.h>
#include <stdio.h>

#define DEFAULT_LOCK_PRIO 20

void reader (char *msg, int lck)
{
        kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, READ, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (3);
        kprintf ("  %s: to release lock\n", msg);
	releaseall (1, lck);
}

void writer (char *msg, int lck)
{
	kprintf ("  %s: to acquire lock\n", msg);
        lock (lck, WRITE, DEFAULT_LOCK_PRIO);
        kprintf ("  %s: acquired lock, sleep 3s\n", msg);
        sleep (3);
        kprintf ("  %s: to release lock\n", msg);
        releaseall (1, lck);
}

void semprocs (char *msg, int sem)
{
	kprintf("  %s: to acquire sem\n", msg);
        wait(sem);
        kprintf ("  %s: acquired sem, sleep 3s\n", msg);
        sleep (3);
        kprintf ("  %s: to release sem\n", msg);
        signal(sem);
}

void proc(char *msg)
{
        int i,j = 0;
        sleep(2);
        kprintf("  ");
        for(;i<40;i++){
                //int j = 0;
                kprintf("L");
        }
	kprintf("\n");
}



void lockstest()
{
    int lck;
    int pid1, pid2, pid3;

    lck  = lcreate ();  /* create a lock*/

    /* Writer b acquires the lock and sleeps, reader a and writer c then try to acquire the lock(A<B<C)*/

    pid1 = create(reader, 2000, 50, "reader a", 2, "Reader A", lck);
    pid2 = create(writer, 2000, 30, "writer b", 2, "Writer B", lck);
    pid3 = create(proc, 2000, 40,"dummy", 1, "L");

    resume(pid2);
    sleep(1);
    resume(pid1);
    resume(pid3);
    kprintf("  $$$Prio of writer is %d$$$\n", getprio(pid2));
    sleep(7);
   
}

void semstest()
{
	int sem;
	int pid1, pid2, pid3;
	
	sem = screate(1);	/* create a sem */

	pid1 = create(semprocs, 2000, 50, "reader a", 2, "Reader A", sem);
	pid2 = create(semprocs, 2000, 30, "writer b", 2, "Writer B", sem);
	pid3 = create(proc, 2000, 40,"dummy", 1, "L");

	resume(pid2);
	sleep(1);
	resume(pid1);
	resume(pid3);
	kprintf("  $$$Prio of writer is %d$$$\n", getprio(pid2));
	sleep(7);
	
}
int task1()
{	kprintf("\n  ---LOCK TEST ---\n");
	lockstest();
	kprintf("\n  ---SEM TEST --- \n");
        semstest();
	sleep(2);
	shutdown();

}

