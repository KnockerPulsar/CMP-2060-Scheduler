#include "headers.h"

/* Modify this file as needed*/

// Variables for communication between the scheduler & each process


int main(int agrc, char *argv[])
{
    initClk();

    fflush(stdin);
    printf("Process with PID %d initialized at time %d\n", getpid(), getClk());

    int S_P_ShMemid = shmget(ftok(SCHED_PROC_QKEY, 'B'), sizeof(int) ,0666 |IPC_CREAT);

    // Attach the shared memory to the scheduler
    int* memAdr = (int *) shmat(S_P_ShMemid, (void *) 0, 0);


    while (*memAdr > 0)
    {
        // Busy waiting
    }   

    fflush(stdin);
    printf("Terminating process with PID %d at time %d\n", getpid(), getClk());

    destroyClk(false);

    // Terminate this process
    raise(SIGKILL);

    return 0;
}
