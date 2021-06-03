#include "headers.h"

struct PG_S_MB processGenBuffer;
int PG_S_MQid;
int sigAlgo;
// If the PG is sending a new process to the scheduler, it signals it before sending
// Via the message queue
void new_process_handler(int signum)
{
    int PG_S_recVal = msgrcv(PG_S_MQid, &processGenBuffer, sizeof(processGenBuffer.P), processGenBuffer.mtype, !IPC_NOWAIT);
    if (PG_S_recVal != -1)
    {
        int pid = fork();
        if (pid == 0) // Child
        {
            execl("process", "process", (char *)NULL);
        }
        // TODO: Add this newly created process to the PCB
        
        // we may need to clear the receive buffer (not sure)
        // Afterwards, add it to different to whatever DS that's hodling tthe process
        // This depends on which signal algo we're going to use
        
    }
    signal(SIGUSR1, new_process_handler);
}


int main(int argc, char *argv[])
{
    initClk();
    // if the scheduler receives this signal, it means that the PG is sending it a new process
    signal(SIGUSR1, new_process_handler);
    // get the id of the PG_S message queue
    key_t kid = ftok(PG_S_FN, 'A');
    PG_S_MQid = msgget(kid, 0666 | IPC_CREAT);
    processGenBuffer.mtype = getpid() % 10000;
    sigAlgo = atoi(argv[1]);
    // Just testing if the forking works fine
    printf("Scheduler spawned!\n");
    
    // Main scheduler loop
    while (1)
    {
       
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
