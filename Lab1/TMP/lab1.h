/*Scheduling class*/
#define RANDOMSCHED 1 
#define LINUXSCHED 2

extern  int currsched;      /* currently running shceduling algorithm */
				/* Default value greater than 2 */

int getschedclass(void);    /* Function to retrive currently executing scheduling algorithm */
void setschedclass(int sched);
int getRandom(int val);
void reschedule(void);
int getRandomProc(int totP);
int getRandom(int val);
int getTotPrio(void);
void updateVal(int pid);
int newProc(void);
void goodness(void);
void quantum(void);
