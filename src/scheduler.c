#include "headers.h"

int main(int argc, char *argv[])
{
    initClk();
    int PG_S_recVal, PG_S_MQid, pid;
    // Just testing if the forking works fine
    printf("Scheduler spawned!\n");

    // Message buffer for communication between process generator and the scheduler
    // Used to receive new processes from the process generator
    PG_S_MQid = msgget('A', 0666 | IPC_CREAT);
    if (PG_S_MQid == -1)
    {
        perror("Error in creating the PG_S MQ");
        exit(-1);
    }
    struct PG_S_MB processGenBuffer;
    processGenBuffer.mtype = getpid() % 10000;
    // Main scheduler loop
    while (1)
    {
        // Start of by checking if the process generator sent a new process
        PG_S_recVal = msgrcv(PG_S_MQid, &processGenBuffer, sizeof(processGenBuffer.P), processGenBuffer.mtype, IPC_NOWAIT);
        // If the scheduler actually receives something
        if (PG_S_recVal != -1)
        {
            pid = fork();
            if (pid == 0) // Child
            {
                execl("process.exe", "process.exe", (char *)NULL);
            }
        }
        
    }
    
    //TODO: implement the scheduler.
    //TODO: upon termination release the clock resources.

    destroyClk(true);
}

// Process gen
// Process Generator takes input (process info. & scheduler algorithm of choice)
// Creates Schedeuler process & clock process
// 1 - Send process info to sched. when it's arival time arrives (could send more than one process at the same time)
// [
//      
//
//
//
//  ]
// ...
// 2 - Fork/create process & give it it's parameters 
// 3 - Sorting Algos
// 4 - Create an array(or any other DS) of structs with info about each process (state, running time, remaining time etc..)
// 5 - 
