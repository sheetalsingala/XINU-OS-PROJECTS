//
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

// constant definitions
#define READERS 4
#define WRITERS 4
#define READS 4
#define WRITES 4

//global variables declaration
unsigned int sharedValue = 0;
pthread_mutex_t sharedMemLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t readPhase = PTHREAD_COND_INITIALIZER;
pthread_cond_t writePhase = PTHREAD_COND_INITIALIZER;
int readers =0;
int waitingReaders = 0;



void *writerMain ( void *threadArg)
{
    int id = *((int*)threadArg);
    int i=0; 
    int numreaders=0;

    for(i=0;i<WRITES;i++)
    {
        usleep(1000*(random() % READERS+WRITERS));

        pthread_mutex_lock(&sharedMemLock);
        while(readers!=0)
        {
            pthread_cond_wait(&writePhase,&sharedMemLock);
        }
        readers=-1;
        numreaders=readers;
        pthread_mutex_unlock(&sharedMemLock);

         fprintf(stdout, "[w%d] writing %u* [readers: %2d]\n", id, ++sharedValue, numreaders);

        pthread_mutex_lock(&sharedMemLock);
        readers=0;
        if(waitingReaders>0)
        {
            pthread_cond_signal(&readPhase);
        }
        else
        {
            pthread_cond_signal(&writePhase);
        }
        pthread_mutex_unlock(&sharedMemLock);
        

    } 
    pthread_exit(0);
}

void *readerMain (void *threadArg)
{
    int id = *((int*)threadArg);
    int i=0; 
    int numreaders=0;

    for(i=0;i<READS;i++)
    {
        usleep(1000*(random() % READERS+WRITERS));

        pthread_mutex_lock(&sharedMemLock);
        waitingReaders++;
        while(readers==-1)
        {
            pthread_cond_wait(&readPhase,&sharedMemLock);
        }
        waitingReaders--;
        numreaders=++readers;
        pthread_mutex_unlock(&sharedMemLock);

         fprintf(stdout, "[r%d] reading %u  [readers: %2d]\n", id, sharedValue, numreaders);

         pthread_mutex_lock(&sharedMemLock);
         readers--;
         if(readers==0)
         {
            pthread_cond_signal(&writePhase);
         }
         pthread_mutex_unlock(&sharedMemLock);
    }
 pthread_exit(0);

}
// main function

int main(int argc, char **argv)
{
    int i;

    int readernum[READERS];
    int writernum[WRITERS];

    pthread_t readerThreadIDs[READERS];
    pthread_t writerThreadIDs[WRITERS];

    srandom((unsigned int)time(NULL));
    
    for(i=0;i<READERS;i++)
    {
        readernum[i] = i;
        pthread_create(&readerThreadIDs[i],NULL,readerMain,&readernum[i]);
    }

    for (i=0;i<WRITERS;i++)
    {
        writernum[i]=i;
        pthread_create(&writerThreadIDs[i],NULL,writerMain,&writernum[i]);
    }

    for(i=0;i<READERS;i++)
    {
        pthread_join(readerThreadIDs[i],NULL);   
    }

    for(i=0;i<WRITERS;i++)
    {
        pthread_join(writerThreadIDs[i],NULL);   
    }
return 0;

}






