/* mutex.c - mutex_init */

#include <xinu.h>

// declare global variables
uint32 sum = 0;
int lastpid = -1;
sid32 mutex_sem = NULL;

// initialize the mutex
void mutex_init(
        void
    )
{
    // TODO: initialize the mutex here
    mutex_sem = semcreate(1);
}

void incrementer(
    uint32 val,  // value to be incremented in each iteration
    uint32 iter     // number of iterations
) 
{
    int i;
    for(i = 0; i < iter; i++) {

        wait(mutex_sem); // lock
        
        // critical section
        sum = sum + val;
        lastpid = getpid();
        sleepms(50); // delay

        signal(mutex_sem); // unlock
    }   

}

void monitor(void)
{
    while (1) {

        wait(mutex_sem); // lock

        sleepms(500);
        uprintf("Value of sum is: %d and ID of last process is: %d\n", sum, lastpid);
        
        signal(mutex_sem); // unlock
    }
}